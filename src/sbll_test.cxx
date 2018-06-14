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
  utils::lteqgt_nt insert_cmp(XxxxNode const& Xxxx_data) const override
  {
    return Xxxx_data.m_data == m_data ? utils::EQUAL : Xxxx_data.m_data < m_data ? utils::LESS : utils::GREATER;
  }

  bool find_cmp(void const* key) const override
  {
    XxxxNode const* ptr = static_cast<XxxxNode const*>(key);
    return m_data == ptr->m_data;
  }

  void delinked(void) override
  {
  }

  void done(void) override
  {
  }

  void deceased(utils::SbllNodeImpl* UNUSED_ARG(node), void* UNUSED_ARG(label)) override
  {
  }

  DEBUGLLISTS_LLISTS_METHODS(XxxxNode);
};

using XxxxList = utils::SbllList<utils::SbllListImpl, XxxxNode>;

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  XxxxList foo("Xxxx");
  XxxxNode* Xxxx_node;
  XxxxNode* bar;

  bar = new XxxxNode(42);
  foo.add(bar);
  Xxxx_node = foo.find(bar);
  Xxxx_node->del();
}
