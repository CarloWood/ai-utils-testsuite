#include "sys.h"
#include "utils/has_to_string.h"
#include "utils/to_string.h"
#include "utils/VectorIndex.h"
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

  // This will use the N::A::to_string() member function.
  auto a_str = to_string(a);
  Dout(dc::notice, "a = " << a_str);
  ASSERT(a_str == "A!");

  N::B b;

  // This will use the N::to_string(B) free function.
  auto b_str = to_string(b);
  Dout(dc::notice, "b = " << b_str);
  ASSERT(b_str == "B!");

  N::E1 e1 = N::e1;

  // This will use the N::to_string(E1) free function.
  auto e1_str = to_string(e1);
  Dout(dc::notice, "e1 = " << e1_str);
  ASSERT(e1_str == "e1!");
  // This will use the N::to_string(E1) free function.
  auto e1_utils_str = utils::to_string(e1);
  Dout(dc::notice, "e1 = " << e1_utils_str);
  ASSERT(e1_utils_str == "e1!");

  N::E2 e2 = N::E2::e2;

  // This will use the N::to_string(E2) free function.
  auto e2_str = to_string(e2);
  Dout(dc::notice, "e2 = " << e2_str);
  ASSERT(e2_str == "e2!");
  // This will use the N::to_string(E2) free function.
  auto e2_utils_str = utils::to_string(e2);
  Dout(dc::notice, "e2 = " << e2_utils_str);
  ASSERT(e2_utils_str == "e2!");

  N::E3 e3 = N::e3_no_exclamation_mark_ok;

  // This will use enchantum::to_string.
  auto e3_utils_str = utils::to_string(e3);
  Dout(dc::notice, "e3 = " << e3_utils_str);
  ASSERT(e3_utils_str == "e3_no_exclamation_mark_ok");

  utils::VectorIndex<N::E3> vi(42);

  // This will use the utils::to_string(VectorIndex<Category> const& index) free function.
  auto vi_str = to_string(vi);
  Dout(dc::notice, "vi = " << vi_str);
  ASSERT(vi_str == "#42");
  // This will use the utils::to_string(VectorIndex<Category> const& index) free function.
  auto vi_utils_str = to_string(vi);
  Dout(dc::notice, "vi = " << vi_utils_str);
  ASSERT(vi_utils_str == "#42");
}
