#include "sys.h"
#include "utils/List.h"
#include "utils/print_using.h"
#include <vector>
#include <deque>
#include <random>
#include <algorithm>
#include <list>
#include <chrono>
#include "utils/debug_ostream_operators.h"
#include <unistd.h>
#include "debug.h"

using list_type = utils::List<int>;

int compares;

struct IntCompare
{
  bool operator()(int lhs, int rhs) const
  {
//    Dout(dc::notice, "Comparing " << lhs << " < " << rhs);
    ++compares;
    return lhs < rhs;
  }
};

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  Dout(dc::notice, "A sorting algorithm.");

  // Prepare 100000 random lists.
  std::array<std::list<int>, 100000> std_lists;
  std::array<utils::List<int>, 100000> utils_lists;

  for (int i = 0; i < 100000; ++i)
  {
    constexpr int size = 128;
    std::vector<int> input;
    input.reserve(size);
    std::random_device rd;  // Used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(1, 10 * size);  // Define range for random numbers
    std::generate_n(std::back_inserter(input), size, [&]() { return distrib(gen); });
    std_lists[i] = std::list<int>{input.begin(), input.end()};
    utils_lists[i] = utils::List<int>{input.begin(), input.end()};
  }

  auto start = std::chrono::high_resolution_clock::now();
  unsigned long long std_sum = 0;
  for (int i = 0; i < 100000; ++i)
  {
    compares = 0;
    std_lists[i].sort(IntCompare{});
    std_sum += compares;
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto std_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "Execution time std::list<int>::sort " << (std_duration.count() / 100000.0) << " microseconds" << std::endl;
  Dout(dc::notice, "On average " << (std_sum / 100000.0) << " compares.");

  start = std::chrono::high_resolution_clock::now();
  unsigned long long utils_sum = 0;
  for (int i = 0; i < 100000; ++i)
  {
    compares = 0;
    utils_lists[i].sort(IntCompare{});
    utils_sum += compares;
  }
  end = std::chrono::high_resolution_clock::now();
  auto utils_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "Execution time utils::List<int>::sort " << (utils_duration.count() / 100000.0) << " microseconds" << std::endl;
  Dout(dc::notice, "On average " << (utils_sum / 100000.0) << " compares.");

  _exit(0);
}
