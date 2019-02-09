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
    case 0:
      return fuzzy::False;
    case 1:
      return fuzzy::WasFalse;
    case 2:
      return fuzzy::WasTrue;
    case 3:
      break;
  }
  return fuzzy::True;
}

void print_table(std::function<FuzzyBool(FuzzyBool const&, FuzzyBool const&)> op)
{
  std::cout << std::setw(15) << ' ' << ' ';
  for (int v1 = 0; v1 < 4; ++v1)
  {
    FuzzyBool fb1 = get_fuzzy_bool(v1);
    std::cout << " |" << std::setw(15) << fb1;
  }
  std::cout << "\n-----------------------------------------------------------\n";
  for (int v0 = 0; v0 < 4; ++v0)
  {
    FuzzyBool fb0 = get_fuzzy_bool(v0);
    std::cout << std::setw(15) << fb0 << ' ';
    for (int v1 = 0; v1 < 4; ++v1)
    {
      FuzzyBool fb1 = get_fuzzy_bool(v1);

      std::cout << " |" << std::setw(15) << op(fb0, fb1);
    }
    std::cout << '\n';
  }
}

bool equal(FuzzyBool fb1, FuzzyBool fb2)
{
  return (fb1.is_true() && fb2.is_true()) ||
         (fb1.is_transitory_true() && fb2.is_transitory_true()) ||
         (fb1.is_transitory_false() && fb2.is_transitory_false()) ||
         (fb1.is_false() && fb2.is_false());
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
  ASSERT(fb2.is_momentary_true());
  fb2 = WasTrue;
  ASSERT((fb2 == WasTrue).is_transitory_true());
  ASSERT((fb2 != True).is_transitory_false());
  ASSERT(fb2.is_momentary_true());

  ASSERT(fb4 == False);
  ASSERT(fb4.is_momentary_false());
  fb4 = WasFalse;
  ASSERT((fb4 == WasFalse).is_transitory_true());
  ASSERT((fb4 != False).is_transitory_false());
  ASSERT(fb4.is_momentary_false());

  fb2 = (!fb2);
  ASSERT(fb2.is_transitory_false());

  fb4 = (!fb4);
  ASSERT(fb4.is_transitory_true());

  Dout(dc::notice, "fb2 = " << fb2 << "; fb4 = " << fb4);

  std::cout << "\nIdentity:\n";
  print_table([](FuzzyBool const&, FuzzyBool const& fb2){ return fb2; });
  std::cout << "\nLogical NOT:\n";
  print_table([](FuzzyBool const&, FuzzyBool const& fb2){ ASSERT(equal(!!fb2, fb2)); return (!fb2); });
  std::cout << "\nLogical AND:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){ return (fb1 && fb2); });
  std::cout << "\nLogical OR:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){ ASSERT(equal(!(!fb1 && !fb2), fb1 || fb2)); return (fb1 || fb2); });
  std::cout << "\nOperator !=:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){ ASSERT(equal((fb1 && !fb2) || (!fb1 && fb2), fb1 != fb2)); return (fb1 != fb2); });
  std::cout << "\nOperator ==:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){ ASSERT(equal(!(fb1 != fb2), fb1 == fb2)); return (fb1 == fb2); });
}
