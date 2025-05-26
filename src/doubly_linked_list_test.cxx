#include "sys.h"
#include "utils/List.h"
#include <gtest/gtest.h>
#include <list>
#include <vector>
#include <numeric>
#include "debug.h"
#include "debug_ostream_operators.h"

int main(int argc, char** argv)
{
  Debug(NAMESPACE_DEBUG::init());
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

template<typename T>
struct TestList : public utils::List<T>
{
  using utils::List<T>::List;

  // Copy constructor.
  TestList(TestList const& other) : utils::List<T>(other) { }

  // Move constructor.
  TestList(TestList&& other) : utils::List<T>(std::move(other)) { }

  // Copy assignment.
  TestList& operator=(TestList const& orig)
  {
    return static_cast<TestList&>(utils::List<T>::operator=(orig));
  }

  // Move assignment.
  TestList& operator=(TestList&& orig)
  {
    return static_cast<TestList&>(utils::List<T>::operator=(std::move(orig)));
  }

  static typename utils::List<T>::size_type count(utils::List<T>::const_iterator first, utils::List<T>::const_iterator last)
  {
    return utils::List<T>::count(first, last);
  }
};

using list_type = TestList<int>;

// Fixture base class that initializes a vector 'values_' with a sequence of integers.
class ListTestBase : public ::testing::Test
{
 protected:
  std::vector<int> values_;

 protected:
  ListTestBase(int values_size) : values_(values_size)
  {
    // Fill values_ with 1 to values_size.
    std::iota(values_.begin(), values_.end(), 1);
  }

  int size() const
  {
    return static_cast<int>(values_.size());
  }

  void check_empty_list(list_type& list);
};

void ListTestBase::check_empty_list(list_type& list)
{
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.size(), 0);

  list_type::iterator beg = list.begin();
  list_type::iterator end = list.end();
  EXPECT_EQ(beg, end);

  // Member functions specific for utils::List.
  EXPECT_FALSE(beg.is_begin());         // is_begin() returns false if begin() == end().
  EXPECT_TRUE(end.is_end());
  EXPECT_EQ(list.count(list.begin(), list.end()), 0);

  // end() is dereferencable (to base_node_).
  list_type::node_type* beg_node = *reinterpret_cast<list_type::node_type**>(&beg);
  list_type::node_type* end_node = *reinterpret_cast<list_type::node_type**>(&end);
  // References the same object (base_node).
  EXPECT_EQ(beg_node, end_node);

  // Can assign iterator to const_iterator, and dereference a const_iterator to value_type const&.
  list_type::const_iterator const_beg = beg;
#if 0
  list_type::value_type const& const_beg_value = *const_beg;

  // value_type is assignable to underlying type (even though they weren't really dereferencable).
  // Note: this reads *(int*)&size_ because size_ follows base_node_ in value_type.
  int beg_data = beg_value;
  int end_data = end_value;

  // This is expected to compile (and will return the same iterator).
  list_type::iterator next = end_value.next();
  EXPECT_EQ(next, end);
#endif
}

class DoublyLinkedListTest : public ListTestBase
{
 public:
  DoublyLinkedListTest() : ListTestBase(5) { }
};

class ListSpliceTest : public ListTestBase
{
 public:
  ListSpliceTest() : ListTestBase(10) { }
};

class ListEraseTest : public ListTestBase
{
 public:
  ListEraseTest() : ListTestBase(5) { }
};

class ListTest : public ListTestBase
{
 public:
  ListTest() : ListTestBase(10) { }
};

struct IteratorPair
{
  list_type::iterator tst_iterator_;
  std::list<int>::iterator std_iterator_;
};

struct ConstIteratorPair
{
  list_type::const_iterator tst_iterator_;
  std::list<int>::const_iterator std_iterator_;

  ConstIteratorPair(IteratorPair const& iterator_pair) :
    tst_iterator_(iterator_pair.tst_iterator_), std_iterator_(iterator_pair.std_iterator_) { }
};

class ListPair
{
 public:
  list_type tst_list_;
  std::list<int> std_list_;

 public:
  // Constructor that initializes lists with a range from 'values'.
  ListPair(std::vector<int>::iterator begin, std::vector<int>::iterator end) : tst_list_(begin, end), std_list_(begin, end) { }

  // Copy constructor to create copies of both lists.
  ListPair(ListPair const& other) : tst_list_(other.tst_list_), std_list_(other.std_list_) { }

  // Move construct to move both lists.
  ListPair(ListPair&& other) : tst_list_(std::move(other.tst_list_)), std_list_(std::move(other.std_list_)) { }

  // Initializer list.
  ListPair(std::initializer_list<int> const& init) : tst_list_(init), std_list_(init) { }

  template<typename... Args>
  ListPair(Args&&... args) : tst_list_(std::forward<Args>(args)...), std_list_(std::forward<Args>(args)...) { }

  ~ListPair()
  {
    verify_lists();
  }

  ListPair& operator=(ListPair const& other)
  {
    tst_list_ = other.tst_list_;
    std_list_ = other.std_list_;
    return *this;
  }

  ListPair& operator=(ListPair&& other)
  {
    tst_list_ = std::move(other.tst_list_);
    std_list_ = std::move(other.std_list_);
    return *this;
  }

  size_t size() const
  {
    list_type::size_type ret = tst_list_.size();
    ASSERT(ret == std_list_.size());
    return ret;
  }

  bool empty() const
  {
    bool ret = tst_list_.empty();
    ASSERT(ret == std_list_.empty());
    return ret;
  }

  // Verify that both lists are equal.
  void verify_lists();

  // Returns iterators to the nth element in both lists.
  IteratorPair nth_element(int n);

  // Checks if two iterators point to the same relative position in their respective lists.
  void verify(IteratorPair const& it)
  {
    if (it.tst_iterator_ != tst_list_.end() && it.std_iterator_ != std_list_.end())
      EXPECT_EQ(*it.tst_iterator_, *it.std_iterator_);
    else
      EXPECT_EQ(it.tst_iterator_ == tst_list_.end(), it.std_iterator_ == std_list_.end());
  }

  void sort()
  {
    tst_list_.sort();
    std_list_.sort();
  }

  IteratorPair erase(ConstIteratorPair first, ConstIteratorPair last)
  {
    return {
      tst_list_.erase(first.tst_iterator_, last.tst_iterator_),
      std_list_.erase(first.std_iterator_, last.std_iterator_)
    };
  }

  IteratorPair erase(ConstIteratorPair pos)
  {
    return {
      tst_list_.erase(pos.tst_iterator_),
      std_list_.erase(pos.std_iterator_)
    };
  }

  void splice(ConstIteratorPair pos, ListPair& other, ConstIteratorPair first, ConstIteratorPair last)
  {
    tst_list_.splice(pos.tst_iterator_, other.tst_list_, first.tst_iterator_, last.tst_iterator_);
    std_list_.splice(pos.std_iterator_, other.std_list_, first.std_iterator_, last.std_iterator_);
  }

  void splice(ConstIteratorPair pos, ListPair& other)
  {
    tst_list_.splice(pos.tst_iterator_, other.tst_list_);
    std_list_.splice(pos.std_iterator_, other.std_list_);
  }

  IteratorPair insert(ConstIteratorPair pos, int const& value)
  {
    return {
      tst_list_.insert(pos.tst_iterator_, value),
      std_list_.insert(pos.std_iterator_, value)
    };
  }

  IteratorPair insert(ConstIteratorPair pos, size_t const count, int const& value)
  {
    return {
      tst_list_.insert(pos.tst_iterator_, count, value),
      std_list_.insert(pos.std_iterator_, count, value)
    };
  }

  template<typename InputIt>
  IteratorPair insert(ConstIteratorPair pos, InputIt const first, InputIt const last)
  {
    return {
      tst_list_.insert(pos.tst_iterator_, first, last),
      std_list_.insert(pos.std_iterator_, first, last)
    };
  }

  IteratorPair insert(ConstIteratorPair pos, std::initializer_list<int> const ilist)
  {
    return {
      tst_list_.insert(pos.tst_iterator_, ilist),
      std_list_.insert(pos.std_iterator_, ilist)
    };
  }

  std::pair<size_t, size_t> remove(int const& value)
  {
    size_t tst_ret = tst_list_.remove(value);
    size_t std_ret = std_list_.remove(value);
    EXPECT_EQ(tst_ret, std_ret);
    return {tst_ret, std_ret};
  }

  template<typename UnaryPredicate>
  std::pair<size_t, size_t> remove_if(UnaryPredicate p)
  {
    size_t tst_ret = tst_list_.remove_if(p);
    size_t std_ret = std_list_.remove_if(p);
    EXPECT_EQ(tst_ret, std_ret);
    return {tst_ret, std_ret};
  }

  std::pair<size_t, size_t> unique()
  {
    size_t tst_ret = tst_list_.unique();
    size_t std_ret = std_list_.unique();
    EXPECT_EQ(tst_ret, std_ret);
    return {tst_ret, std_ret};
  }

  template<typename BinaryPredicate>
  std::pair<size_t, size_t> unique(BinaryPredicate p)
  {
    size_t tst_ret = tst_list_.unique(p);
    size_t std_ret = std_list_.unique(p);
    EXPECT_EQ(tst_ret, std_ret);
    return {tst_ret, std_ret};
  }

  template<typename Compare>
  void merge(ListPair& other, Compare comp)
  {
    tst_list_.merge(other.tst_list_, comp);
    std_list_.merge(other.std_list_, comp);
  }

  void reverse() noexcept
  {
    tst_list_.reverse();
    std_list_.reverse();
  }

  template<typename Compare>
  void sort(Compare comp)
  {
    tst_list_.sort(comp);
    std_list_.sort(comp);
  }

  void push_back(int const& value)
  {
    tst_list_.push_back(value);
    std_list_.push_back(value);
  }
};

void ListPair::verify_lists()
{
  ASSERT_EQ(tst_list_.size(), std_list_.size());
  ASSERT_TRUE((tst_list_.size() == 0) == tst_list_.empty());

  // Verify the forward pointing links.
  auto it_tst_list = tst_list_.begin();
  auto it_std_list = std_list_.begin();
  int count = 0;
  for (; it_tst_list != tst_list_.end(); ++it_tst_list, ++it_std_list)
  {
    ++count;
    EXPECT_EQ(*it_tst_list, *it_std_list);
  }
  ASSERT_EQ(count, std_list_.size());
  // Also check the backwards pointing links.
  auto rit_tst_list = tst_list_.rbegin();
  auto rit_std_list = std_list_.rbegin();
  count = 0;
  for (; rit_tst_list != tst_list_.rend(); ++rit_tst_list, ++rit_std_list)
  {
    ++count;
    EXPECT_EQ(*rit_tst_list, *rit_std_list);
  }
  ASSERT_EQ(count, std_list_.size());
}

IteratorPair ListPair::nth_element(int n)
{
  auto it_tst_list = tst_list_.begin();
  auto it_std_list = std_list_.begin();
  for (int i = 0; i < n; ++i)
  {
    ++it_tst_list;
    ++it_std_list;
  }
  return {it_tst_list, it_std_list};
}

// Test default construction.
TEST_F(DoublyLinkedListTest, DefaultConstruction)
{
  // Default construction.
  list_type list;

  for (int i = 0; i < 2; ++i)
  {
    check_empty_list(list);

    // Calling clear() on an empty list doesn't change its state.
    list.clear();
  }
  // We're allowed to call clear() multiple times.
  list.clear();
}

TEST_F(DoublyLinkedListTest, PushBack)
{
  list_type list;
  list_type::iterator base_node = list.end();

  list_type::size_type expected_size = 0;
  for (int i = 0; i < 5; ++i)
  {
    int test_value = 42 + 3 * i;

    // Test push_back_node.
    list.push_back(test_value);
    ++expected_size;

    EXPECT_TRUE(!list.empty());
    EXPECT_EQ(list.size(), expected_size);

    list_type::iterator beg = list.begin();
    list_type::iterator end = list.end();
    EXPECT_NE(beg, end);

    // The end iterator was not invalidated.
    EXPECT_EQ(base_node, end);

    // Member functions specific for utils::List.
    EXPECT_TRUE(beg.is_begin());
    EXPECT_TRUE(end.is_end());
    EXPECT_EQ(list.count(list.begin(), list.end()), expected_size);

    // end() is dereferencable (to base_node_).
    list_type::value_type& beg_value = *beg;
    list_type::value_type& end_value = *end;
    // References different objects.
    EXPECT_NE(&beg_value, &end_value);

    // Call prev() and next() expected_size - 1 times.
    list_type::iterator first = end;
    list_type::iterator last = beg;
    for (int j = 0; j < expected_size - 1; ++j)
    {
      first = first->prev();
      last = last->next();
    }
    EXPECT_TRUE(first->prev().is_begin());

    // next() and prev() work as expected.
    EXPECT_EQ(last->next(), end);
    EXPECT_EQ(end_value.prev(), last);

    // value_type is assignable to underlying type.
    int last_data = *last;
    EXPECT_EQ(last_data, test_value);
    // Can also compare directly.
    EXPECT_EQ(*last, test_value);
  }

  list.clear();
  check_empty_list(list);
}

TEST_F(DoublyLinkedListTest, PushFront)
{
  list_type list;
  list_type::iterator base_node = list.end();

  list_type::size_type expected_size = 0;
  for (int i = 0; i < 5; ++i)
  {
    int test_value = 42 + 3 * i;

    // Test insert_node (on the second to fifth call).
    list.push_front(test_value);
    ++expected_size;

    EXPECT_TRUE(!list.empty());
    EXPECT_EQ(list.size(), expected_size);

    list_type::iterator beg = list.begin();
    list_type::iterator end = list.end();
    EXPECT_NE(beg, end);

    // The end iterator was not invalidated.
    EXPECT_EQ(base_node, end);

    // Member functions specific for utils::List.
    EXPECT_TRUE(beg.is_begin());
    EXPECT_TRUE(end.is_end());
    EXPECT_EQ(list.count(list.begin(), list.end()), expected_size);

    // end() is dereferencable (to base_node_).
    list_type::value_type& beg_value = *beg;
    list_type::value_type& end_value = *end;
    // References different objects.
    EXPECT_NE(&beg_value, &end_value);

    // Call prev() and next() expected_size - 1 times.
    list_type::iterator first = end;
    list_type::iterator last = beg;
    for (int j = 0; j < expected_size - 1; ++j)
    {
      first = first->prev();
      last = last->next();
    }
    EXPECT_TRUE(first->prev().is_begin());

    // next() and prev() work as expected.
    EXPECT_EQ(last->next(), end);
    EXPECT_EQ(end_value.prev(), last);

    // value_type is assignable to underlying type.
    int last_data = *beg;
    EXPECT_EQ(last_data, test_value);
    // Can also compare directly.
    EXPECT_EQ(*beg, test_value);
  }

  list.clear();
  check_empty_list(list);
}

TEST_F(ListSpliceTest, EntireList)
{
  for (int size1 = 0; size1 <= 5; ++size1)
  {
    for (int size2 = 0; size2 <= 5; ++size2)
    {
      for (int pos = 0; pos < size1; ++pos)             // Iterate over all possible positions.
      {
        // Create the lists.
        ListPair list_pair1(values_.begin(), values_.begin() + size1);
        ListPair list_pair2(values_.begin() + size1, values_.begin() + size1 + size2);

        // Verify lists also before splicing.
        list_pair1.verify_lists();
        list_pair2.verify_lists();

        // Get iterator to the position to erase.
        IteratorPair it_pos1 = list_pair1.nth_element(pos);

        // Splice the entire list_pair2 into list_pair1.
        list_pair1.splice(it_pos1, list_pair2);
      }
    }
  }
}

TEST_F(ListSpliceTest, SameList)
{
  for (int size = 0; size <= values_.size(); ++size)    // Iterate over all possible list sizes from 0 to 5.
  {
    for (int before = 0; before < size; ++before)       // Iterate over all possible insert positions.
    {
      for (int first = 0; first <= size; ++first)       // Iterate over all possible ranges [first, last).
      {
        for (int last = first; last <= size; ++last)
        {
          // before may not be in the range (that would be Undefined Behavior).
          if (before >= first && before < last)
            continue;

          // Create the list.
          ListPair list_pair(values_.begin(), values_.begin() + size);

          // Verify list also before splicing.
          list_pair.verify_lists();

          IteratorPair it_before = list_pair.nth_element(before);
          IteratorPair it_first  = list_pair.nth_element(first);
          IteratorPair it_last   = list_pair.nth_element(last);

          // Splice the list.
          list_pair.splice(it_before, list_pair, it_first, it_last);
        }
      }
    }
  }
}

TEST_F(ListSpliceTest, OtherList)
{
  for (int size1 = 0; size1 <= 5; ++size1)
  {
    for (int size2 = 0; size2 <= 5; ++size2)
    {
      for (int before = 0; before < size1; ++before)
      {
        for (int first2 = 0; first2 < size2; ++first2)
        {
          for (int last2 = first2; last2 < size2; ++last2)
          {
            // Create the lists.
            ListPair list_pair1(values_.begin(), values_.begin() + size1);
            ListPair list_pair2(values_.begin() + size1, values_.begin() + size1 + size2);

            // Verify lists also before splicing.
            list_pair1.verify_lists();
            list_pair2.verify_lists();

            IteratorPair it_first2 = list_pair2.nth_element(first2);
            IteratorPair it_last2  = list_pair2.nth_element(last2);
            IteratorPair it_before = list_pair1.nth_element(before);

            // Splice the lists.
            list_pair1.splice(it_before, list_pair2, it_first2, it_last2);
          }
        }
      }
    }
  }
}

TEST_F(ListEraseTest, Position)
{
  for (int size = 0; size <= values_.size(); ++size)    // Iterate over all possible list sizes from 0 to 5.
  {
    for (int pos = 0; pos < size; ++pos)                // Iterate over all possible positions.
    {
      // Create the list.
      ListPair list_pair(values_.begin(), values_.begin() + size);

      // Get iterator to the position to erase.
      IteratorPair it_pos = list_pair.nth_element(pos);

      // Erase single element at it_pos.
      IteratorPair ret_it = list_pair.erase(it_pos);

      // Verify return iterator.
      list_pair.verify(ret_it);
    }
  }
}

TEST_F(ListEraseTest, Range)
{
  for (int size = 0; size <= values_.size(); ++size)    // Iterate over all possible list sizes from 0 to 5.
  {
    for (int first = 0; first <= size; ++first)         // Iterate over all possible ranges [first, last).
    {
      for (int last = first; last <= size; ++last)
      {
        // Create the list.
        ListPair list_pair(values_.begin(), values_.begin() + size);

        // Get iterators to the range [first, last).
        IteratorPair it_first = list_pair.nth_element(first);
        IteratorPair it_last  = list_pair.nth_element(last);

        // Erase the range [first, last).
        IteratorPair ret_it = list_pair.erase(it_first, it_last);

        // Verify return iterator.
        list_pair.verify(ret_it);
      }
    }
  }
}

TEST_F(ListTest, MoveConstructor)
{
  for (int size = 0; size <= 5; ++size)
  {
    // Create the initial ListPair with 'size' elements.
    ListPair list_pair_original(values_.begin(), values_.begin() + size);

    // Move-construct a new ListPair from the original.
    ListPair list_pair_moved(std::move(list_pair_original));

    // Verify that the moved-from list is in a valid state.
    EXPECT_TRUE(list_pair_original.empty());
  }
}

TEST_F(ListTest, MoveAssignment)
{
  for (int size = 0; size <= 5; ++size)
  {
    for (int dest_size = 0; dest_size <= 5; ++dest_size)
    {
      // Create the original ListPair with 'size' elements.
      ListPair list_pair_original(values_.begin(), values_.begin() + size);

      // Create the destination ListPair with 'dest_size' elements.
      ListPair list_pair_dest(values_.begin() + size, values_.begin() + size + dest_size);

      // Perform move assignment.
      list_pair_dest = std::move(list_pair_original);

      // Verify that the moved-from list is empty.
      EXPECT_TRUE(list_pair_original.empty());
    }
  }
}

TEST_F(ListTest, InsertSingleElement)
{
  constexpr int insert_value = 42;
  for (int size = 0; size <= 5; ++size)
  {
    for (int pos = 0; pos <= size; ++pos)
    {
      // Create the list.
      ListPair list_pair(values_.begin(), values_.begin() + size);

      // Get iterator to the insertion position.
      IteratorPair it_pos = list_pair.nth_element(pos);

      // Insert the value.
      IteratorPair ret_it = list_pair.insert(it_pos, insert_value);

      // Verify return iterator.
      list_pair.verify(ret_it);
    }
  }
}

TEST_F(ListTest, InsertFill)
{
  constexpr int insert_value = 42;
  for (int size = 0; size <= 5; ++size)
  {
    for (int pos = 0; pos <= size; ++pos)
    {
      for (int count = 0; count <= 3; ++count)
      {
        // Create the list.
        ListPair list_pair(values_.begin(), values_.begin() + size);

        // Get iterator to the insertion position.
        IteratorPair it_pos = list_pair.nth_element(pos);

        // Insert the value count times.
        IteratorPair ret_it = list_pair.insert(it_pos, count, insert_value);

        // Verify return iterator.
        list_pair.verify(ret_it);
      }
    }
  }
}

TEST_F(ListTest, InsertRange)
{
  // Possible ranges to insert.
  std::vector<int> const range = {42, 43, 44, 45, 46};

  for (int size = 0; size <= 5; ++size)
  {
    for (int pos = 0; pos <= size; ++pos)
    {
      for (int range_size = 0; range_size <= range.size(); ++range_size)
      {
        // Create the list.
        ListPair list_pair(values_.begin(), values_.begin() + size);

        // Get iterator to the insertion position.
        IteratorPair it_pos = list_pair.nth_element(pos);

        // Perform the insertion.
        IteratorPair ret_it = list_pair.insert(it_pos, range.begin(), range.begin() + range_size);

        // Verify return iterator.
        list_pair.verify(ret_it);
      }
    }
  }
}

TEST_F(ListTest, InsertInitializerList)
{
  // Possible initializer lists to insert.
  std::vector<std::initializer_list<int>> insert_ilists = {
    {},                  // Empty initializer list.
    {42},                // Single element.
    {42, 43},
    {42, 43, 44},
    {42, 43, 44, 45},
    {42, 43, 44, 45, 46}
  };

  for (int size = 0; size <= 5; ++size)
  {
    for (int pos = 0; pos <= size; ++pos)
    {
      for (auto const& ilist : insert_ilists)
      {
        // Create the list.
        ListPair list_pair(values_.begin(), values_.begin() + size);

        // Get iterator to the insertion position.
        IteratorPair it_pos = list_pair.nth_element(pos);

        // Perform the insertion.
        IteratorPair ret_it = list_pair.insert(it_pos, ilist);

        // Verify return iterator.
        list_pair.verify(ret_it);
      }
    }
  }
}

TEST_F(ListTest, Remove)
{
  constexpr int remove_value = 42;

  // Possible test lists
  std::vector<std::vector<int>> test_lists = {
    {},                            // Empty list
    {1},                           // List without remove_value
    {remove_value},                // List with only remove_value
    {1, remove_value, 3},          // remove_value in the middle
    {remove_value, 2, remove_value}, // remove_value at start and middle
    {1, 2, 3, 4, 5},               // List without remove_value
    {remove_value, remove_value, remove_value}, // List with only remove_value
    {1, remove_value, remove_value, 4, remove_value}, // Multiple remove_values
  };

  for (auto const& elements : test_lists)
  {
    // Create the list.
    ListPair list_pair(elements.begin(), elements.end());

    // Remove the remove_value.
    [[maybe_unused]] auto removed_count = list_pair.remove(remove_value);
  }
}

TEST_F(ListTest, RemoveIf)
{
  for (int size = 0; size <= 5; ++size)
  {
    // Create two lists.
    ListPair list_pair0(values_.begin(), values_.begin() + size);
    ListPair list_pair1(values_.begin(), values_.begin() + size);

    [[maybe_unused]] auto removed0 = list_pair0.remove_if([](int n) { return n % 2 == 0; });
    [[maybe_unused]] auto removed1 = list_pair1.remove_if([](int n) { return n % 2 == 1; });
  }
}

TEST_F(ListTest, Unique)
{
  // Possible test lists.
  std::vector<std::vector<int>> test_lists = {
    {},                           // Empty list
    {1},                          // Single element
    {1, 1},                       // Duplicates
    {1, 1, 2, 2, 3, 3},           // Consecutive duplicates
    {1, 2, 2, 3, 4, 4, 5},        // Mixed duplicates
    {1, 2, 3, 4, 5},              // No duplicates
  };

  for (auto const& elements : test_lists)
  {
    // Create the list.
    ListPair list_pair(elements.begin(), elements.end());

    // Perform unique operation.
    [[maybe_unused]] auto removed_count = list_pair.unique();
  }

  for (int e1 = 1; e1 <= 5; ++e1)
    for (int e2 = 1; e2 <= 5; ++e2)
      for (int e3 = 1; e3 <= 5; ++e3)
        for (int e4 = 1; e4 <= 5; ++e4)
          for (int e5 = 1; e5 <= 5; ++e5)
          {
            ListPair list_pair = {e1, e2, e3, e4, e5};
            [[maybe_unused]] auto removed_count = list_pair.unique();
          }
}

TEST_F(ListTest, UniqueWithPredicate)
{
  // Predicate: consider elements equal if they have the same parity (both even or both odd).
  auto unique_parity = [](int a, int b) { return (a % 2) == (b % 2); };

  // Possible test lists.
  std::vector<std::vector<int>> test_lists = {
    {},                           // Empty list
    {1, 2, 2, 3, 4, 4, 5},        // Mixed duplicates
    {2, 4, 6, 8},                 // All even numbers
    {1, 3, 5, 7},                 // All odd numbers
    {1, 2, 3, 4, 5},              // Alternating even and odd
  };

  for (auto const& elements : test_lists)
  {
    // Create the list.
    ListPair list_pair(elements.begin(), elements.end());

    // Perform unique operation with predicate.
    [[maybe_unused]] auto removed_count = list_pair.unique(unique_parity);
  }

  auto unique_mod3 = [](int a, int b) { return (a % 3) == (b % 3); };

  for (int e1 = 1; e1 <= 5; ++e1)
    for (int e2 = 1; e2 <= 5; ++e2)
      for (int e3 = 1; e3 <= 5; ++e3)
        for (int e4 = 1; e4 <= 5; ++e4)
          for (int e5 = 1; e5 <= 5; ++e5)
          {
            ListPair list_pair = {e1, e2, e3, e4, e5};
            [[maybe_unused]] auto removed_count = list_pair.unique(unique_mod3);
          }
}

TEST_F(ListTest, Merge)
{
  // Possible test cases.
  std::vector<std::pair<std::vector<int>, std::vector<int>>> test_cases = {
    {{}, {}},                            // Both lists empty
    {{1, 3, 5}, {2, 4, 6}},              // Interleaved elements
    {{1, 2, 3}, {}},                     // Second list empty
    {{}, {4, 5, 6}},                     // First list empty
    {{1, 2, 3}, {4, 5, 6}},              // Non-overlapping ranges
    {{1}, {2}},                          // Single-element lists
  };

  for (auto const& [elements1, elements2] : test_cases)
  {
    // Create the lists.
    ListPair list_pair1(elements1.begin(), elements1.end());
    ListPair list_pair2(elements2.begin(), elements2.end());

    // Perform merge operation.
    list_pair1.merge(list_pair2, std::greater<int>{});
  }
}

TEST_F(ListTest, Reverse)
{
  for (int size = 0; size <= 5; ++size)
  {
    // Create the list.
    ListPair list_pair(values_.begin(), values_.begin() + size);

    // Perform reverse operation.
    list_pair.reverse();
  }
}

TEST_F(ListTest, Sort)
{
  std::vector<int> nums = {1, 2, 3, 4, 5};
  do
  {
    for (int size = 0; size <= 5; ++size)
    {
      // Create the list.
      ListPair list_pair;
      for (int e = 0; e < size; ++e)
        list_pair.push_back(nums[e]);

      list_pair.sort(std::greater<int>());
    }
  }
  while (std::next_permutation(nums.begin(), nums.end()));

}
