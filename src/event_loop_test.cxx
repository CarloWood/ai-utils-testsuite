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
  loop.wait_for(1s);
  EXPECT_EQ(loop.result().events(), highest);
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
  loop.wait();
  auto const& result = loop.result();
  auto result_events = result.events();
  EXPECT_EQ(result.status(), Status::pending_events);
  EXPECT_EQ(result.events(), first | second);
  loop.wait_for(0ns);
  expect_empty(result, Status::timed_out);
  loop.notify(first);
  loop.wait_for(1s);
  auto& next = loop.result();
  EXPECT_EQ(next.status(), Status::pending_events);
  EXPECT_EQ(next.events(), first);
  EXPECT_EQ(result_events, first | second);
}

// Each timeout must replace a known nonempty result, rather than merely preserve
// default or previously timed-out storage. Positive waits must also honor their delay.
TEST(EventLoop, Timeouts)
{
  Loop loop;
  auto event = loop.watch_event();
  auto const& result = loop.result();
  for (int kind = 0; kind < 7; ++kind)
  {
    SCOPED_TRACE(kind);
    loop.notify(event);
    loop.wait_for(1s);
    ASSERT_EQ(result.status(), Status::pending_events);
    ASSERT_EQ(result.events(), event);

    auto start = std::chrono::steady_clock::now();
    switch (kind)
    {
      case 0:
        loop.wait_for(0ns);
        break;
      case 1:
        loop.wait_for(-1ms);
        break;
      case 2:
        loop.wait_until(std::chrono::steady_clock::now() - 1s);
        break;
      case 3:
        loop.wait_until(std::chrono::system_clock::now() - 1s);
        break;
      case 4:
        loop.wait_for(20ms);
        break;
      case 5:
        loop.wait_until(std::chrono::steady_clock::now() + 20ms);
        break;
      case 6:
        loop.wait_until(std::chrono::system_clock::now() + 20ms);
        break;
    }
    expect_empty(result, Status::timed_out);
    if (kind >= 4)
      EXPECT_GE(std::chrono::steady_clock::now() - start, 20ms);
  }
}

// An expired deadline still polls pending work; it must not unconditionally
// return a timeout. Start each case from a checked empty result.
TEST(EventLoop, ExpiredWaitsConsumePendingNotifications)
{
  Loop loop;
  auto event = loop.watch_event();
  auto const& result = loop.result();
  for (int kind = 0; kind < 4; ++kind)
  {
    SCOPED_TRACE(kind);
    loop.wait_for(0ns);
    expect_empty(result, Status::timed_out);
    loop.notify(event);
    switch (kind)
    {
      case 0:
        loop.wait_for(0ns);
        break;
      case 1:
        loop.wait_for(-1ms);
        break;
      case 2:
        loop.wait_until(std::chrono::steady_clock::now() - 1s);
        break;
      case 3:
        loop.wait_until(std::chrono::system_clock::now() - 1s);
        break;
    }
    ASSERT_EQ(result.status(), Status::pending_events);
    ASSERT_EQ(result.events(), event);
    loop.wait_for(0ns);
    expect_empty(result, Status::timed_out);
  }
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
    loop.wait_until(deadline);
    auto const& result = loop.result();
    ASSERT_EQ(result.status(), Status::pending_events);
    EXPECT_TRUE((received & result.events()).none());
    for (int i = 0; i < count; ++i)
      if (result.events().test(events[i]))
        EXPECT_EQ(payloads[i], 100 + i);
    received |= result.events();
  }
  loop.wait_for(0ns);
  auto const& result = loop.result();
  expect_empty(result, Status::timed_out);
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
  loop.wait_for(1s);
  auto const& result = loop.result();
  ASSERT_EQ(result.status(), Status::pending_events);
  EXPECT_EQ(result.events(), event);
  for (int i = 0; i < count; ++i)
    EXPECT_EQ(payloads[i], 100 + i);
  loop.wait_for(0ns);
  expect_empty(result, Status::timed_out);
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
    if (kind == 0)
      loop.wait();
    else if (kind == 1)
      loop.wait_for(2s);
    else
      loop.wait_until(std::chrono::system_clock::now() + 2s);
    auto const& result = loop.result();
    EXPECT_EQ(result.status(), Status::pending_events);
    EXPECT_EQ(result.events(), event);
  }
}

// Ready descriptors and notifications share one mask; readiness is level-triggered.
// A retained reference observes the reused result storage after subsequent waits.
TEST(EventLoop, DescriptorReadinessAndResultReuse)
{
  SocketPair first;
  SocketPair second;
  Loop loop;
  auto a = loop.watch_fd(first[0], Loop::PollEvents::read);
  auto ai = Loop::index(a);
  auto b = loop.watch_fd(second[0], Loop::PollEvents::read);
  auto bi = Loop::index(b);
  auto message = loop.watch_event();
  char byte = 'x';
  ASSERT_EQ(::write(first[1], &byte, 1), 1);
  ASSERT_EQ(::write(second[1], &byte, 1), 1);
  loop.notify(message);
  loop.wait_for(1s);
  auto const& result = loop.result();
  ASSERT_EQ(result.status(), Status::pending_events);
  ASSERT_EQ(result.events(), a | b | message);
  EXPECT_EQ(result.fd(ai), first[0]);
  EXPECT_EQ(result.fd(bi), second[0]);
  EXPECT_TRUE(has_event(result.ready_events(ai), Loop::ReadyEvents::read));
  EXPECT_TRUE(has_event(result.ready_events(bi), Loop::ReadyEvents::read));
  loop.wait_for(0ns);
  auto const& repeated = loop.result();
  EXPECT_EQ(repeated.events(), a | b);
  ASSERT_EQ(::read(first[0], &byte, 1), 1);
  ASSERT_EQ(::read(second[0], &byte, 1), 1);
  loop.wait_for(0ns);
  expect_empty(result, Status::timed_out);
}

// Readiness flags must be replaced, not accumulated, even when the same event
// remains present in consecutive results. Draining input leaves only writable readiness.
TEST(EventLoop, ReadinessFlagsAreReplaced)
{
  SocketPair sockets;
  Loop loop;
  auto event = loop.watch_fd(sockets[0], Loop::PollEvents::read | Loop::PollEvents::write);
  auto index = Loop::index(event);
  auto const& result = loop.result();
  for (int iteration = 0; iteration < 3; ++iteration)
  {
    SCOPED_TRACE(iteration);
    char byte = 'x';
    ASSERT_EQ(::write(sockets[1], &byte, 1), 1);
    loop.wait_for(1s);
    ASSERT_EQ(result.status(), Status::pending_events);
    ASSERT_EQ(result.events(), event);
    EXPECT_EQ(result.fd(index), sockets[0]);
    EXPECT_EQ(result.ready_events(index), Loop::ReadyEvents::read | Loop::ReadyEvents::write);

    ASSERT_EQ(::read(sockets[0], &byte, 1), 1);
    loop.wait_for(1s);
    ASSERT_EQ(result.status(), Status::pending_events);
    ASSERT_EQ(result.events(), event);
    EXPECT_EQ(result.fd(index), sockets[0]);
    EXPECT_EQ(result.ready_events(index), Loop::ReadyEvents::write);
  }
}

// A socket can report read and write together; peer shutdown reports hangup even
// though unread data remains available. No registered descriptor is closed early.
TEST(EventLoop, CombinedReadinessAndHangup)
{
  SocketPair sockets;
  Loop loop;
  auto event = loop.watch_fd(sockets[0], Loop::PollEvents::read | Loop::PollEvents::write);
  auto event_index = Loop::index(event);
  loop.wait_for(1s);
  auto const& result = loop.result();
  ASSERT_EQ(result.events(), event);
  EXPECT_TRUE(has_event(result.ready_events(event_index), Loop::ReadyEvents::write));
  char byte = 'x';
  ASSERT_EQ(::write(sockets[1], &byte, 1), 1);
  loop.wait_for(1s);
  ASSERT_EQ(result.events(), event);
  EXPECT_TRUE(has_event(result.ready_events(event_index), Loop::ReadyEvents::read));
  EXPECT_TRUE(has_event(result.ready_events(event_index), Loop::ReadyEvents::write));
  ASSERT_EQ(::shutdown(sockets[1], SHUT_RDWR), 0);
  loop.wait_for(1s);
  ASSERT_EQ(result.events(), event);
  EXPECT_TRUE(has_event(result.ready_events(event_index), Loop::ReadyEvents::hangup));
  EXPECT_TRUE(has_event(result.ready_events(event_index), Loop::ReadyEvents::read));
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
  auto const& result = loop.result();
  do
    loop.wait();
  while (result.status() != Status::terminated);
  expect_empty(result, Status::terminated);
  terminator.join();
  EXPECT_TRUE(returned.load());
  loop.notify(event);
  loop.wait_for(0ns);
  expect_empty(result, Status::terminated);
}

// A termination request also wakes an otherwise idle, indefinitely waiting owner.
TEST(EventLoop, TerminationWakesBlockedWait)
{
  Loop loop;
  std::jthread terminator([&] {
    std::this_thread::sleep_for(20ms);
    loop.terminate();
  });
  loop.wait();
  auto const& result = loop.result();
  expect_empty(result, Status::terminated);
}

} // namespace

// Initialize the project's debug runtime before executing the standalone suite.
int main(int argc, char** argv)
{
  Debug(NAMESPACE_DEBUG::init());
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
