#include "utils/utf8_string_to_filename.h"
#include <iostream>
#include <cassert>

int main()
{
  std::u8string msg = u8"This /is/ a उदहारण! It _contains_ a\uFEFF% and ‗"; // "example" in Hindi. \uFEFF is a zero-width no-break space (aka, invisible)

  for (char8_t c : msg)
  {
    std::cout << std::hex << (int)c << ' ';
  }
  std::cout << '\n';

  auto fn = utils::utf8_string_to_filename(msg);

  std::cout << "Encoded: " << fn << '\n';

  auto decoded = utils::utf8_filename_to_string(fn);
  std::cout << "Decoded: \"" << (char const*)decoded.c_str() << "\"\n";

  for (char8_t c : decoded)
  {
    std::cout << std::hex << (int)c << ' ';
  }
  std::cout << '\n';

  assert(decoded == msg);

  // There are 20 different paths an encoded glyph can take.
  // All input characters originate from one of eight possible
  // positions in the VEN diagram made up of the three dictionaries:
  //
  //  F: From dictionary
  //  T: To dictionary
  //  I: Illegal dictionary
  //
  //  1: Not in F, T or I (r)
  //  2: Not in F or T, but in I (q)
  //  3: Not in F or I, but in T (B, F, J and N)             [2]
  //  4: Not in I or T, but in F (i, j, k and l)
  //  5: Not in F but in T and I (D, H, L and P)             [4]
  //  6: Not in T but in F and I (m, n, o and p)
  //  7: Not in I but in F and T (a, b, c and d)             [1]
  //  8: In F, T and I           (e, f, g, h, C, G, K and O) [3]
  //                                                          ^
  //                                                          |
  //  There are four different intersections with T (see diagram below)
  //  There are also four different intersections with F, and from each
  //  a translation can end up in section 1, 2, 3 and 4.
  //  Those four times four possibilities are all the translations
  //  from F (a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) to
  //  their corresponding point in T (A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P).
  //  Furthermore, in the original input could be characters that
  //  are in T but not in F: either not in I: B, F, J and N; or also in
  //  I: D, H, L and P. These eight can thus be both input and output.
  //
  //   From
  //    |
  //    v
  //  .----------------------------------------.
  //  |                                        |
  //  |                                    j--------.
  //  |                                        |    |
  //  |                              i     k   |    |
  //  |                     .--------|-----|---+----|----------------.
  //  |                     |1   a   v     | b--->B v               2|<-- To
  //  |                  .--->M  |   I     |   |    J                |
  //  |                  |  |  E v         |   |                     |
  //  |                  |  |  ^ A   d  c  |   |                     |
  //  |          .-------|--+--|-----|--|--|---+---------.           |
  //  |          |       m  |3 | g   |  v  v   |         |           |
  //  |          |          |  e |   |  C  K   |         |           |
  //  |          | p        |    v   |         |         |           |
  //  |    l     | |   n  o-->O  G h |       f------------->F        |
  //  '----|-----+-|---|----+------|-|---------'         |           |
  //       |     | |   |    |4     v v                   |           |
  //       |     | |   |    |      H D                   |           |
  //       |     | |   `----------------------------------->N        |
  //       |     | `--------->P                          |           |
  //       `----------------->L                          |           |
  //             |          |                            |           |
  //             |          '----------------------------+-----------'
  //             |                                       |
  //             |              q                        |   r
  //             |                                       |
  //             |                                       |<-- Illegal
  //             '---------------------------------------'

  std::u8string const from = u8"abcdefghijklmnop";
  std::u8string const to   = u8"ABCDEFGHIJKLMNOP";
  std::u8string const illegal = u8"efghmnopqCDGHKLOP";
  std::u8string const input = u8"abcdefghijklmnopqrABCDEFGHIJKLMNOP";

  std::cout << "Input: " << (char const*)input.c_str() << '\n';
  auto fn2 = utils::utf8_string_to_filename(input, illegal, from, to);
  std::cout << "Encoded: " << fn2 << '\n';
  auto decoded2 = utils::utf8_filename_to_string(fn2, to, from);
  std::cout << "Decoded: \"" << (char const*)decoded2.c_str() << "\"\n";
  assert(decoded2 == input);
}
