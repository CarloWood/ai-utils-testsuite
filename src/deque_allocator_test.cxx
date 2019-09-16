#include "sys.h"
#include "utils/MemoryPagePool.h"
#include "utils/SimpleSegregatedStorage.h"
#include <deque>
#include <cstdlib>
#include <new>
#include <array>

class AIStatefulTask;

static constexpr size_t glibcxx_deque_buf_size =
#ifdef _GLIBCXX_DEQUE_BUF_SIZE
    _GLIBCXX_DEQUE_BUF_SIZE
#else
    512
#endif
  ;

static utils::MemoryPagePool<0x1000> s_deque_memory_page_pool;       // Returns blocks of 0x1000 bytes.

template<typename T>
struct DequePoolAllocator
{
  using value_type = T;
  using map_pointer_type = T*;

  DequePoolAllocator() = default;
  DequePoolAllocator(DequePoolAllocator const&) { }

  // This allocator also supports allocating blocks with map_pointer_type's, for which we use a normal std::allocator.
  operator std::allocator<map_pointer_type> const&() const noexcept { return m_deque_map_alloc; }
  template<typename U> struct rebind { using other = std::conditional_t<std::is_same_v<U, T>, DequePoolAllocator<T>, std::allocator<U>>; };

  [[nodiscard]] T* allocate(std::size_t n)
  {
//    Dout(dc::notice, "Calling DequePoolAllocator<" << libcwd::type_info_of<T>().demangled_name() << ">::allocate(" << n << ")");
    ASSERT(n <= glibcxx_deque_buf_size / sizeof(T));
    if (auto p = static_cast<T*>(m_sss.allocate([this](){ return add_block(); })))
      return p;
    throw std::bad_alloc();
  }

  void deallocate(T* p, std::size_t n) noexcept
  {
//    Dout(dc::notice, "Calling DequePoolAllocator<" << libcwd::type_info_of<T>().demangled_name() << ">::deallocate(" << p << ", " << n << ")");
    m_sss.deallocate(p);
  }

 protected:
  bool add_block()
  {
    void* const ptr = s_deque_memory_page_pool.allocate();
    if (AI_UNLIKELY(ptr == nullptr))
      return false;
    m_sss.add_block(ptr, s_deque_memory_page_pool.block_size(), glibcxx_deque_buf_size);
    return true;
  }

 private:
  utils::SimpleSegregatedStorage m_sss;
  std::allocator<map_pointer_type> m_deque_map_alloc;
};

template<typename T>
bool operator==(DequePoolAllocator<T> const& alloc1, DequePoolAllocator<T> const& alloc2)
{
  return &alloc1 == &alloc2;
}

template<typename T>
bool operator!=(DequePoolAllocator<T> const& alloc1, DequePoolAllocator<T> const& alloc2)
{
  return &alloc1 != &alloc2;
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  AIStatefulTask* ptr = nullptr;

  DequePoolAllocator<AIStatefulTask*> alloc;
  {
    std::deque<AIStatefulTask*, DequePoolAllocator<AIStatefulTask*>> test_deque(alloc);

    for (int n = 0; n < 10000; ++n)
      test_deque.push_back(ptr);
    for (int j = 0; j < 10; ++j)
    {
      for (int n = 0; n < 9000; ++n)
        test_deque.pop_front();
      for (int n = 0; n < 1000; ++n)
        test_deque.push_back(ptr);
      for (int n = 0; n < 1000; ++n)
        test_deque.pop_front();
      for (int n = 0; n < 9000; ++n)
        test_deque.push_back(ptr);
    }

    AIStatefulTask* res = test_deque.front();
    test_deque.pop_front();

    Dout(dc::notice, "Popped " << res);

    Dout(dc::notice, "Destructing deque...");
  }

  Dout(dc::notice, "Leaving main()...");
}
