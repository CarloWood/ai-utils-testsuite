#include "sys.h"
#include "debug.h"
#include "utils/MultiLoop.h"
#include <iostream>
#include <exception>

std::string digest;

void expect(std::string str)
{
  static int count;
  static int stage;
  static int last_ok;
  static size_t pos;

  if (digest.substr(pos, str.length()) != str)
  {
    Dout(dc::warning, "Expected \"" << digest.substr(pos, str.length()) << "\", got \"" << str << "\".");
    if (stage == 1)
      exit(0);
    stage = 1;
    pos = 0;
    count = 0;
    throw std::runtime_error("Mismatch");
  }
  pos += str.length();
  if (stage == 0)
    last_ok = count;
  std::cout << count++ << ": " << str << std::endl;
  if (stage == 1 && count - 1 == last_ok)
    Debug(attach_gdb());
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  for (int i0 = 0; i0 < 14; ++i0)
  {
    digest += std::to_string(i0) + ',';
    for (int i1 = i0; i1 < 12; ++i1)
    {
      if (i1 % 5 == 0)
        break;
      digest += std::to_string(i1) + ',';
      for (int i2 = i1; i2 < 10; ++i2)
      {
        if (i2 == 7)
        {
          i1 = 100;     // break out of two loops.
          break;
        }
        digest += std::to_string(i2) + ',';
        for (int i3 = i2; i3 < 8; ++i3)
        {
          if (i3 % 3 == 1)
            continue;
          digest += std::to_string(i3) + ',';
          if (i3 % 5 == 1)
            continue;
          if (i3 % 7 == 1)
            break;
          digest += "E3,";
        }
        digest += "e2,";
      }
      if (i1 != 100)
        digest += "e1,";
    }
    digest += "e0,";
  }

  for (;;)
  {
    try
    {
      for (MultiLoop ml(4); !ml.finished(); ml.next_loop()) // Have 4 for loops inside eachother.
      {
        // *ml : loop number, in the range [0, 4].
        // ml(): value of the current loop counter.
        while (ml() < 14 - 2 * (int)*ml)
        {
          if (*ml == 1 && ml() % 5 == 0)
          {
            ml.breaks(1);
            break;
          }
          if (*ml == 2 && ml() == 7)
          {
            ml.breaks(2);
            break;
          }
          if (!ml.inner_loop())
            expect(std::to_string(ml()) + ',');
          else
          {
            // Inner loop starts here.
            if (ml() % 3 == 1)
            {
              ml.breaks(0);
              break;
            }
            expect(std::to_string(ml()) + ',');
            if (ml() % 5 == 1)
            {
              ml.breaks(0);
              break;
            }
            if (ml() % 7 == 1)
            {
              ml.breaks(1);
              break;
            }
            // And ends here.
            expect("E3,");
          }
          // Mandatory line. The value passed is the starting value of the next loop
          // unless this is the inner loop, in which case the value is ignored.
          ml.start_next_loop_at(ml());
        }
        if (ml.end_of_loop())
        {
          // Other loops end here.
          expect("e" + std::to_string(*ml - 1) + ",");
        }
      }
    }
    catch (std::exception const&)
    {
      continue;
    }
    break;
  }
}
