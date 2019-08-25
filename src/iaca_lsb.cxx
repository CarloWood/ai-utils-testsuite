#include "iacaMarks.h"
using uint64_t = unsigned long;

void f()
{
  uint64_t m = 0x0000080e70100000UL;
  uint64_t lsb;

  IACA_START
  asm volatile ("" : "=r" (m));
  lsb = m & -m;
  asm volatile ("" :: "r" (lsb));
  IACA_END
}
