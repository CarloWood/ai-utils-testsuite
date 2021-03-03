#include "sys.h"
#include "utils/Register.h"
#include "debug.h"

struct A {
  int n;
};

struct B {
  int n;
};

static constexpr A a1 = { 1 };
static constexpr A a2 = { 2 };

static constexpr B b1 = { 1 };
static constexpr B b2 = { 2 };

void callback(size_t n, A const& a)
{
  std::cout << a.n << '/' << n << std::endl;
}

void callback(size_t n, B const& b, char const* message)
{
  std::cout << b.n << '/' << n << " [" << message << "]" << std::endl;
}

namespace {
utils::Register<A> a1_([](size_t n){ callback(n, a1); });
utils::Register<A> a2_([](size_t n){ callback(n, a2); });
utils::Register<B> b1_([](size_t n){ callback(n, b1, "b1"); });
utils::Register<B> b2_([](size_t n){ callback(n, b2, "b2"); });
}

int main()
{
  Debug(debug::init());
  utils::RegisterGlobals::finish_registration();
}
