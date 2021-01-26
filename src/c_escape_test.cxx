#include "sys.h"
#include "debug.h"
#include "utils/c_escape_iterator.h"
#include <string>
#include <string_view>
#include <iostream>

int main()
{
  Debug(debug::init());

  std::string s = "-=-\t\v\xff-=-\n";
  std::string_view const sv(s);

  using it_escaped_t = utils::c_escape_iterator<std::string_view::const_iterator>;

  std::string escaped(it_escaped_t(sv.begin(), sv.end()), it_escaped_t(sv.end(), sv.end()));
  std::cout << "escaped: \"" << escaped << "\"." << std::endl;
}
