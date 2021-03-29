#include "sys.h"
#include "utils/ColorPool.h"
#include "debug.h"
#include <string>

int main()
{
  Debug(debug::init());
  // Fake history:
  // 0 1 2 3 4 5 6 7
  int input[] = {    0,1,2,0,7,4,3,6,0,3,2,0,5,2,3,1,7,4,6,0,5,2,3,1 };
  //   | | |   |       |     |
  //   | | |   |       |     `-+---+-------+-+-+-+-+. etc
  //   | | |   |       `-----+-+---+---+-+-+-+.     |
  //   | | |   `-+-----+-+-+-+-+-+.           |     |
  //   | | `-+-+-+-+-+-+-+.       |           |     |
  //   | `-+-+-+-+-+-+-+. |       |           |     |
  //   `-+-+-+-+-+-+-+. | |       |           |     |
  // Result:          1 2 3 3 3 3 5 5 5 5 5 5 1 1 1 7 4 6 0 5 2 3 1 7

  utils::ColorPool<8> s;
  std::string result;
  for (auto c : input)
  {
    s.use_color(c);
    result += '0' + s.get_color();
  }
  Dout(dc::notice, result);
  ASSERT(result == "123333555555111746052317");
  Dout(dc::notice, "Success");
}
