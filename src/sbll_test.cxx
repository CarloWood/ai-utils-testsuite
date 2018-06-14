#include "sys.h"
#include "utils/sbll.h"

struct XxxxData
{
  int n;

  XxxxData(int n_) : n(n_) { }

  friend bool operator==(XxxxData const& d1, XxxxData const& d2) { return d1.n == d2.n; }
  friend bool operator<(XxxxData const& d1, XxxxData const& d2) { return d1.n < d2.n; }
};

class XxxxNode : public utils::SbllNode<utils::SbllNodeImpl, XxxxNode>
{
  XxxxData m_data;

 public:
  XxxxNode(int data) : m_data(data) { }

 protected:
  lteqgt_nt insert_cmp(XxxxNode const& Xxxx_data) const override
  {
    return Xxxx_data.m_data == m_data ? EQUAL : Xxxx_data.m_data < m_data ? LESS : GREATER;
  }

  bool find_cmp(void const* key) const override
  {
    XxxxData const* data = static_cast<XxxxData const*>(key);
    return m_data == *data;
  }

  void delinked(void) override
  {
    std::cout << "Calling delinked() for node '" << m_data.n << "'" << std::endl;
  }

  void done(void) override
  {
    std::cout << "Calling done() for node '" << m_data.n << "'" << std::endl;
    if (3 <= m_data.n && m_data.n <= 5)
    {
      prev_node()->del();
      next_node()->del();
    }
  }

  void deceased(utils::SbllNodeImpl* UNUSED_ARG(node), void* UNUSED_ARG(label)) override
  {
  }

  LLISTS_METHODS(XxxxNode);
};

using XxxxList = utils::SbllList<utils::SbllListImpl, XxxxNode>;

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  XxxxList foo("Xxxx");

  for (int i = 0; i < 10; ++i)
    foo.add(new XxxxNode(i));

  XxxxData key(4);
  XxxxNode* Xxxx_node = foo.find(&key);
  if (Xxxx_node)
    Xxxx_node->del();
  else
    std::cout << "Could not find 4." << std::endl;

  Dout(dc::notice|flush_cf, "Leaving main()");
}
