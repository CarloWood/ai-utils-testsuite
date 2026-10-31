#include "sys.h"
#include "utils/to_string.h"
#include "enchantum/bitwise_operators.hpp"      // ENCHANTUM_DEFINE_BITWISE_FOR
#include "debug.h"

enum class Bits : std::uint32_t
{
  bitA,
  bitB,
  bitC
};

ENCHANTUM_DEFINE_BITWISE_FOR(Bits);

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  Dout(dc::notice, utils::to_string(Bits::bitB));

  Bits combination = Bits::bitB | Bits::bitC;
  Dout(dc::notice, utils::to_string(combination));
}
