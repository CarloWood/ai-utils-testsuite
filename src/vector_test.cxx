#include "sys.h"
#include "utils/Vector.h"
#include "debug.h"
#include <iostream>

using namespace utils;
using vector_type = Vector<int>;

int main()
{
  vector_type v(100, -1);
  ASSERT(v.size() == 100);

  vector_type::index_type index{v.iend()};      // index == 100
  vector_type::index_type i;
  i = --index;                                  // i == index == 99
  ASSERT(v[i] == -1);
  v[i--] = 42;                                  // v[99] = 42; i == 98
  ASSERT(index.get_value() == 99);
  ASSERT(v[index] == 42);
  ASSERT(i.get_value() == 98);
  ASSERT(v[i] == -1);
  ASSERT(i != index);

  std::cout << "v[" << index << "] = " << v[index] << std::endl;
  std::cout << "v[" << i << "] = " << v[i] << std::endl;

  ++i;                                          // i == 99
  ASSERT(i.get_value() == 99);
  ASSERT(i == index);
}
