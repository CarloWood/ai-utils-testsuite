#include "sys.h"
#include "utils/is_between.h"
#include <iomanip>
#include <iostream>
#include <algorithm>
#include "debug.h"

using integral_type = int;

bool slow_is_between(integral_type a, integral_type val, integral_type b)
{
  return std::min(a, b) < val && val < std::max(a, b);
}

enum Equal {
  unknown,
  less_than,
  less_than_or_equal
};

void set(Equal& comp, Equal e)
{
  ASSERT(comp == unknown || comp == e);
  comp = e;
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  Equal low = unknown;
  Equal high = unknown;

  for (integral_type a = 42; a < 47; ++a)
    for (integral_type b = 42; b < 47; ++b)
      for (integral_type val = 42; val < 47; ++val)
      {
        bool fact = slow_is_between(a, val, b);
        bool test = utils::is_between_lt_le(a, val, b);
        bool equal = a == val || b == val;
        bool empty = a == b;

//        std::cout << "a = " << a << ", b = " << b << ", val = " << val << " : " << std::boolalpha << test << '\n';

        if (empty)
        {
          // If the range is empty (low == high), then
          // the result should only be true if we have
          //
          //  low <= val <= high
          //
          if (test)
          {
            // If val != a (and b) then the result should have been false (not in between).
            ASSERT(val == a);
            // If the result is true then a == b == val and we must have low <= val <= high.
            set(low, less_than_or_equal);
            set(high, less_than_or_equal);
          }
          else if (val == a)
          {
            // If low == val == high, and the result is false then at least one comparision must be not `less_than_or_equal`.
            ASSERT(low != less_than_or_equal || high != less_than_or_equal);
          }
        }
        else if (equal)
        {
          integral_type min = std::min(a, b);
          integral_type max = std::max(a, b);

          if (val == min)
          {
            if (test)
              set(low, less_than_or_equal);
            else
              set(low, less_than);
          }
          else // val == max
          {
            if (test)
              set(high, less_than_or_equal);
            else
              set(high, less_than);
          }
        }
        else
        {
          // If low != val != high  then it is unambiguous what the result should be.
          ASSERT(test == fact);
        }
      }

  ASSERT(low != unknown && high != unknown);

  std::cout << "low <";
  if (low == less_than_or_equal)
    std::cout << "=";
  std::cout << " val <";
  if (high == less_than_or_equal)
    std::cout << "=";
  std::cout << " high" << std::endl;
}
