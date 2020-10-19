//
// Created by egork on 14.10.2020.
//
#include <iostream>
#include <vector>
#include <tuple>
#include <optional>
#include <map>
#include <memory>

struct Node;

struct Transition {
  Transition() = default;
  explicit Transition(Node *node,
                      size_t start,
                      std::optional<size_t> end)
      : node(node), start(start), end(end) {}

  size_t Length() const { return *end - start; }

  std::unique_ptr<Node> node = nullptr;
  // [start, end)
  size_t start = 0;
  std::optional<size_t> end = std::nullopt;
};

struct Node {
  std::map<char, Transition> to;
  Node *suffix_link = nullptr;
  size_t id = 0;
};

struct ActivePoint {
  Node *node = nullptr;
  // c == \0 <=> length = 0 and that we don't have an edge to go to
  char c = '\0';
  size_t length = 0;
};

class SuffixTree {
 public:
  SuffixTree(const std::string &s) : s_(s) {
    ap_.node = &root_;
    AddString();
  }

  void PrintTree(std::ostream &out, size_t first_size);

 private:
  void AddString();

  void AddSymbol(char c, size_t index);
  void FinishString();
  void FinishString(Node &from);
  bool NeedToSplit(char c) const;
  void FixLengthOverflow(size_t index);

  void PrintNode(Node &from,
                 std::ostream &out,
                 size_t first_size,
                 size_t &node_id);

  const std::string &s_;
  Node root_{};
  ActivePoint ap_{};
  size_t remainder_ = 0;
  size_t node_count_ = 0;
};

void SuffixTree::AddString() {
  ++node_count_;

  for (size_t i = 0; i < s_.size(); ++i)
    AddSymbol(s_[i], i);

  FinishString();
}

bool SuffixTree::NeedToSplit(char c) const {
  if (ap_.c)
    return s_[ap_.node->to.at(ap_.c).start + ap_.length] != c;
  else
    return ap_.node->to.find(c) == ap_.node->to.end();
}

void SuffixTree::FixLengthOverflow(size_t index) {
  while (ap_.c && ap_.node->to[ap_.c].end.has_value()
      && ap_.length > *ap_.node->to[ap_.c].end - ap_.node->to[ap_.c].start) {
    ActivePoint new_ap{};
    size_t node_len = ap_.node->to[ap_.c].Length();
    char next_c = s_[index - ap_.length + node_len];
    new_ap.node = ap_.node->to[ap_.c].node.get();
    new_ap.length = ap_.length - node_len;
    new_ap.c = next_c;
    ap_ = new_ap;
  }

  if (ap_.c) {
    Transition &to = ap_.node->to[ap_.c];

    if (to.end.has_value() && ap_.length == *to.end - to.start) {
      ap_.node = to.node.get();
      ap_.length = 0;
      ap_.c = '\0';
    }
  }
}

void SuffixTree::AddSymbol(char c, size_t index) {
  ++remainder_;
  Node *prev_created = nullptr;

  while (remainder_ > 0) {
    if (NeedToSplit(c)) {
      if (ap_.c) {
        Transition &to = ap_.node->to[ap_.c];

        size_t split_end = to.start + ap_.length;

        auto other_node = std::make_unique<Node>();
        to.node.swap(other_node);
        to.node->to.emplace(
            std::make_pair(s_[split_end], Transition(other_node.release(), split_end, to.end))
        );
        to.end = split_end;
        to.node->to.emplace(
            std::make_pair(c, Transition(new Node(), index, std::nullopt))
        );

        node_count_ += 2;

        if (prev_created)
          prev_created->suffix_link = to.node.get();

        prev_created = to.node.get();
      } else {
        Node *new_node = new Node();
        ap_.node->to.emplace(
            std::make_pair(c, Transition(new_node, index, std::nullopt))
        );
        ++node_count_;

        if (prev_created && ap_.node != &root_)
          prev_created->suffix_link = ap_.node;

        prev_created = nullptr;
      }

      --remainder_;

      if (ap_.node == &root_) {
        if (ap_.length > 0)
          --ap_.length;

        ap_.c = ap_.length ? s_[index - remainder_ + 1] : '\0';
      } else {
        ap_.node = ap_.node->suffix_link ? ap_.node->suffix_link : &root_;
      }

      FixLengthOverflow(index);
    } else {
      if (!ap_.c)
        ap_.c = c;

      ++ap_.length;

      if (prev_created && ap_.node != &root_)
        prev_created->suffix_link = ap_.node;

      prev_created = nullptr;

      FixLengthOverflow(index);
      break;
    }
  }
}

void SuffixTree::FinishString() {
  FinishString(root_);
}

void SuffixTree::FinishString(Node &from) {
  for (auto &it : from.to) {
    if (!it.second.end.has_value())
      it.second.end = s_.size();

    FinishString(*it.second.node);
  }
}

void SuffixTree::PrintTree(std::ostream &out, size_t first_size) {
  std::cout << node_count_ << '\n';
  size_t node_id = 0;
  PrintNode(root_, out, first_size, node_id);
}

void SuffixTree::PrintNode(Node &from,
                           std::ostream &out,
                           size_t first_size,
                           size_t &node_id) {
  from.id = node_id;
  ++node_id;

  for (const auto &it : from.to) {
    bool from_first = it.second.start < first_size;
    size_t str_id = from_first ? 0 : 1;
    size_t start = from_first ? it.second.start : it.second.start - first_size;
    size_t end = from_first ? std::min(*it.second.end, first_size) : *it.second.end - first_size;
    out << from.id << ' ' << str_id << ' ' << start << ' ' << end << '\n';
    PrintNode(*it.second.node, out, first_size, node_id);
  }
}

int main() {
  std::ios_base::sync_with_stdio(false);
  std::string s;
  std::string t;
  std::cin >> s >> t;

  std::string combined = s + t;

  SuffixTree suffix_tree(combined);
  suffix_tree.PrintTree(std::cout, s.size());
}
