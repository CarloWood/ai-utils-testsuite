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

  fb2 = !fb2.load();
  ASSERT(fb2.has_same_value_as(WasFalse));

  fb4 = !fb4.load();
  ASSERT(fb4.has_same_value_as(WasTrue));

  Dout(dc::notice, "fb2 = " << fb2 << "; fb4 = " << fb4);

  std::cout << "\nIdentity:\n";
  print_table([](FuzzyBool const&, FuzzyBool const& fb2){
      AtomicFuzzyBool afb = fb2;
      ASSERT(afb.has_same_value_as(fb2));
      return afb.load();
  });

  std::cout << "\nLogical NOT:\n";
  print_table([](FuzzyBool const&, FuzzyBool const& fb2){
      ASSERT((!!fb2).has_same_value_as(fb2));
      AtomicFuzzyBool afb = fb2;
      FuzzyBool old = afb.fetch_invert();
      ASSERT(afb.has_same_value_as(!fb2));
      ASSERT(old.has_same_value_as(fb2));
      return afb.load();
  });

  std::cout << "\nLogical AND:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){
      AtomicFuzzyBool afb = fb1;
      FuzzyBool old = afb.fetch_AND(fb2);
      ASSERT(afb.has_same_value_as(fb1 && fb2));
      ASSERT(old.has_same_value_as(fb1));
      return afb.load();
  });

  std::cout << "\nLogical OR:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){
      ASSERT((!(!fb1 && !fb2)).has_same_value_as(fb1 || fb2));
      AtomicFuzzyBool afb = fb1;
      FuzzyBool old = afb.fetch_OR(fb2);
      ASSERT(afb.has_same_value_as(fb1 || fb2));
      ASSERT(old.has_same_value_as(fb1));
      return afb.load();
  });

  std::cout << "\nLogical XOR:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){
      ASSERT(((fb1 && !fb2) || (!fb1 && fb2)).has_same_value_as(fb1 != fb2));
      AtomicFuzzyBool afb = fb1;
      FuzzyBool old = afb.fetch_XOR(fb2);
      ASSERT(afb.has_same_value_as(fb1 != fb2));
      ASSERT(old.has_same_value_as(fb1));
      return afb.load();
  });

  std::cout << "\nLogical NOT XOR:\n";
  print_table([](FuzzyBool const& fb1, FuzzyBool const& fb2){
      ASSERT((!(fb1 != fb2)).has_same_value_as(fb1 == fb2));
      AtomicFuzzyBool afb = fb1;
      FuzzyBool old = afb.fetch_NOT_XOR(fb2);
      ASSERT(afb.has_same_value_as(fb1 == fb2));
      ASSERT(old.has_same_value_as(fb1));
      return afb.load();
  });
}
