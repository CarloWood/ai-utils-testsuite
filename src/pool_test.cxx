#include "sys.h"
#include "debug.h"
#include "utils/NodeMemoryPool.h"
#include <vector>
#include <algorithm>

unsigned int seed = 4321;

struct Foo {
  char a[120];
};

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  utils::NodeMemoryPool pool(128);

  size_t num_mallocs = 0;
  std::vector<void*> ptrs;
  for (int i = 0; i < 1025; ++i)
  {
    void* ptr = pool.malloc<Foo>();
    ++num_mallocs;
    if (i % 2 == 0)
      pool.free(ptr);
    else
      ptrs.push_back(ptr);
  }
  Dout(dc::notice, "Number of allocated chunks now " << ptrs.size() << "; " << pool);
  ASSERT(ptrs.size() == 1025 / 2);
  for (int i = 0; i < 1023; ++i)
  {
    void* ptr = pool.malloc<Foo>();
    ++num_mallocs;
    ptrs.push_back(ptr);
  }
  Dout(dc::notice, "Number of allocated chunks now " << ptrs.size() << "; " << pool);
  ASSERT(ptrs.size() == 1025 / 2 + 1023);
  std::vector<unsigned int> index(ptrs.size());
  for (unsigned int i = 0; i < ptrs.size(); ++i)
    index[i] = i;
  std::random_shuffle(index.begin(), index.end());
  for (unsigned int i = 0; i < ptrs.size(); ++i)
    pool.free(ptrs[index[i]]);

  Dout(dc::notice, "Did " << num_mallocs << " allocations and frees.");
  Dout(dc::notice, pool);
  Dout(dc::notice, "Leaving main()...");
}
