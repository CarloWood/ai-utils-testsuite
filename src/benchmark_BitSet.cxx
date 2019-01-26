#include "sys.h"
#include "debug.h"
#include "cwds/benchmark.h"
#include "iacaMarks.h"

double const cpu_frequency = 3612059050.0;        // In cycles per second.
int const cpu = 0;                                // The CPU to run on.
size_t const loopsize = 1000;                     // Number of iterations.
size_t const minimum_of = 8;
int const nk = 8;
//double const resolution = 0.000000001;            // In seconds.

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  benchmark::Stopwatch stopwatch(cpu);

  // Calibrate Stopwatch overhead.
  stopwatch.calibrate_overhead(loopsize, minimum_of);

  uint64_t const m = 0x0000080e70100000UL;

  // The lambda is marked mutable because of the asm() that claims to change m,
  // however - you should not *really* change the input variables!
  // The [m = m] is needed because our m is const and doing just [m] weirdly
  // enough makes the type of the captured m also const, despite the mutable.
  auto result = stopwatch.measure<nk>(loopsize, [m = m]() mutable {
      uint64_t lsb;
      IACA_START
      asm volatile ("" : "=r" (m));

      // Code under test.
      lsb = m & -m;

      asm volatile ("" :: "r" (lsb));
      IACA_END
  }, minimum_of);

  std::cout << "Result: " << (result / cpu_frequency * 1e9 / loopsize) << " ns [measured " << result << " clocks]." << std::endl;
}
