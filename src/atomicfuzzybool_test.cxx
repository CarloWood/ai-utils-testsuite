#include "sys.h"
#include "debug.h"
#include "utils/AtomicFuzzyBool.h"
#include <iostream>
#include <iomanip>
#include <functional>

using utils::FuzzyBool;
using utils::AtomicFuzzyBool;

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
  //case 3:
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

bool equal(AtomicFuzzyBool const& fb1, AtomicFuzzyBool const& fb2)
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
  AtomicFuzzyBool default_constructed;
  // Construction by literal.
  AtomicFuzzyBool const fb1(fuzzy::True);
  // Assignment.
  AtomicFuzzyBool fb2;
  fb2 = fb1.load();
  // Copy constructor.
  AtomicFuzzyBool fb3(fb2.load());
  // Not operator.
  AtomicFuzzyBool fb4 = !fb3.load();
  // Compare equal.
  ASSERT(fb3.load() == fuzzy::True);
  ASSERT(fb3 == fb1.load());
  // Compare unequal.
  ASSERT(fb3 != fuzzy::False);
  ASSERT(fb3.load() != fb4);
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

  fb2 = !fb2.load();
  ASSERT(fb2.is_transitory_false());

  fb4 = !fb4.load();
  ASSERT(fb4.is_transitory_true());

  Dout(dc::notice, "fb2 = " << fb2 << "; fb4 = " << fb4);

  std::cout << "\nIdentity:\n";
  print_table([](FuzzyBool const&, FuzzyBool const& fb2){
      AtomicFuzzyBool afb = fb2;
      ASSERT(equal(afb, fb2));
      return afb.load();
  });

  std::cout << "\nLogical NOT:\n";
  print_table([](FuzzyBool const&, FuzzyBool const& fb2){
      ASSERT(equal(!!fb2, fb2));
      AtomicFuzzyBool afb = fb2;
      FuzzyBool old = afb.fetch_invert();
      ASSERT(equal(afb, !fb2));
      ASSERT(equal(old, fb2));
      return afb.load();
  });

  std::cout << "\nLogical AND:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){
      AtomicFuzzyBool afb = fb1;
      FuzzyBool old = afb.fetch_AND(fb2);
      ASSERT(equal(afb, fb1 && fb2));
      ASSERT(equal(old, fb1));
      return afb.load();
  });

  std::cout << "\nLogical OR:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){
      ASSERT(equal(!(!fb1 && !fb2), fb1 || fb2));
      AtomicFuzzyBool afb = fb1;
      FuzzyBool old = afb.fetch_OR(fb2);
      ASSERT(equal(afb, fb1 || fb2));
      ASSERT(equal(old, fb1));
      return afb.load();
  });

  std::cout << "\nLogical XOR:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){
      ASSERT(equal((fb1 && !fb2) || (!fb1 && fb2), fb1 != fb2));
      AtomicFuzzyBool afb = fb1;
      FuzzyBool old = afb.fetch_XOR(fb2);
      ASSERT(equal(afb, fb1 != fb2));
      ASSERT(equal(old, fb1));
      return afb.load();
  });

  std::cout << "\nLogical NOT XOR:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){
      ASSERT(equal(!(fb1 != fb2), fb1 == fb2));
      AtomicFuzzyBool afb = fb1;
      FuzzyBool old = afb.fetch_NOT_XOR(fb2);
      ASSERT(equal(afb, fb1 == fb2));
      ASSERT(equal(old, fb1));
      return afb.load();
  });
}
