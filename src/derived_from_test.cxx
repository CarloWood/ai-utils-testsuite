#include "sys.h"
#include "utils/HasDerivedFrom.h"

// Test.

struct AB
{
  short m_base;
  AB(int base) : m_base(base) { }

 public:
  virtual void print_on(std::ostream& os) const { os << m_base; }
  friend std::ostream& operator<<(std::ostream& os, AB const& ab) { ab.print_on(os); return os; }
};

class AC : public utils::HasDerivedFrom<AC, AB>
{
 public:
  int m_a;
  int m_b;
  void* padding;

 public:
  AC(int a, int b) : utils::HasDerivedFrom<AC, AB>(m_derived), m_a(a), m_b(b) { }
  void print() const;

 private:
  // This must be the last member:
  utils::DerivedFrom<AB> m_derived;
};

// User code.

struct AT : public AB
{
  int m_test1;
  short m_test2;

  AT(int a, int b, short c) : AB(a), m_test1(b), m_test2(c) { }
  ~AT() { std::memset(reinterpret_cast<char*>(this), 0x66, sizeof(AT)); }

  void print_on(std::ostream& os) const override
  {
    AB::print_on(os);
    os << ' ' << m_test1 << ' ' << m_test2;
  }
};

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  boost::intrusive_ptr<AC> a = AC::create<AT>(1, 2)(3, 4, 5); // 1, 2: constuctor args of AC; 3, 4, 5: constructor args of AT.
  a->print();
}

void AC::print() const
{
  std::cout << m_a << ' ' << m_b << ' ' << m_derived << std::endl;
}
