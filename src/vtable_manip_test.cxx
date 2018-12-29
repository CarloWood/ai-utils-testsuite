#include "sys.h"
#include "debug.h"
#include "utils/VTPtr.h"
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>
#include <cassert>

class B
{
 public:
  struct VT_type
  {
    int (*_g)(B* self, int n);
    int (*_h)(B* self, double d);
    int (*_pv)(B* self, int x, int y);  // Pure virtual function.
  };

  struct VT_impl
  {
    static int g(B* self, int n) { return self->m_b + n + 42; }
    static int h(B* self, double d) { return self->m_b + d * 42; }

    static constexpr VT_type VT{g, h, nullptr};
  };

  // Make a deep copy of VT_ptr.
  virtual VT_type* clone_VT() { return VT_ptr.clone(this); }

  utils::VTPtr<B> VT_ptr;

  int g(int n) { return VT_ptr->_g(this, n); }
  int h(double d) { return VT_ptr->_h(this, d); }
  int pv(int x, int y) { return VT_ptr->_pv(this, x, y); }

 private:
  int m_b;

 public:
  B(int b) : VT_ptr(this), m_b(b) { }
};

class D : public B
{
 public:
  struct VT_type : B::VT_type
  {
    char const* (*_i)(char const* p, size_t n);
  };

  struct VT_impl : B::VT_impl
  {
    static int h(B* self, double d) { return static_cast<D*>(self)->m_d + d * 20; }            // override.
    static int pv(B* self, int x, int y) { return static_cast<D*>(self)->m_d + x + y; }        // Implement pure virtual.
    static char const* i(char const* p, size_t n) { return p + n; }                            // New virtual function.

    static constexpr VT_type VT{g, h, pv, i};
  };

  // Make a deep copy of VT_ptr.
  VT_type* clone_VT() override { return VT_ptr.clone(this); }

  utils::VTPtr<D, B> VT_ptr;

 private:
  int m_d;

 public:
  D(int d, int b) : B(b), VT_ptr(this), m_d(d) { }

  char const* i(char const* p, size_t n) { return VT_ptr->_i(p, n); }
};

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  B b(123);
  assert(b.g(58) == 223 && b.h(5) == 333);
  D d(234, 345);
  assert(d.g(58) == 445 && d.h(5) == 334 && d.pv(33, 66) == 333);
  B* bp = &d;
  assert(bp->g(58) == 445 && bp->h(5) == 334 && bp->pv(33, 66) == 333);

  Dout(dc::notice, d.i("No Success", 3));
}
