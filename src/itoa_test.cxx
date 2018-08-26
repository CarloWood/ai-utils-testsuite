#include "utils/itoa.h"
#include <iostream>

int main()
{
  unsigned short x = 0xffff;

  std::array<char, 6> buf;
  char const* a = utils::itoa(buf, x);
  std::cout << a << std::endl;
}
