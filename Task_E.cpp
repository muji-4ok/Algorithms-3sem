//
// Created by egork on 14.10.2020.
//
#include <iostream>
#include <vector>
#include <fstream>
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
  SuffixTree(const std::string &s) : s(s) {
    ap.node = &root;
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

  const std::string &s;
  Node root{};
  ActivePoint ap{};
  size_t remainder = 0;
  size_t node_count = 0;
};

void SuffixTree::AddString() {
  ++node_count;

  for (size_t i = 0; i < s.size(); ++i)
    AddSymbol(s[i], i);

  FinishString();
}

bool SuffixTree::NeedToSplit(char c) const {
  if (ap.c)
    return s[ap.node->to.at(ap.c).start + ap.length] != c;
  else
    return ap.node->to.find(c) == ap.node->to.end();
}

void SuffixTree::FixLengthOverflow(size_t index) {
  while (ap.c && ap.node->to[ap.c].end.has_value()
      && ap.length > *ap.node->to[ap.c].end - ap.node->to[ap.c].start) {
    ActivePoint new_ap{};
    size_t node_len = ap.node->to[ap.c].Length();
    char next_c = s[index - ap.length + node_len];
    new_ap.node = ap.node->to[ap.c].node.get();
    new_ap.length = ap.length - node_len;
    new_ap.c = next_c;
    ap = new_ap;
  }

  if (ap.c) {
    Transition &to = ap.node->to[ap.c];

    if (to.end.has_value() && ap.length == *to.end - to.start) {
      ap.node = to.node.get();
      ap.length = 0;
      ap.c = '\0';
    }
  }
}

void SuffixTree::AddSymbol(char c, size_t index) {
  ++remainder;
  Node *prev_created = nullptr;

  while (remainder > 0) {
    if (NeedToSplit(c)) {
      if (ap.c) {
        Transition &to = ap.node->to[ap.c];

        size_t split_end = to.start + ap.length;

        auto other_node = std::make_unique<Node>();
        to.node.swap(other_node);
        to.node->to.emplace(
            std::make_pair(s[split_end], Transition(other_node.release(), split_end, to.end))
        );
        to.end = split_end;
        to.node->to.emplace(
            std::make_pair(c, Transition(new Node(), index, std::nullopt))
        );

        node_count += 2;

        if (prev_created)
          prev_created->suffix_link = to.node.get();

        prev_created = to.node.get();
      } else {
        Node *new_node = new Node();
        ap.node->to.emplace(
            std::make_pair(c, Transition(new_node, index, std::nullopt))
        );
        ++node_count;

        if (prev_created && ap.node != &root)
          prev_created->suffix_link = ap.node;

        prev_created = nullptr;
      }

      --remainder;

      if (ap.node == &root) {
        if (ap.length > 0)
          --ap.length;

        ap.c = ap.length ? s[index - remainder + 1] : '\0';
      } else {
        ap.node = ap.node->suffix_link ? ap.node->suffix_link : &root;
      }

      FixLengthOverflow(index);
    } else {
      if (!ap.c)
        ap.c = c;

      ++ap.length;

      if (prev_created && ap.node != &root)
        prev_created->suffix_link = ap.node;

      prev_created = nullptr;

      FixLengthOverflow(index);
      break;
    }
  }
}

void SuffixTree::FinishString() {
  FinishString(root);
}

void SuffixTree::FinishString(Node &from) {
  for (auto &it : from.to) {
    if (!it.second.end.has_value())
      it.second.end = s.size();

    FinishString(*it.second.node);
  }
}

void SuffixTree::PrintTree(std::ostream &out, size_t first_size) {
  std::cout << node_count << '\n';
  size_t node_id = 0;
  PrintNode(root, out, first_size, node_id);
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
