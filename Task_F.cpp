//
// Created by egork on 14.10.2020.
//
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <tuple>
#include <optional>
#include <map>
#include <set>
#include <memory>

#define DEBUG

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
  size_t first_count = 0;
  size_t second_count = 0;
  size_t common_count = 0;

  std::string label() const {
//    return std::to_string(id);
    return "\"" + std::to_string(id) + " : " + std::to_string(common_count) + "\"";
  }
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

  void ComputeCommonStrings(size_t first_length);
  std::string FindKthCommon(size_t k);

 private:
  void AddString();
  void AddSymbol(char c, size_t index);
  void FinishString();
  void FinishString(Node &from);
  bool NeedToSplit(char c) const;
  void FixLengthOverflow(size_t index);

  std::tuple<bool, bool> ComputeCommonStrings(Node &from,
                                              size_t first_length,
                                              size_t to_start,
                                              size_t to_end);

 public:
  void ExportToDot(const std::string &s, size_t read_size);
  void ExportToDot(std::ostream &out, const std::string &s, size_t read_size);
  void ExportNodeToDot(Node &from,
                       std::ostream &out,
                       const std::string &s,
                       size_t read_size,
                       size_t &node_id);
  void ExportSuffLinks(Node &from, std::ostream &out);

 public:
  const std::string &s_;
  Node root_{};
  ActivePoint ap_{};
  size_t remainder_ = 0;
  size_t node_count_ = 0;
  size_t step_id_ = 0;
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

void SuffixTree::ComputeCommonStrings(size_t first_length) {
  ComputeCommonStrings(root_, first_length, 0, 0);
}

std::tuple<bool, bool> SuffixTree::ComputeCommonStrings(Node &from,
                                                        size_t first_length,
                                                        size_t to_start,
                                                        size_t to_end) {
  from.first_count = 0;
  from.second_count = 0;
  from.common_count = 0;

  bool has_first = false;
  bool has_second = false;

  for (auto &it : from.to) {
    if (it.second.node->to.empty()) {
      if (it.second.start < first_length) {
        has_first = true;
        it.second.node->first_count = first_length - 1 - it.second.start;
        it.second.node->second_count = 0;
      } else {
        has_second = true;
        it.second.node->first_count = 0;
        it.second.node->second_count = *it.second.end - 1 - it.second.start;
      }
    } else {
      auto[first, second] = ComputeCommonStrings(*it.second.node,
                                                 first_length,
                                                 it.second.start,
                                                 *it.second.end);
      has_first = has_first || first;
      has_second = has_second || second;

      if (first && second)
        from.common_count += it.second.node->common_count;
    }

    from.first_count += it.second.node->first_count;
    from.second_count += it.second.node->second_count;
  }

  if (&from != &root_ && has_first && has_second)
    from.common_count += to_end - to_start;

  if (&from != &root_ && has_first)
    from.first_count += to_end - to_start;

  if (&from != &root_ && has_second)
    from.second_count += to_end - to_start;

  return {has_first, has_second};
}

// first string starts at 0
std::string SuffixTree::FindKthCommon(size_t k) {
  if (k >= root_.common_count)
    return "";

  std::string result;
  Node *cur = &root_;
  size_t to_start = 0;
  size_t to_end = 0;

  while (true) {
    // invariant: k < cur->common_count
    for (const auto &it : cur->to)
      if (k < it.second.node->common_count) {
        cur = it.second.node.get();
        result += s_.substr(to_start, to_end - to_start);
        to_start = it.second.start;
        to_end = *it.second.end;
        break;
      } else {
        k -= it.second.node->common_count;
      }

    if (k < to_end - to_start)
      return result + s_.substr(to_start, k + 1);
    else
      k -= to_end - to_start;
  }
}

std::set<std::string> ComputeSubstringsNaive(const std::string &s) {
  std::set<std::string> substrings;

  for (size_t i = 0; i < s.size(); ++i)
    for (size_t j = 1; j <= s.size() - i; ++j)
      substrings.insert(s.substr(i, j));

  return substrings;
}

void SuffixTree::ExportToDot(const std::string &s, size_t read_size) {
#ifndef DEBUG
  return;
#endif

  std::ofstream file("step_" + std::to_string(step_id_++) + ".dot");
  ExportToDot(file, s, read_size);
}

void SuffixTree::ExportToDot(std::ostream &out, const std::string &s, size_t read_size) {
  out << "digraph G {\n";
  out << "rankdir = LR;\n";
  out << "nodesep = 0.5;\n";
  size_t node_id = 0;
  root_.id = node_id;
  ExportNodeToDot(root_, out, s, read_size, node_id);
  ExportSuffLinks(root_, out);
  out << "}\n";
}

void SuffixTree::ExportNodeToDot(Node &from,
                                 std::ostream &out,
                                 const std::string &s,
                                 size_t read_size,
                                 size_t &node_id) {
  if (&from == ap_.node)
    out << from.label() << R"( [color="red", style="filled"])" << "\n";

  for (const auto &it : from.to) {
    std::string label;
    size_t end_index = it.second.end.value_or(read_size) - it.second.start;

    if (&from == ap_.node && it.first == ap_.c)
      label = s.substr(it.second.start, ap_.length) + "|"
          + s.substr(it.second.start + ap_.length, end_index - ap_.length);
    else
      label = s.substr(it.second.start, end_index);

    it.second.node->id = node_id++;

    out << from.label() << " -> " << it.second.node->label() << " [label=\"" << label
        << "\"";
    out << R"(, color="blue"])";
    out << '\n';

    ExportNodeToDot(*it.second.node, out, s, read_size, node_id);
  }
}

void SuffixTree::ExportSuffLinks(Node &from, std::ostream &out) {
  if (from.suffix_link)
    out << from.label() << " -> " << from.suffix_link->label() << R"( [style="dotted"])" << "\n";

  for (const auto &it : from.to)
    ExportSuffLinks(*it.second.node, out);
}

int main() {
  std::ios_base::sync_with_stdio(false);
  std::string s;
  std::string t;
  std::cin >> s >> t;
//  s = "jaskldfjwoikskskskskskkkkkkwkwaiiiqu";
//  t = "ajajajajjaaaaiiaiujwjkwkwkjskskjkskskskskksksks";
//  auto subs_s = ComputeSubstringsNaive(s);
//  auto subs_t = ComputeSubstringsNaive(t);
//  std::vector<std::string> common_subs;
//  std::set_intersection(subs_s.begin(),
//                        subs_s.end(),
//                        subs_t.begin(),
//                        subs_t.end(),
//                        std::inserter(common_subs, common_subs.begin()));
//  std::cout << common_subs.size() << '\n';
  s += "$";
  t += "#";
  size_t k;
  std::cin >> k;
  --k;

  std::string combined = s + t;

  SuffixTree suffix_tree(combined);
  suffix_tree.ComputeCommonStrings(s.size());
  std::string result = suffix_tree.FindKthCommon(k);

  if (result.empty())
    std::cout << "-1\n";
  else
    std::cout << result << '\n';

//  suffix_tree.ExportToDot(combined, combined.size());
//  std::cout << suffix_tree.root_.first_count << '\n';
//  std::cout << suffix_tree.root_.second_count << '\n';
//  std::cout << suffix_tree.root_.common_count << '\n';

//  std::vector<std::string> common_algo;
//  common_algo.reserve(suffix_tree.root_.common_count);

//  if (suffix_tree.FindKthCommon(1099999871).empty())
//    std::cout << "-1\n";

//  for (size_t k = 0; k < suffix_tree.root_.common_count; ++k)
//    common_algo.push_back(suffix_tree.FindKthCommon(k));

//  if (common_subs != common_algo) {
//    std::cout << "REAL:\n";
//
//    for (const auto &sub : common_subs)
//      std::cout << sub << '\n';
//
//    std::cout << "ALGO:\n";
//
//    for (const auto &sub : common_algo)
//      std::cout << sub << '\n';
//  } else {
//    std::cout << "All good\n";
//  }
}
