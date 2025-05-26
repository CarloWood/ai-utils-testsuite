#include "sys.h"
#include <utils/has_print_on.h>
#include <utils/has_to_string.h>
#include "debug.h"

namespace test {
using utils::has_print_on::operator<<;
using utils::has_to_string::to_string;

class Foo
{
 public:
  enum FooE {
    one,
    two
  };

  Foo(int x) : x_(x), e_{one} { }
  constexpr Foo(FooE e) : e_(e) { }

  void print_on(std::ostream& os) const;
  static constexpr std::string to_string(FooE);
  constexpr std::string to_string() const { return to_string(e_); }

 private:
  int x_;
  FooE e_;
};

void Foo::print_on(std::ostream& os) const
{
  os << "x_:" << x_;
}

//static
constexpr std::string Foo::to_string(FooE e)
{
  switch (e)
  {
    case one: return "one";
    case two: return "two";
  }
  AI_NEVER_REACHED
}

} // namespace test

namespace utils::has_to_string {

template<>
struct enclosing_class<test::Foo::FooE>
{
  using type = test::Foo;
};

} // namespace utils::has_to_string

// Nested namespace test.
namespace N1 {
namespace N2 {

struct C {
  std::string x_{"c"};
};

} // namespace N2

// This class defines a to_string method.
using utils::has_to_string::to_string;

struct B
{
  static std::string to_string(N2::C const& c);
};

std::string B::to_string(N2::C const& c)
{
  return std::string{"Calling B::to_string("} + c.x_ + "}";
}

} // namespace N1

namespace utils::has_to_string {
template<>
struct enclosing_class<N1::N2::C> {
  using type = typename N1::B;
};
} // namespace utils::has_to_string

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  test::Foo foo(42);

  Dout(dc::notice, "foo = " << foo);

  using namespace test;

  Foo e1 = Foo::two;

  Dout(dc::notice, "foo = " << to_string(e1));

  auto e2 = Foo::two;

  Dout(dc::notice, "foo = " << to_string(e2));

  N1::N2::C c;
  Dout(dc::notice, "c = " << to_string(c));
}
