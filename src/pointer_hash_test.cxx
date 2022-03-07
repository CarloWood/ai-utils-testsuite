#include "sys.h"
#include "utils/pointer_hash.h"
#include "utils/popcount.h"
#include <array>
#include <vector>
#include <iostream>
#include <iomanip>
#include <bitset>

int show_equal_bits(std::vector<uint64_t> const& hashes);

int main()
{
  // Test four different 'base' values for pointers.
  std::array<uint64_t, 4> bases = { 0, 0x2080000, 0x7fffffff00000000, 0x5618771908e0 };

  // Run over all base values.
  for (uint64_t base : bases)
  {
    std::cout << "Base: "<< std::hex << base << std::dec << '\n';

    // Generate 256 sequential heap allocation "pointer" values.
    std::vector<uint64_t> ptrs;
    for (uint64_t d = 0; d < 200 * sizeof(int); d += sizeof(int))
      ptrs.push_back(base + d);

    // Calculate the hash between 0 and each of these values.
    std::vector<uint64_t> hashes;
    for (auto ptr : ptrs)
    {
      uint64_t h = utils::pointer_hash((void*)0x0, (void*)ptr);
//      std::cout << std::hex << h << '\n';
      hashes.push_back(h);
    }

    // Find what is the maximum number of bits that are the same for any pair of two hash values.
    int same = show_equal_bits(hashes);
    std::cout << "Found two hashes of 0 and ptr that are equal in " << same << " bits.\n";

    // Calculate hashes for every pair of pointers for each base.
    hashes.clear();
    for (auto ptr1 : ptrs)
      for (auto ptr2 : ptrs)
      {
        if (ptr1 == ptr2)
          continue;
        uint64_t h = utils::pointer_hash((void*)ptr1, (void*)ptr2);
        hashes.push_back(h);
      }

    std::cout << "Calculating equal bits for " << hashes.size() << " hash values..." << std::endl;
    same = show_equal_bits(hashes);
    std::cout << "Found two hashes of ptr pairs that are equal in " << same << " bits.\n";
  }
}

int show_equal_bits(std::vector<uint64_t> const& hashes)
{
  int max_same = 0;
  for (uint64_t h1 : hashes)
    for (uint64_t h2 : hashes)
    {
      if (h1 == h2)
        continue;
      uint64_t equal = ~h1 ^ h2;
      int same = utils::popcount(equal);
//      if (same >= 54)
//        std::cout << "Equal:\n\t" << std::bitset<64>(h1) << "\n\t" << std::bitset<64>(h2) << "\n\t" << std::bitset<64>(equal) << '\n';
      if (same > max_same)
        max_same = same;
    }
  return max_same;
}
