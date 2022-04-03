#include <cstddef>
#include <iostream>
#include <stack>
#include <vector>

template <typename T>
class AVL {
 public:
  bool empty() { return (nNodes == 0); }
  size_t height() { return (Node::getHeight(rootNode)); }
  size_t size() { return (nNodes); }

  bool has(const T& v) {
    if (rootNode != nullptr)
      return (rootNode->has(v));
    else
      return (false);
  }
  void insert(const T& v) {
    if (rootNode == nullptr)
      rootNode = new Node(v);
    else
      rootNode = rootNode->insert(v);
    nNodes++;
  }
  void erase(const T& v) {
    if (rootNode == nullptr) return;
    bool found = false;
    rootNode = rootNode->erase(v, found);
    if (found) nNodes--;
  }
  void print_in_order() {
    if (rootNode == nullptr) return;
    std::stack<Node*> s;

    Node* currentNode = rootNode;
    while(currentNode != nullptr || !s.empty()) {
      while(currentNode != nullptr) {
        s.push(currentNode);
        currentNode = currentNode->left;
      }
      currentNode = s.top();
      s.pop();
      std::cout << currentNode->value << " ";
      currentNode = currentNode->right;
    }
    std::cout << std::endl;
  }

 private:
  struct Node {
    Node* left{nullptr};
    Node* right{nullptr};
    T value;
    size_t height{1};

    Node() = delete;
    Node(const T& v) : value(v) {}

    bool has(const T& v) {
      if (value == v)
        return (true);
      else if (v < value && left != nullptr)
        return (left->has(v));
      else if (v > value && right != nullptr)
        return (right->has(v));
      else
        return (false);
    }
    Node* insert(const T& v) {
      if (v <= value) {
        if (left == nullptr)
          left = new Node(v);
        else
          left = left->insert(v);
      } else {
        if (right == nullptr)
          right = new Node(v);
        else
          right = right->insert(v);
      }
      return (update());
    }
    Node* erase(const T& v, bool& found) {
      if (value == v) {
        found = true;
        if (left == nullptr && right == nullptr) {
          delete this;
          return (nullptr);
        } else if (left == nullptr) {
          Node* aux = right;
          *this = *right;
          delete aux;
        } else if (right == nullptr) {
          Node* aux = left;
          *this = *left;
          delete aux;
        } else {
          std::stack<Node*> trace;

          Node* currentNode = left;
          while (currentNode != 0) {
            trace.push(currentNode);
            currentNode = currentNode->right;
          }

          currentNode = trace.top();
          value = currentNode->value;
          Node* lsubtree = currentNode->left;
          delete currentNode;
          trace.pop();

          if (trace.empty()) {
            left = lsubtree;
          } else {
            trace.top()->right = lsubtree;
            trace.pop();
            while (!trace.empty()) {
              currentNode = trace.top();
              currentNode->right = currentNode->right->update();
              trace.pop();
            }
          }
        }
        return (update());
      } else if (v < value) {
        if (left != nullptr) {
          left = left->erase(v, found);
          return (update());
        } else {
          return (this);
        }
      } else {
        if (right != nullptr) {
          right = right->erase(v, found);
          return (update());
        } else {
          return (this);
        }
      }
    }
    Node* update() {
      updateHeight();

      auto bf = getBF(this);
      if (bf >= 2) {
        if (getBF(left) <= -1) LR();
        return (LL());
      } else if (bf <= -2) {
        if (getBF(right) >= 1) RL();
        return (RR());
      } else {
        return (this);
      }
    }

    void LR() {
      Node* lrcopy_ = left->right;
      left->right = lrcopy_->left;
      lrcopy_->left = left;
      left = lrcopy_;
      left->left->updateHeight();
      left->updateHeight();
      updateHeight();
    }
    void RL() {
      Node* rlcopy_ = right->left;
      right->left = rlcopy_->right;
      rlcopy_->right = right;
      right = rlcopy_;
      right->right->updateHeight();
      right->updateHeight();
      updateHeight();
    }
    Node* LL() {
      Node* lcopy_ = left;
      left = left->right;
      lcopy_->right = this;
      if (lcopy_->left != nullptr) lcopy_->left->updateHeight();
      if (lcopy_->right != nullptr) lcopy_->right->updateHeight();
      lcopy_->updateHeight();
      return (lcopy_);
    }
    Node* RR() {
      Node* rcopy_ = right;
      right = right->left;
      rcopy_->left = this;
      if (rcopy_->left != nullptr) rcopy_->left->updateHeight();
      if (rcopy_->right != nullptr) rcopy_->right->updateHeight();
      rcopy_->updateHeight();
      return (rcopy_);
    }

    static size_t getHeight(Node* node) {
      return (node == nullptr ? 0 : node->height);
    }
    static long getBF(Node* node) {
      if (node == nullptr) return (0);
      auto left_height = getHeight(node->left);
      auto right_height = getHeight(node->right);

      long difference =
          static_cast<long>(left_height) - static_cast<long>(right_height);
      return (difference);
    }
    void updateHeight() {
      height = std::max(getHeight(left), getHeight(right)) + 1;
    }
  };

  Node* rootNode{nullptr};
  size_t nNodes{0};
};

int main(int argc, const char* argv[]) {
  AVL<int> avl{};
  std::vector<int> data{5, -1, 2, 1112, 503, -7, 5, 19};
  for(const auto v : data)
    avl.insert(v);
  
  if(avl.size() != 8) {
    std::cout << "Failed: avl.size() != 8" << std::endl;
    return(EXIT_FAILURE);
  }
  if(avl.empty()) {
    std::cout << "Failed: avl.empty()" << std::endl;
    return(EXIT_FAILURE);
  }
  for(const auto v : data) {
    if(!avl.has(v)) {
      std::cout << "Failed: !avl.has(" << v << ")" << std::endl;
      return(EXIT_FAILURE);
    }
  }
  avl.print_in_order();

  for(const auto v : data)
    avl.erase(v);
  
  if(!avl.empty()) {
    std::cout << "Failed: !avl.empty()" << std::endl;
    return(EXIT_FAILURE);
  }
  if(avl.size() != 0) {
    std::cout << "Failed: avl.size() != 0" << std::endl;
    return(EXIT_FAILURE);
  }
  if(avl.height() != 0) {
    std::cout << "Failed: avl.height() != 0" << std::endl;
    return(EXIT_FAILURE);
  }

  return(EXIT_SUCCESS);
}
