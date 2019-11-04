#include "sys.h"
#include "debug.h"
#include "utils/print_using.h"

using utils::print_using;

struct A {
};

std::ostream& operator<<(std::ostream& os, A const&)
{
  return os << "A";
}

// Global function with one argument.
void print_on(std::ostream& os)
{
  os << 42;
}

// Global function with two arguments.
void print_on(std::ostream& os, double d)
{
  os << d;
}

void print_on(std::ostream& os, A const& a)
{
  os << a;
}

// Member function.
class Foo
{
 public:
  void print_on(std::ostream& os) const
  {
    os << "Foo";
  }
};

void f(A& a)
{
  Dout(dc::notice, print_using(a, print_on));
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  Foo foo;
  A a;

  Dout(dc::notice, print_using(print_on));
  Dout(dc::notice, print_using(3.14, print_on));
  f(a);
  Dout(dc::notice, print_using(foo, &Foo::print_on));
}
