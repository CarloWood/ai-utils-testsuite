#include "sys.h"
#include "debug.h"
#include "utils/print_using.h"

using utils::print_using;

// Global function with one argument.
void print_on(std::ostream& os)
{
  os << "global function without argument.";
}

// Global function with two arguments.
void print_on(std::ostream& os, double d)
{
  os << "global function by value: " << d;
}

// A test class that is not copyable, and has a member function for printing.
class Foo
{
 public:
  Foo() = default;
  Foo(Foo const&) = delete;
  void print_on(std::ostream& os) const { os << "member function"; }
};

// Printing Foo with a global function, using the same name as the above global functions (overloaded 'print_on').
void print_on(std::ostream& os, Foo const&)
{
  os << "global function";
}

// Test printing Foo with different qualifiers.
template<typename F>
void test(F foo)
{
  DoutEntering(dc::notice, "test<" << libcwd::type_info_of<F>().demangled_name() << ">");
  Dout(dc::notice, print_using(foo, print_on));                                                 // Using the global function.
  Dout(dc::notice, print_using(foo, &Foo::print_on));                                           // Using a member function.
  Dout(dc::notice, print_using(foo, [](std::ostream& os, Foo const&){ os << "lambda"; }));      // Using a lambda.
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  // Using a function pointer.
  Dout(dc::notice, print_using(print_on));

  // Printing a builtin type, using a function pointer.
  Dout(dc::notice, print_using(3.14, print_on));

  // Test printing a plain Foo.
  test<Foo>(Foo());

  // Test printing a const Foo.
  test<Foo const>(Foo());

  // Test printing a non-const reference Foo.
  Foo foo;
  test<Foo&>(foo);

  // Test printing a const reference Foo.
  test<Foo const&>(foo);

  // Test printing a pointer to Foo.
  Foo* ptr = &foo;
  test<Foo*>(ptr);

  // Test printing a const pointer to Foo.
  Foo const* const_ptr = &foo;
  test<Foo const*>(const_ptr);

  // Test printing a reference to a (const) pointer to a (const) Foo.
  test<Foo*&>(ptr);
  test<Foo* const&>(ptr);
  test<Foo const*&>(const_ptr);
  test<Foo const* const&>(const_ptr);

  // Test printing a shared ptr.
  std::shared_ptr<Foo> sptr = std::make_shared<Foo>();
  test<std::shared_ptr<Foo>>(sptr);
  test<std::shared_ptr<Foo>&>(sptr);
  test<std::shared_ptr<Foo> const&>(sptr);
}
