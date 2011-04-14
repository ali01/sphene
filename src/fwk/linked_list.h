/* Invasive linked list implementation */
#ifndef LINKED_LIST_H_1I3JTK98
#define LINKED_LIST_H_1I3JTK98

#include <set>

#include "ptr_interface.h"

namespace Fwk {

template <typename NodeType> /* NodeType must inherit from LinkedList::Node */
class LinkedList : public PtrInterface<LinkedList<NodeType> > {
 public:
  typedef Fwk::Ptr<const LinkedList<NodeType> > PtrConst;
  typedef Fwk::Ptr<LinkedList<NodeType> > Ptr;

  class Node : public PtrInterface<Node> {
   public:
    typedef Fwk::Ptr<const Node> PtrConst;
    typedef Fwk::Ptr<Node> Ptr;

    static Ptr New() {
      return new Node();
    }

    Fwk::Ptr<NodeType> next() const { return next_; }
    Fwk::Ptr<NodeType> prev() const { return prev_; }

   protected:
    Node() : next_(NULL), prev_(NULL) {}

    /* Data members. */
    Fwk::Ptr<NodeType> next_;
    NodeType* prev_; /* weak pointer to prevent circular references */

    friend class LinkedList;

    /* Operations disallowed */
    Node(const Node&);
    void operator=(const Node&);
  };

  /* public constructor allows compile-time allocation */
  LinkedList() : front_(NULL), back_(NULL) {}

  static Ptr New() {
    return new LinkedList();
  }

  Fwk::Ptr<NodeType> front() const { return front_; }
  Fwk::Ptr<NodeType> back() const { return back_; }

  void pushFront(Fwk::Ptr<NodeType> node);
  void pushBack(Fwk::Ptr<NodeType> node);
  Fwk::Ptr<NodeType> del(Fwk::Ptr<NodeType> node);
  size_t size() const { return node_set_.size(); }

 protected:
  bool validateNode(Fwk::Ptr<NodeType> node) const;

 private:
  /* Data members. */
  Fwk::Ptr<NodeType> front_;
  Fwk::Ptr<NodeType> back_;
  std::set<Node*> node_set_; /* Prevents double insertion of a node. */

  /* Operations disallowed. */
  LinkedList(const LinkedList&);
  void operator=(const LinkedList&);
};


template <typename NodeType>
inline void
LinkedList<NodeType>::pushFront(Fwk::Ptr<NodeType> node) {
  if (!validateNode(node))
    return;

  node_set_.insert(node.ptr());

  if (front_)
    front_->prev_ = node.ptr();

  node->prev_ = NULL;
  node->next_ = front_;
  front_ = node;

  if (back_ == NULL)
    back_ = front_;
}

template <typename NodeType>
inline void
LinkedList<NodeType>::pushBack(Fwk::Ptr<NodeType> node) {
  if (!validateNode(node))
    return;

  node_set_.insert(node.ptr());

  if (back_)
    back_->next_ = node;

  node->prev_ = back_.ptr();
  node->next_ = NULL;
  back_ = node;

  if (front_ == NULL)
    front_ = back_;
}

template <typename NodeType>
inline Fwk::Ptr<NodeType>
LinkedList<NodeType>::del(Fwk::Ptr<NodeType> node) {
  if (node == NULL || front_ == NULL)
    return NULL;

  /* If node is the head node. */
  if (front_ == node)
    front_ = node->next_;
  else
    node->prev_->next_ = node->next_;

  /* If node is the tail node. */
  if (back_ == node)
    back_ = node->prev_;
  else
    node->next_->prev_ = node->prev_;

  node_set_.erase(node.ptr());

  return node->next_;
}

template <typename NodeType>
inline bool
LinkedList<NodeType>::validateNode(Fwk::Ptr<NodeType> node) const {
  if (node == NULL)
    return false;

  /* If node is already in the linked list: if (non-empty and in set). */
  if (front_ && node_set_.find(node.ptr()) != node_set_.end())
    return false;

  return true;
}

} /* end of namespace Fwk */

#endif
