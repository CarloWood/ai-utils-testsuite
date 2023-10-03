#include "sys.h"
#include "utils/ObjectTracker.h"
#include "debug.h"

struct Node;

class NodeTracker : public utils::ObjectTracker<Node>
{
 public:
  NodeTracker(utils::Badge<utils::TrackedObject<NodeTracker>>, Node* tracked) : utils::ObjectTracker<Node>(tracked) { }
};

class Node : public utils::TrackedObject<NodeTracker>
{
 private:
  std::string s_;

 public:
  Node(std::string const& s) : s_(s) { }

  std::string const& s() const { return s_; }
};

int main()
{
  Debug(NAMESPACE_DEBUG::init());
  Dout(dc::notice, "Entered main()...");

  Node node("This is a long string so that it won't be put inside the std::string.");
  std::weak_ptr<NodeTracker> node_tracker = node;
  Node node2(std::move(node));
  ASSERT(node.s().empty());
  std::cout << "tracked.s = " << node_tracker.lock()->tracked_object().s() << std::endl;

  Dout(dc::notice, "Leaving main()...");
}
