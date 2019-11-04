#include "sys.h"
#include "debug.h"
#include "utils/is_pointer_like.h"
#include <vector>
#include <memory>
#include <iostream>

template<typename T>
struct TestNone
{
  static int const n;
};

template<typename T>
struct TestRef : public TestNone<T>
{
  T const& operator*() { return TestNone<T>::n; }
};

template<typename T>
struct TestArrow : public TestNone<T>
{
  TestArrow const* operator->() { return this; }
};

template<typename T>
struct TestBoth : virtual public TestRef<T>, virtual public TestArrow<T>
{
};

//static
template<typename T>
int const TestNone<T>::n = 42;

template<typename T>
void test()
{
  Dout(dc::notice|continued_cf, '"' << libcwd::type_info_of<T>().demangled_name() << "\" is ");
  bool something = true;
  if (utils::is_pointer_like_v<T>)
    Dout(dc::continued, "pointer-like.");
  else
  {
    if (utils::is_pointer_like_arrow_dereferencable_v<T>)
      Dout(dc::continued, "arrow dereferencable; ");
    else
      something = utils::is_pointer_like_dereferencable_v<T>;
    if (utils::is_pointer_like_dereferencable_v<T>)
      Dout(dc::continued, "dereferencable; ");
  }
  if (!something)
    Dout(dc::continued, "not a pointer or pointer-like.");
  Dout(dc::finish, "");
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  test<int>();
  test<int*>();
  test<std::vector<int>>();
  test<std::vector<int>*>();
  test<std::shared_ptr<int>>();
  test<std::shared_ptr<int>*>();
  test<std::shared_ptr<int***>>();
  test<TestNone<int>>();
  test<TestNone<int>*>();
  test<TestRef<int>>();
  test<TestRef<int>*>();
  test<TestArrow<int>>();
  test<TestArrow<int>*>();
  test<TestBoth<int>>();
  test<TestBoth<int>*>();
}
