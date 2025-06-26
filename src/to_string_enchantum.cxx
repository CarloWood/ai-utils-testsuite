#include "sys.h"
#include "utils/has_to_string.h"
#include "utils/to_string.h"
#include "debug.h"

namespace N {
using utils::has_to_string::to_string;

struct A
{
  std::string to_string() const
  {
    return "A!";
  }
};

struct B
{
};

std::string to_string(B b)
{
  return "B!";
}

enum E1 {
  e1
};

std::string to_string(E1)
{
  return "e1!";
}

enum class E2 {
  e2
};

std::string to_string(E2)
{
  return "e2!";
}

enum E3 {
  e3_no_exclamation_mark_ok
};

} // namespace N

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  N::A a;

  Dout(dc::notice, "a = " << to_string(a));

  N::B b;

  Dout(dc::notice, "b = " << to_string(b));

  N::E1 e1 = N::e1;

  Dout(dc::notice, "e1 = " << to_string(e1));
  Dout(dc::notice, "e1 = " << utils::to_string(e1));

  N::E2 e2 = N::E2::e2;

  Dout(dc::notice, "e2 = " << to_string(e2));
  Dout(dc::notice, "e2 = " << utils::to_string(e2));

  N::E3 e3 = N::e3_no_exclamation_mark_ok;

  Dout(dc::notice, "e3 = " << utils::to_string(e3));
}
