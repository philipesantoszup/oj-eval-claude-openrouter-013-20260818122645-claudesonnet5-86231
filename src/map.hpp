/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   typedef pair<const Key, T> value_type;

  private:
   struct Node {
       value_type *data;
       Node *left, *right, *parent;
       bool red;
       Node() : data(nullptr), left(nullptr), right(nullptr), parent(nullptr), red(true) {}
   };

   Node *root;
   Node *nil;
   size_t size_;
   Compare comp;

   Node *tree_minimum(Node *x) const {
       while (x->left != nil) x = x->left;
       return x;
   }

   Node *tree_maximum(Node *x) const {
       while (x->right != nil) x = x->right;
       return x;
   }

   Node *tree_successor(Node *x) const {
       if (x->right != nil) return tree_minimum(x->right);
       Node *y = x->parent;
       while (y != nil && x == y->right) {
           x = y;
           y = y->parent;
       }
       return y;
   }

   Node *tree_predecessor(Node *x) const {
       if (x->left != nil) return tree_maximum(x->left);
       Node *y = x->parent;
       while (y != nil && x == y->left) {
           x = y;
           y = y->parent;
       }
       return y;
   }

   Node *find_node(const Key &key) const {
       Node *x = root;
       while (x != nil) {
           if (comp(key, x->data->first)) x = x->left;
           else if (comp(x->data->first, key)) x = x->right;
           else return x;
       }
       return nil;
   }

   void left_rotate(Node *x) {
       Node *y = x->right;
       x->right = y->left;
       if (y->left != nil) y->left->parent = x;
       y->parent = x->parent;
       if (x->parent == nil) root = y;
       else if (x == x->parent->left) x->parent->left = y;
       else x->parent->right = y;
       y->left = x;
       x->parent = y;
   }

   void right_rotate(Node *x) {
       Node *y = x->left;
       x->left = y->right;
       if (y->right != nil) y->right->parent = x;
       y->parent = x->parent;
       if (x->parent == nil) root = y;
       else if (x == x->parent->right) x->parent->right = y;
       else x->parent->left = y;
       y->right = x;
       x->parent = y;
   }

   void insert_fixup(Node *z) {
       while (z->parent->red) {
           if (z->parent == z->parent->parent->left) {
               Node *y = z->parent->parent->right;
               if (y->red) {
                   z->parent->red = false;
                   y->red = false;
                   z->parent->parent->red = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->right) {
                       z = z->parent;
                       left_rotate(z);
                   }
                   z->parent->red = false;
                   z->parent->parent->red = true;
                   right_rotate(z->parent->parent);
               }
           } else {
               Node *y = z->parent->parent->left;
               if (y->red) {
                   z->parent->red = false;
                   y->red = false;
                   z->parent->parent->red = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->left) {
                       z = z->parent;
                       right_rotate(z);
                   }
                   z->parent->red = false;
                   z->parent->parent->red = true;
                   left_rotate(z->parent->parent);
               }
           }
       }
       root->red = false;
   }

   void transplant(Node *u, Node *v) {
       if (u->parent == nil) root = v;
       else if (u == u->parent->left) u->parent->left = v;
       else u->parent->right = v;
       v->parent = u->parent;
   }

   void erase_fixup(Node *x) {
       while (x != root && !x->red) {
           if (x == x->parent->left) {
               Node *w = x->parent->right;
               if (w->red) {
                   w->red = false;
                   x->parent->red = true;
                   left_rotate(x->parent);
                   w = x->parent->right;
               }
               if (!w->left->red && !w->right->red) {
                   w->red = true;
                   x = x->parent;
               } else {
                   if (!w->right->red) {
                       w->left->red = false;
                       w->red = true;
                       right_rotate(w);
                       w = x->parent->right;
                   }
                   w->red = x->parent->red;
                   x->parent->red = false;
                   w->right->red = false;
                   left_rotate(x->parent);
                   x = root;
               }
           } else {
               Node *w = x->parent->left;
               if (w->red) {
                   w->red = false;
                   x->parent->red = true;
                   right_rotate(x->parent);
                   w = x->parent->left;
               }
               if (!w->left->red && !w->right->red) {
                   w->red = true;
                   x = x->parent;
               } else {
                   if (!w->left->red) {
                       w->right->red = false;
                       w->red = true;
                       left_rotate(w);
                       w = x->parent->left;
                   }
                   w->red = x->parent->red;
                   x->parent->red = false;
                   w->left->red = false;
                   right_rotate(x->parent);
                   x = root;
               }
           }
       }
       x->red = false;
   }

   void erase_node(Node *z) {
       Node *y = z;
       Node *x;
       bool y_original_red = y->red;
       if (z->left == nil) {
           x = z->right;
           transplant(z, z->right);
       } else if (z->right == nil) {
           x = z->left;
           transplant(z, z->left);
       } else {
           y = tree_minimum(z->right);
           y_original_red = y->red;
           x = y->right;
           if (y->parent == z) {
               x->parent = y;
           } else {
               transplant(y, y->right);
               y->right = z->right;
               y->right->parent = y;
           }
           transplant(z, y);
           y->left = z->left;
           y->left->parent = y;
           y->red = z->red;
       }
       if (!y_original_red) erase_fixup(x);
       delete z->data;
       delete z;
       --size_;
   }

   Node *copy_tree(Node *src, Node *src_nil, Node *parent) {
       if (src == src_nil) return nil;
       Node *node = new Node();
       node->data = new value_type(*src->data);
       node->red = src->red;
       node->parent = parent;
       node->left = copy_tree(src->left, src_nil, node);
       node->right = copy_tree(src->right, src_nil, node);
       return node;
   }

   void destroy(Node *node) {
       if (node == nil) return;
       destroy(node->left);
       destroy(node->right);
       delete node->data;
       delete node;
   }

   void init_nil() {
       nil = new Node();
       nil->red = false;
       nil->left = nil->right = nil->parent = nil;
       root = nil;
   }

  public:
   class const_iterator;
   class iterator {
      public:
       Node *node;
       map *mp;

       iterator() : node(nullptr), mp(nullptr) {}

       iterator(Node *n, map *m) : node(n), mp(m) {}

       iterator(const iterator &other) : node(other.node), mp(other.mp) {}

       iterator operator++(int) {
           iterator tmp = *this;
           ++(*this);
           return tmp;
       }

       iterator &operator++() {
           if (node == nullptr) throw invalid_iterator();
           Node *succ = mp->tree_successor(node);
           node = (succ == mp->nil) ? nullptr : succ;
           return *this;
       }

       iterator operator--(int) {
           iterator tmp = *this;
           --(*this);
           return tmp;
       }

       iterator &operator--() {
           if (node == nullptr) {
               if (mp->root == mp->nil) throw invalid_iterator();
               node = mp->tree_maximum(mp->root);
           } else {
               Node *pred = mp->tree_predecessor(node);
               if (pred == mp->nil) throw invalid_iterator();
               node = pred;
           }
           return *this;
       }

       value_type &operator*() const {
           return *(node->data);
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node && mp == rhs.mp;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node && mp == rhs.mp;
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       value_type *operator->() const noexcept {
           return node->data;
       }
   };

   class const_iterator {
      public:
       Node *node;
       const map *mp;

       const_iterator() : node(nullptr), mp(nullptr) {}

       const_iterator(Node *n, const map *m) : node(n), mp(m) {}

       const_iterator(const const_iterator &other) : node(other.node), mp(other.mp) {}

       const_iterator(const iterator &other) : node(other.node), mp(other.mp) {}

       const_iterator operator++(int) {
           const_iterator tmp = *this;
           ++(*this);
           return tmp;
       }

       const_iterator &operator++() {
           if (node == nullptr) throw invalid_iterator();
           Node *succ = mp->tree_successor(node);
           node = (succ == mp->nil) ? nullptr : succ;
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator tmp = *this;
           --(*this);
           return tmp;
       }

       const_iterator &operator--() {
           if (node == nullptr) {
               if (mp->root == mp->nil) throw invalid_iterator();
               node = mp->tree_maximum(mp->root);
           } else {
               Node *pred = mp->tree_predecessor(node);
               if (pred == mp->nil) throw invalid_iterator();
               node = pred;
           }
           return *this;
       }

       const value_type &operator*() const {
           return *(node->data);
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node && mp == rhs.mp;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node && mp == rhs.mp;
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       const value_type *operator->() const noexcept {
           return node->data;
       }
   };

   map() : size_(0) {
       init_nil();
   }

   map(const map &other) : size_(other.size_) {
       init_nil();
       root = copy_tree(other.root, other.nil, nil);
   }

   map &operator=(const map &other) {
       if (this == &other) return *this;
       clear();
       root = copy_tree(other.root, other.nil, nil);
       size_ = other.size_;
       return *this;
   }

   ~map() {
       clear();
       delete nil;
   }

   T &at(const Key &key) {
       Node *x = find_node(key);
       if (x == nil) throw index_out_of_bound();
       return x->data->second;
   }

   const T &at(const Key &key) const {
       Node *x = find_node(key);
       if (x == nil) throw index_out_of_bound();
       return x->data->second;
   }

   T &operator[](const Key &key) {
       Node *x = find_node(key);
       if (x != nil) return x->data->second;
       return insert(value_type(key, T())).first->second;
   }

   const T &operator[](const Key &key) const {
       return at(key);
   }

   iterator begin() {
       if (root == nil) return end();
       return iterator(tree_minimum(root), this);
   }

   const_iterator cbegin() const {
       if (root == nil) return cend();
       return const_iterator(tree_minimum(root), this);
   }

   iterator end() {
       return iterator(nullptr, this);
   }

   const_iterator cend() const {
       return const_iterator(nullptr, this);
   }

   bool empty() const {
       return size_ == 0;
   }

   size_t size() const {
       return size_;
   }

   void clear() {
       destroy(root);
       root = nil;
       size_ = 0;
   }

   pair<iterator, bool> insert(const value_type &value) {
       Node *y = nil;
       Node *x = root;
       while (x != nil) {
           y = x;
           if (comp(value.first, x->data->first)) x = x->left;
           else if (comp(x->data->first, value.first)) x = x->right;
           else return pair<iterator, bool>(iterator(x, this), false);
       }
       Node *z = new Node();
       z->data = new value_type(value);
       z->left = nil;
       z->right = nil;
       z->parent = y;
       z->red = true;
       if (y == nil) root = z;
       else if (comp(value.first, y->data->first)) y->left = z;
       else y->right = z;
       insert_fixup(z);
       ++size_;
       return pair<iterator, bool>(iterator(z, this), true);
   }

   void erase(iterator pos) {
       if (pos.mp != this || pos.node == nullptr) throw invalid_iterator();
       erase_node(pos.node);
   }

   size_t count(const Key &key) const {
       return find_node(key) == nil ? 0 : 1;
   }

   iterator find(const Key &key) {
       Node *x = find_node(key);
       if (x == nil) return end();
       return iterator(x, this);
   }

   const_iterator find(const Key &key) const {
       Node *x = find_node(key);
       if (x == nil) return cend();
       return const_iterator(x, this);
   }
};

}

#endif
