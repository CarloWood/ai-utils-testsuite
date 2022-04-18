#include "sys.h"
#include "utils/UltraHash.h"
#include "utils/AIAlert.h"
#include "utils/nearest_power_of_two.h"
#include "utils/debug_ostream_operators.h"
#include "debug.h"
#include <random>
#include <set>
#include <chrono>

#ifdef __OPTIMIZE__
#define BENCHMARK
#endif

#ifdef BENCHMARK
#include "cwds/benchmark.h"

double const cpu_frequency = 3612059050.0;      // In cycles per second.
int const cpu = benchmark::Stopwatch::cpu_any;  // The CPU to run on.
size_t const loopsize = 1000;                   // We'll be measing the number of clock cylces needed for this many iterations of the test code.
size_t const minimum_of = 3;                    // All but the fastest measurement of this many measurements are thrown away (3 is normally enough).
int const nk = 3;                               // The number of buckets of FrequencyCounter (with the highest counts) that are averaged over.
#endif

int main()
{
  Debug(NAMESPACE_DEBUG::init());

#ifdef BENCHMARK
  benchmark::Stopwatch stopwatch(cpu);          // Declare stopwatch and configure on which CPU it must run.

  // Calibrate Stopwatch overhead.
  stopwatch.calibrate_overhead(loopsize, minimum_of);
#endif

  utils::UltraHash ultra_hash;

  std::mt19937_64::result_type seed = 0x5dc53d8c54c8f;
  std::mt19937_64 seed_gen64(seed);

  constexpr long maxkeys = 50 * (1 << utils::UltraHash::max_test_bits);
  for (int number_of_keys = 1; number_of_keys <= maxkeys;)
  {
    for (int count = 0; count < 10; ++count)
    {
      seed = seed_gen64();
      Dout(dc::notice, "seed = " << std::hex << seed);
      std::mt19937_64 gen64(seed);

      std::vector<uint64_t> hashes;
      for (int i = 0; i < number_of_keys; ++i)
        hashes.push_back(gen64());

      try
      {
//#ifdef BENCHMARK
        auto start = std::chrono::steady_clock::now();
//#endif
        int size = ultra_hash.initialize(hashes);
//#ifdef BENCHMARK
        auto end = std::chrono::steady_clock::now();
        Dout(dc::notice, "Call to UltraHash::initialize() took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms.");
        std::cout << "Call to UltraHash::initialize() took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms." << std::endl;
//#endif
        Dout(dc::notice, "count = " << count);

#ifdef BENCHMARK
        // Benchmark the lookups too.
        size_t msum = 0;
#endif

        // Check if it worked.
        std::set<int> indices;
        for (uint64_t key : hashes)
        {
#ifdef BENCHMARK
          stopwatch.start();
#endif
          int index = ultra_hash.index(key);
#ifdef BENCHMARK
          stopwatch.stop();
#endif
          ASSERT(0 <= index && index < size);
          auto res = indices.insert(index);
  //        Dout(dc::notice, std::hex << key << " --> " << std::dec << index);
          ASSERT(res.second);
#ifdef BENCHMARK
          msum += stopwatch.diff_cycles();
#endif
        }
#ifdef BENCHMARK
        std::cout << "Average lookup time: " << (msum / hashes.size()) << " clock cycles (" << (msum / hashes.size() / cpu_frequency * 1e9) << " ns)." << std::endl;
#endif
      }
      catch (AIAlert::Error const& error)
      {
        DoutFatal(dc::core, error);
      }
    }
    double n;
    long npo2 = utils::nearest_power_of_two(std::lround(number_of_keys * 0.7071));
    if (number_of_keys <= 8)
      n = number_of_keys + 1;
    else if (number_of_keys < npo2 * 0.88)
      n = npo2 * 0.88;
    else if (number_of_keys > npo2 * 1.135)
      n = utils::nearest_power_of_two(number_of_keys) * 0.88;
    else
      number_of_keys *= 1.01;
    number_of_keys = std::max(number_of_keys + 1L, std::min(maxkeys, std::lround(n)));
  }
}
