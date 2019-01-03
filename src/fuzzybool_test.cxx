#include "sys.h"
#include "debug.h"
#include "utils/FuzzyBool.h"
#include <iostream>
#include <iomanip>
#include <functional>

using utils::FuzzyBool;

FuzzyBool get_fuzzy_bool(int val)
{
  switch (val)
  {
    case utils::fuzzy_true:
      return fuzzy::True;
    case utils::fuzzy_was_true:
      return fuzzy::WasTrue;
    case utils::fuzzy_was_false:
      return fuzzy::WasFalse;
    case utils::fuzzy_false:
      break;
  }
  return fuzzy::False;
}

void print_table(std::function<FuzzyBool(FuzzyBool,FuzzyBool)> op)
{
  std::cout << std::setw(10) << ' ' << ' ';
  for (int v1 = 0; v1 < 4; ++v1)
  {
    FuzzyBool fb1 = get_fuzzy_bool(v1);
    std::cout << " |" << std::setw(10) << fb1;
  }
  std::cout << "\n-----------------------------------------------------------\n";
  for (int v0 = 0; v0 < 4; ++v0)
  {
    FuzzyBool fb0 = get_fuzzy_bool(v0);
    std::cout << std::setw(10) << fb0 << ' ';
    for (int v1 = 0; v1 < 4; ++v1)
    {
      FuzzyBool fb1 = get_fuzzy_bool(v1);

      std::cout << " |" << std::setw(10) << op(fb0, fb1);
    }
    std::cout << '\n';
  }
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  // Default construction.
  FuzzyBool default_constructed;
  // Construction by literal.
  FuzzyBool const fb1(fuzzy::True);
  // Assignment.
  FuzzyBool fb2;
  fb2 = fb1;
  // Copy constructor.
  FuzzyBool fb3(fb2);
  // Not operator.
  FuzzyBool fb4 = !fb3;
  // Compare equal.
  ASSERT(fb3 == fuzzy::True);
  ASSERT(fb3 == fb1);
  // Compare unequal.
  ASSERT(fb3 != fuzzy::False);
  ASSERT(fb3 != fb4);
  // Printing.
  Dout(dc::notice, "fb1 = " << fb1 << "; fb4 = " << fb4);

  // Special cases.
  using namespace fuzzy;
  ASSERT(fb2 == True);
  ASSERT(fb2.likely());
  fb2 = WasTrue;
  ASSERT((fb2 == WasTrue).has_same_value_as(WasTrue));
  ASSERT((fb2 != True).has_same_value_as(WasFalse));
  ASSERT(fb2.likely());

  ASSERT(fb4 == False);
  ASSERT(fb4.unlikely());
  fb4 = WasFalse;
  ASSERT((fb4 == WasFalse).has_same_value_as(WasTrue));
  ASSERT((fb4 != False).has_same_value_as(WasFalse));
  ASSERT(fb4.unlikely());

  fb2 = !fb2;
  ASSERT(fb2.has_same_value_as(WasFalse));

  fb4 = !fb4;
  ASSERT(fb4.has_same_value_as(WasTrue));

  Dout(dc::notice, "fb2 = " << fb2 << "; fb4 = " << fb4);

  std::cout << "\nIdentity:\n";
  print_table([](FuzzyBool, FuzzyBool fb2){ return fb2; });
  std::cout << "\nLogical NOT:\n";
  print_table([](FuzzyBool, FuzzyBool fb2){ ASSERT((!!fb2).has_same_value_as(fb2)); return !fb2; });
  std::cout << "\nLogical AND:\n";
  print_table([](FuzzyBool fb1, FuzzyBool fb2){ return fb1 && fb2; });
  std::cout << "\nLogical OR:\n";
  print_table([](FuzzyBool fb1, FuzzyBool fb2){ ASSERT((!(!fb1 && !fb2)).has_same_value_as(fb1 || fb2)); return fb1 || fb2; });
  std::cout << "\nOperator !=:\n";
  print_table([](FuzzyBool fb1, FuzzyBool fb2){ ASSERT(((fb1 && !fb2) || (!fb1 && fb2)).has_same_value_as(fb1 != fb2)); return fb1 != fb2; });
  std::cout << "\nOperator ==:\n";
  print_table([](FuzzyBool fb1, FuzzyBool fb2){ ASSERT((!(fb1 != fb2)).has_same_value_as(fb1 == fb2)); return fb1 == fb2; });
}
