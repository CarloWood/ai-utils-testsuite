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
  TestList& operator=(TestList const& orig) = delete;
//  {
//    return static_cast<TestList&>(utils::List<T>::operator=(orig));
//  }

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

  ListPair& operator=(ListPair const&& other)
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

  IteratorPair insert(ConstIteratorPair pos, size_t count, int const& value)
  {
    return {
      tst_list_.insert(pos.tst_iterator_, count, value),
      std_list_.insert(pos.std_iterator_, count, value)
    };
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

      // The moved-to list (list_pair_dest) will be verified upon destruction.
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
  list_type lst = {1, 4};
  std::vector<int> vec = {2, 3, 3, 5};
  lst.insert(std::next(lst.begin()), vec.begin(), vec.end());
  std::array<int, 6> expected = {{1, 2, 3, 3, 5, 4}};
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}

TEST_F(ListTest, InsertInitializerList)
{
  list_type lst = {1, 4};
  lst.insert(std::next(lst.begin()), {2, 3});
  std::array<int, 4> expected = {{1, 2, 3, 4}};
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}

TEST_F(ListTest, Remove)
{
  list_type lst = {1, 2, 3, 2, 4, 2};
  size_t removed = lst.remove(2);
  ASSERT_EQ(removed, 3);
  std::array<int, 3> expected = {{1, 3, 4}};
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}

TEST_F(ListTest, RemoveIf)
{
  list_type lst = {1, 2, 3, 4, 5, 6};
  size_t removed = lst.remove_if([](int n) { return n % 2 == 0; });
  ASSERT_EQ(removed, 3);
  int expected[] = {1, 3, 5};
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}

TEST_F(ListTest, Unique)
{
  list_type lst = {1, 1, 2, 2, 3, 3};
  size_t removed = lst.unique();
  std::array<int, 3> expected = {{1, 2, 3}};
  ASSERT_EQ(removed, 3);
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}

TEST_F(ListTest, UniqueWithPredicate)
{
  list_type lst = {1, 2, 2, 3, 4, 4, 5};
  size_t removed = lst.unique([](int a, int b) { return (a % 2) == (b % 2); });
  ASSERT_EQ(removed, 2);
  std::array<int, 5> expected = {{1, 2, 3, 4, 5}};
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}

TEST_F(ListTest, Merge)
{
  list_type lst1 = {1, 3, 5};
  list_type lst2 = {2, 4, 6};
  lst1.merge(lst2);
  std::array<int, 6> expected = {{1, 2, 3, 4, 5, 6}};
  ASSERT_TRUE(std::equal(lst1.begin(), lst1.end(), std::begin(expected)));
  check_empty_list(lst2);
}

TEST_F(ListTest, MergeWithComparator)
{
  list_type lst1 = {5, 3, 1};
  list_type lst2 = {6, 4, 2};
  lst1.merge(lst2, std::greater<int>());
  std::array<int, 6> expected = {{6, 5, 4, 3, 2, 1}};
  ASSERT_TRUE(std::equal(lst1.begin(), lst1.end(), std::begin(expected)));
  check_empty_list(lst2);
}

TEST_F(ListTest, Reverse)
{
  list_type lst = {1, 2, 3, 4, 5};
  lst.reverse();
  std::array<int, 5> expected = {{5, 4, 3, 2, 1}};
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}

TEST_F(ListTest, SortEmpty)
{
  list_type lst;
  lst.sort();
  check_empty_list(lst);
}

TEST_F(ListTest, SortOne)
{
  ListPair list_pair = { 42 };
  ASSERT_EQ(list_pair.size(), 1);
  list_pair.sort();
}

TEST_F(ListTest, SortTwo)
{
  ListPair list_pair1 = { 3, 7 };
  ASSERT_EQ(list_pair1.size(), 2);
  list_pair1.sort();
  ListPair list_pair2 = { 7, 3 };
  list_pair2.sort();
}

TEST_F(ListTest, Sort)
{
  list_type lst = {3, 1, 4, 1, 5, 9};
  lst.sort();
  std::array<int, 6> expected = {{1, 1, 3, 4, 5, 9}};
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}

TEST_F(ListTest, SortWithComparator)
{
  list_type lst = {3, 1, 4, 1, 5, 9};
  lst.sort(std::greater<int>());
  std::array<int, 6> expected = {{9, 5, 4, 3, 1, 1}};
  ASSERT_TRUE(std::equal(lst.begin(), lst.end(), std::begin(expected)));
}
