#include "sys.h"
#include "utils/Vector.h"
#include "microbench/microbench.h"
#include "debug.h"
#include <iostream>
#include <cstdio>

#define NORMAL
#ifdef NORMAL
using vector_type = std::vector<int>;
using index_type = vector_type::size_type;
#else
using vector_type = utils::Vector<int>;
using index_type = vector_type::index_type;
#endif

vector_type v(1000, 2);
extern int sum;

void bench_mark()
{
#ifdef NORMAL
  unsigned int const s = v.size();
  for (unsigned int i = 0; i < s; ++i)
#else
  index_type const iend = v.iend();
  for (index_type i = v.ibegin(); i != iend; ++i)
#endif
  {
    sum += v[i];
  }
}

int sum = 0;

int main()
{
  vector_type v(100, -1);
  ASSERT(v.size() == 100);

#ifdef NORMAL
  index_type index{v.size()};
#else
  index_type index{v.iend()};
#endif
  index_type const i99{99};
  index_type const i98{98};
#ifndef NORMAL
  ASSERT(i98.get_value() == 98);
#endif
  index_type i;
  i = --index;                                  // i == index == 99
  ASSERT(v[i] == -1);
  v[i--] = 42;                                  // v[99] = 42; i == 98
  ASSERT(index == i99);
  ASSERT(v[index] == 42);
  ASSERT(i == i98);
  ASSERT(v[i] == -1);
  ASSERT(i != index);

  std::cout << "v[" << index << "] = " << v[index] << std::endl;
  std::cout << "v[" << i << "] = " << v[i] << std::endl;

  ++i;                                          // i == 99
  ASSERT(i == i99);
  ASSERT(i == index);

  // Bench mark.
  moodycamel::stats_t stats = moodycamel::microbench_stats(&bench_mark, 10000, 20);

  printf("utils::Vector statistics: avg: %.2fns, min: %.2fns, max: %.2fns, stddev: %.2fns, Q1: %.2fns, median: %.2fns, Q3: %.2fns\n",
    stats.avg() * 1000 * 1000,
    stats.min() * 1000 * 1000,
    stats.max() * 1000 * 1000,
    stats.stddev() * 1000 * 1000,
    stats.q1() * 1000 * 1000,
    stats.median() * 1000 * 1000,
    stats.q3() * 1000 * 1000);
}
