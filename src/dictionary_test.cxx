#include "sys.h"
#include "utils/Dictionary.h"
#include "utils/Vector.h"
#include "debug.h"
#include <vector>

struct Data;

#if 0

enum enum_type {
  e0, e1, e2
};

using index_type = utils::VectorIndex<enum_type>;
using container_type = utils::Vector<Data, index_type>;

#define IS_UTILS_VECTOR

#else

using enum_type = int;
constexpr enum_type e0 = 0;
constexpr enum_type e1 = 1;
constexpr enum_type e2 = 2;

using index_type = int;
using container_type = std::vector<Data>;

#endif

struct Data {
  std::string m_name;
  Data(index_type, std::string&& name) : m_name(std::move(name)) { }
};

int main()
{
  Debug(debug::init());

  utils::DictionaryData<enum_type, container_type, index_type> dictionary;

  index_type i1{1};
  Data const data1(i1, "ONE");

  dictionary.add(e0, "zero", {index_type{0}, "ZERO"});
  dictionary.add(e1, "one", data1);
  dictionary.add(e2, "two", {index_type{2}, "two"});

  index_type i2 = dictionary.index("two");
  index_type i3 = dictionary.index("three");

  if (i2 == index_type{e2})
    std::cout << "Comparing is possible.\n";

#ifdef IS_UTILS_VECTOR
  std::cout << i2 << " [" << dictionary.word(i2.get_value()) << "], " << i3 << " [" << dictionary.word(i3.get_value()) << "]\n";
#else
  std::cout << i2 << " [" << dictionary.word(i2) << "], " << i3 << " [" << dictionary.word(i3) << "]\n";
#endif

  Data& data_one = dictionary[e1];
  std::cout << data_one.m_name << '\n';
}
