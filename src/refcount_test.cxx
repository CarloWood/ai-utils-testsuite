#include "utils/AIRefCount.h"
#include <iostream>
#include <boost/intrusive_ptr.hpp>
#include <cassert>

struct Test : public AIRefCount {
  int m_magic;
  Test() : m_magic(0x12345678) { std::cout << "Default constructed" << std::endl; }
  Test(Test const& foo) : AIRefCount(foo) { std::cout << "Copy constructed" << std::endl; }
  ~Test() { m_magic = 0; std::cout << "Destructed" << std::endl; }
  char const* test() const { return m_magic == 0x12345678 ? "OK!" : "DELETED!"; }
};

int main()
{
  boost::intrusive_ptr<Test> p1 = new Test;
  assert(p1->unique() && p1->ref_count() == 1);

  boost::intrusive_ptr<Test> p2 = p1;
  {
    boost::intrusive_ptr<Test> p3(p1);
    assert(!p1->unique() && p3->ref_count() == 3);
    p2.reset();
    assert(!p1->unique() && p3->ref_count() == 2);
  }
  assert(p1->unique() && p1->ref_count() == 1);

  Test* p = p1.get();
  p2 = p;
  boost::intrusive_ptr<Test> p3 = p;
  assert(p1->ref_count() == 3);
  assert(p2->ref_count() == 3);
  assert(p3->ref_count() == 3);

  boost::intrusive_ptr<Test> q1 = new Test;
  boost::intrusive_ptr<Test> q2(q1);
  assert(q1->ref_count() == 2);
  assert(q2->ref_count() == 2);

  std::swap(p2, q2);
  assert(p1->ref_count() == 3);
  assert(p2->ref_count() == 2);     // swapped with q2
  assert(p3->ref_count() == 3);
  assert(q1->ref_count() == 2);
  assert(q2->ref_count() == 3);     // swapped with p2

  boost::intrusive_ptr<Test> r1 = new Test(*p);
  assert(r1->unique());
  // No change:
  assert(p1->ref_count() == 3);
  assert(p2->ref_count() == 2);
  assert(p3->ref_count() == 3);
  assert(q1->ref_count() == 2);
  assert(q2->ref_count() == 3);

  p2 = q2;
  assert(p1->ref_count() == 4);
  assert(p2->ref_count() == 4);     // same as q2
  assert(p3->ref_count() == 4);
  assert(q1->ref_count() == 1);     // decremented with 1.
  assert(q2->ref_count() == 4);     // incremented with 1.

  std::cout << p1->test() << std::endl;
}
