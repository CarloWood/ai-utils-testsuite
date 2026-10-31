// SPDX-FileCopyrightText: 2026 Carlo Wood
// SPDX-License-Identifier: MIT

#include "sys.h"
#include "utils/threading/EventLoop.h"

#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <future>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>
#include "debug.h"

namespace {

using Loop = utils::threading::EventLoop;
using Status = Loop::WaitStatus;
using namespace std::chrono_literals;

// Own both ends of a local stream socket pair, closing them even on test failure.
// Socket pairs provide portable read/write readiness without external services.
class SocketPair
{
 private:
  std::array<int, 2> fds_{{-1, -1}};

 public:
  SocketPair()
  {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fds_.data()) == -1)
      throw std::system_error(errno, std::generic_category(), "socketpair");
  }
  ~SocketPair()
  {
    for (int fd : fds_)
      if (fd != -1)
        ::close(fd);
  }
  SocketPair(SocketPair const&) = delete;
  SocketPair& operator=(SocketPair const&) = delete;
  int operator[](std::size_t i) const { return fds_[i]; }
};

// Check the status/mask invariant without accessing FD-only result members.
void expect_empty(Loop::WaitResult const& result, Status status)
{
  EXPECT_EQ(result.status(), status);
  EXPECT_TRUE(result.events().none());
}

// Check unique tokens and the shared notification/descriptor registration limit.
TEST(EventLoop, RegistrationCapacity)
{
  SocketPair sockets;
  Loop loop;
  EXPECT_EQ(loop.number_of_events(), 0u);
  auto fd_event = loop.watch_fd(sockets[0], Loop::PollEvents::read);
  EXPECT_TRUE(loop.is_fd_event(fd_event));
  Loop::EventMask seen = fd_event;
  for (int i = 1; i < Loop::max_number_of_events; ++i)
  {
    auto event = loop.watch_event();
    EXPECT_TRUE(event.is_single_bit());
    EXPECT_FALSE(seen.test(event));
    EXPECT_FALSE(loop.is_fd_event(event));
    EXPECT_EQ(Loop::index(event).get_value(), i);
    seen |= event;
  }
  EXPECT_EQ(loop.number_of_events(), Loop::max_number_of_events);
  EXPECT_THROW((void)loop.watch_event(), std::runtime_error);
  EXPECT_THROW((void)loop.watch_fd(sockets[1], Loop::PollEvents::read), std::runtime_error);
  Loop::Event highest{seen.mssbi()};
  loop.notify(highest);
  EXPECT_EQ(loop.wait_for(1s).events(), highest);
}

// Rejected descriptor registrations must not consume tokens or poison later registration.
TEST(EventLoop, InvalidRegistrations)
{
  SocketPair sockets;
  Loop loop;
  EXPECT_THROW((void)loop.watch_fd(-1, Loop::PollEvents::read), std::invalid_argument);
  EXPECT_THROW((void)loop.watch_fd(sockets[0], Loop::PollEvents{}), std::invalid_argument);
  EXPECT_THROW((void)loop.watch_fd(sockets[0], static_cast<Loop::PollEvents>(4)), std::invalid_argument);
  EXPECT_EQ(loop.number_of_events(), 0u);
  auto event = loop.watch_fd(sockets[0], Loop::PollEvents::read);
  EXPECT_THROW((void)loop.watch_fd(sockets[0], Loop::PollEvents::write), std::invalid_argument);
  EXPECT_EQ(loop.number_of_events(), 1u);
  EXPECT_TRUE(loop.is_fd_event(event));
  EXPECT_FALSE(loop.is_fd_event(loop.watch_event()));
}

// Pending notifications coalesce, are consumed once, and may be published again.
TEST(EventLoop, PendingAndCoalescedNotifications)
{
  Loop loop;
  auto first = loop.watch_event();
  auto second = loop.watch_event();
  loop.notify(first);
  loop.notify(first);
  loop.notify(second);
  auto result = loop.wait();
  EXPECT_EQ(result.status(), Status::pending_events);
  EXPECT_EQ(result.events(), first | second);
  expect_empty(loop.wait_for(0ns), Status::timed_out);
  loop.notify(first);
  auto next = loop.wait_for(1s);
  EXPECT_EQ(next.status(), Status::pending_events);
  EXPECT_EQ(next.events(), first);
  EXPECT_EQ(result.events(), first | second);
}

// Empty waits respect both relative and absolute deadlines, including expired ones.
TEST(EventLoop, Timeouts)
{
  Loop loop;
  expect_empty(loop.wait_for(0ns), Status::timed_out);
  expect_empty(loop.wait_for(-1ms), Status::timed_out);
  expect_empty(loop.wait_until(std::chrono::steady_clock::now() - 1s), Status::timed_out);
  expect_empty(loop.wait_until(std::chrono::system_clock::now() - 1s), Status::timed_out);
  auto start = std::chrono::steady_clock::now();
  expect_empty(loop.wait_for(20ms), Status::timed_out);
  EXPECT_GE(std::chrono::steady_clock::now() - start, 20ms);
  auto deadline = std::chrono::steady_clock::now() + 20ms;
  expect_empty(loop.wait_until(deadline), Status::timed_out);
  EXPECT_GE(std::chrono::steady_clock::now(), deadline);
}

// Multiple producers publish independent ordinary payloads through notify's release.
// Read payloads before joining producers so join does not supply the tested acquire.
TEST(EventLoop, ConcurrentPublication)
{
  Loop loop;
  constexpr int count = 8;
  std::array<Loop::Event, count> events;
  std::array<int, count> payloads{};
  for (auto& event : events)
    event = loop.watch_event();
  std::vector<std::jthread> producers;
  for (int i = 0; i < count; ++i)
    producers.emplace_back([&, i] {
      payloads[i] = 100 + i;
      loop.notify(events[i]);
    });
  Loop::EventMask received{Loop::no_events};
  auto deadline = std::chrono::steady_clock::now() + 2s;
  while (received.count() < count)
  {
    auto result = loop.wait_until(deadline);
    ASSERT_EQ(result.status(), Status::pending_events);
    EXPECT_TRUE((received & result.events()).none());
    for (int i = 0; i < count; ++i)
      if (result.events().test(events[i]))
        EXPECT_EQ(payloads[i], 100 + i);
    received |= result.events();
  }
  expect_empty(loop.wait_for(0ns), Status::timed_out);
}

// Even notifications coalescing onto an already-set bit must publish each
// producer's payload. The relaxed counter coordinates scheduling, not visibility.
TEST(EventLoop, CoalescedPublication)
{
  Loop loop;
  auto event = loop.watch_event();
  constexpr int count = 8;
  std::array<int, count> payloads{};
  std::atomic<int> published{0};
  std::vector<std::jthread> producers;
  for (int i = 0; i < count; ++i)
    producers.emplace_back([&, i] {
      payloads[i] = 100 + i;
      loop.notify(event);
      published.fetch_add(1, std::memory_order_relaxed);
    });
  auto deadline = std::chrono::steady_clock::now() + 2s;
  while (published.load(std::memory_order_relaxed) != count && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();
  ASSERT_EQ(published.load(std::memory_order_relaxed), count);
  auto result = loop.wait_for(1s);
  ASSERT_EQ(result.status(), Status::pending_events);
  EXPECT_EQ(result.events(), event);
  for (int i = 0; i < count; ++i)
    EXPECT_EQ(payloads[i], 100 + i);
  expect_empty(loop.wait_for(0ns), Status::timed_out);
}

// Exercise all three wait entry points with a producer that publishes after a delay.
// The generous deadline and CTest watchdog bound failures without latency thresholds.
TEST(EventLoop, NotificationWakesWaits)
{
  for (int kind = 0; kind < 3; ++kind)
  {
    SCOPED_TRACE(kind);
    Loop loop;
    auto event = loop.watch_event();
    std::jthread producer([&] {
      std::this_thread::sleep_for(20ms);
      loop.notify(event);
    });
    auto result = kind == 0 ? loop.wait() : kind == 1 ? loop.wait_for(2s) : loop.wait_until(std::chrono::system_clock::now() + 2s);
    EXPECT_EQ(result.status(), Status::pending_events);
    EXPECT_EQ(result.events(), event);
  }
}

// Ready descriptors and notifications share one mask; readiness is level-triggered.
// Older results must retain their FD metadata after later waits change readiness.
TEST(EventLoop, DescriptorReadinessAndSnapshots)
{
  SocketPair first;
  SocketPair second;
  Loop loop;
  auto a = loop.watch_fd(first[0], Loop::PollEvents::read);
  auto b = loop.watch_fd(second[0], Loop::PollEvents::read);
  auto message = loop.watch_event();
  char byte = 'x';
  ASSERT_EQ(::write(first[1], &byte, 1), 1);
  ASSERT_EQ(::write(second[1], &byte, 1), 1);
  loop.notify(message);
  auto result = loop.wait_for(1s);
  ASSERT_EQ(result.status(), Status::pending_events);
  ASSERT_EQ(result.events(), a | b | message);
  EXPECT_EQ(result.fd(a), first[0]);
  EXPECT_EQ(result.fd(b), second[0]);
  EXPECT_TRUE(has_event(result.ready_events(a), Loop::ReadyEvents::read));
  EXPECT_TRUE(has_event(result.ready_events(b), Loop::ReadyEvents::read));
  auto repeated = loop.wait_for(0ns);
  EXPECT_EQ(repeated.events(), a | b);
  ASSERT_EQ(::read(first[0], &byte, 1), 1);
  ASSERT_EQ(::read(second[0], &byte, 1), 1);
  expect_empty(loop.wait_for(0ns), Status::timed_out);
  EXPECT_EQ(result.events(), a | b | message);
  EXPECT_EQ(result.fd(a), first[0]);
  EXPECT_TRUE(has_event(result.ready_events(a), Loop::ReadyEvents::read));
}

// A socket can report read and write together; peer shutdown reports hangup even
// though unread data remains available. No registered descriptor is closed early.
TEST(EventLoop, CombinedReadinessAndHangup)
{
  SocketPair sockets;
  Loop loop;
  auto event = loop.watch_fd(sockets[0], Loop::PollEvents::read | Loop::PollEvents::write);
  auto writable = loop.wait_for(1s);
  ASSERT_EQ(writable.events(), event);
  EXPECT_TRUE(has_event(writable.ready_events(event), Loop::ReadyEvents::write));
  char byte = 'x';
  ASSERT_EQ(::write(sockets[1], &byte, 1), 1);
  auto both = loop.wait_for(1s);
  ASSERT_EQ(both.events(), event);
  EXPECT_TRUE(has_event(both.ready_events(event), Loop::ReadyEvents::read));
  EXPECT_TRUE(has_event(both.ready_events(event), Loop::ReadyEvents::write));
  ASSERT_EQ(::shutdown(sockets[1], SHUT_RDWR), 0);
  auto hungup = loop.wait_for(1s);
  ASSERT_EQ(hungup.events(), event);
  EXPECT_TRUE(has_event(hungup.ready_events(event), Loop::ReadyEvents::hangup));
  EXPECT_TRUE(has_event(hungup.ready_events(event), Loop::ReadyEvents::read));
  ASSERT_EQ(::read(sockets[0], &byte, 1), 1);
  EXPECT_EQ(byte, 'x');
  EXPECT_EQ(::read(sockets[0], &byte, 1), 0);
}

// terminate blocks until the wait thread acknowledges it, discarding pending work.
// All registration finishes before the producer starts; promises only coordinate
// the termination handshake, not notification payload publication.
TEST(EventLoop, TerminationAcknowledgement)
{
  Loop loop;
  auto event = loop.watch_event();
  loop.notify(event);
  std::promise<void> entering;
  std::atomic<bool> returned{false};
  std::jthread terminator([&] {
    entering.set_value();
    loop.terminate();
    returned.store(true);
  });
  entering.get_future().wait();
  EXPECT_FALSE(returned.load());
  // A pending notification may win the race with the termination request.
  // Keep waiting until termination is observed rather than assuming scheduling.
  Loop::WaitResult result;
  do
    result = loop.wait();
  while (result.status() != Status::terminated);
  expect_empty(result, Status::terminated);
  terminator.join();
  EXPECT_TRUE(returned.load());
  loop.notify(event);
  expect_empty(loop.wait_for(0ns), Status::terminated);
}

// A termination request also wakes an otherwise idle, indefinitely waiting owner.
TEST(EventLoop, TerminationWakesBlockedWait)
{
  Loop loop;
  std::jthread terminator([&] {
    std::this_thread::sleep_for(20ms);
    loop.terminate();
  });
  expect_empty(loop.wait(), Status::terminated);
}

} // namespace

// Initialize the project's debug runtime before executing the standalone suite.
int main(int argc, char** argv)
{
  Debug(NAMESPACE_DEBUG::init());
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
