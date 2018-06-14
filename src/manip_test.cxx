#include "sys.h"
#include "utils/iomanip.h"
#include "debug.h"

#include <sstream>

struct A
{
  long m_iword;
  void* m_pword;
  bool m_object_used;

  A(long iword, void* pword, bool object_used) : m_iword(iword), m_pword(pword), m_object_used(object_used) { }
  friend std::ostream& operator<<(std::ostream& os, A const& a);
};

//=============================================================================
// Custom ostream manipulator

class Words : public utils::iomanip::Unsticky<2>
{
 private:
  static utils::iomanip::Index s_index;

 public:
  Words(long iword, void* pword) : Unsticky(s_index, iword, pword) { }
  Words(long iword) : Unsticky(s_index, iword) { }
  Words(void* pword) : Unsticky(s_index, pword) { }

  static long ivalue(std::ostream& os) { return get_iword_from(os, s_index); }
  static void* pvalue(std::ostream& os) { return get_pword_from(os, s_index); }
};

//static
utils::iomanip::Index Words::s_index;

class MyObject : public utils::iomanip::Object<2>
{
 private:
  static utils::iomanip::Index s_index;
  long m_magic;

 public:
  MyObject() : Object(s_index), m_magic(0x28549123) { }
  ~MyObject() { m_magic = 0; }

  void print_on(std::ostream& os) const
  {
    // Should not use deleted objects.
    ASSERT(m_magic == 0x28549123);
    os << "OK ";
  }

  static MyObject* ptr(std::ostream& os) { return static_cast<MyObject*>(Object::ptr(os, s_index)); }
};

//static
utils::iomanip::Index MyObject::s_index;

//=============================================================================

int main()
{
  std::ostringstream ss;

  try
  {
    // Manipulators passed by lvalue (reference) will be *Sticky*.
    Words words(1234, (void*)0x1234);
    Words iword(5678);
    Words pword((void*)0x5678);
    // But not when it is a manipulator derived from Object.
    MyObject obj;

    // Print without manipulators.
    ss << "0. " << A(0, nullptr, false) << '\n';
    // Print with sticky manipulator.
    ss << "1. " << words << A(1234, (void*)0x1234, false) << '\n';
    // Print with (unsticky) MyObject manipulator.
    ss << "2. " << obj << A(1234, (void*)0x1234, true) << '\n';

    // Print with temporary (unsticky) manipulator.
    std::cout << "3. " << Words(8765, nullptr) << A(8765, nullptr, false) << ", " << A(8765, nullptr, false) << '\n';
    // Values should be reset here.
    std::cout << "3.1 " << A(0, nullptr, false) << '\n';

    // Print without manipulators. The values should still be those of 'words' (which is still not destructed...)
    ss << "3.5 " << A(1234, (void*)0x1234, true) << '\n';
    // Print with Unsticky manipulator while previous one is in effect.
    ss << "3.6 " << Words(8765, nullptr) << A(8765, nullptr, true) << ", " << A(8765, nullptr, true) << '\n';
    // Print without manipulators. The values should restored to those of 'words' (which is still not destructed...)
    ss << "3.7 " << A(1234, (void*)0x1234, true) << '\n';

    std::cout << "4. " << MyObject() << A(0, nullptr, true) << ", " << A(0, nullptr, true) << '\n';
    // Print without manipulators. Values should still be zero.
    std::cout << "5. " << A(0, nullptr, false) << '\n';

    // Print rvalue (unsticky) manipulator.
    std::cout << "6. " << std::move(words) << A(1234, (void*)0x1234, false) << '\n';
    // Not reset yet because words isn't destructed yet.
    std::cout << "6.1 " << A(1234, (void*)0x1234, false) << '\n';

    std::cout << "7. " << std::move(obj) << A(1234, (void*)0x1234, true) << '\n';

    ss << "7. " << obj << A(1234, (void*)0x1234, true) << '\n';
  }
  catch(const std::system_error& e) {
        std::cout << "Caught system_error with code " << e.code() 
                  << " meaning " << e.what() << '\n';
  }
  // words was destructed but since it is sticky, we still expect the manipulator to be in effect.
  ss << "8. " << A(1234, (void*)0x1234, false) << std::endl;
  // words was destructed and only used in an unsticky way (passed by rvalue reference). So, values should be reset again.
  std::cout << "9. " << A(0, nullptr, false) << '\n';
  std::cout << "stringstream:\n" << ss.str() << std::endl;
}

std::ostream& operator<<(std::ostream& os, A const& DEBUG_ONLY(a))
{
  MyObject* my_object{MyObject::ptr(os)};

  if (my_object)
    my_object->print_on(os);
  // Should only get here when we expect that.
  ASSERT(!my_object != a.m_object_used);
  ASSERT(Words::ivalue(os) == a.m_iword);
  ASSERT(Words::pvalue(os) == a.m_pword);
  return os << "OK ";
}
