//
// Created by egork on 24.09.2020.
//
#include <iostream>
#include <array>
#include <cassert>
#include <algorithm>
#include <vector>
#include <chrono>
#include <iomanip>
#include <unordered_map>

constexpr int ALPHABET_SIZE = 26;
constexpr int ROOT_ID = 0;

int CharNum(char c) { return c - 'a'; }

struct Node {
  Node(int id, int parent_id, char parent_char, bool is_terminal)
      : id(id), parent_id(parent_id), parent_char(parent_char), is_terminal(is_terminal) {
    std::fill(children_ids.begin(), children_ids.end(), -1);
    std::fill(go_ids.begin(), go_ids.end(), -1);
  }

  // id == -1 means nonexistent node
  int id;
  int parent_id;
  int suff_link_id = -1;
  int compressed_suff_link_id = -1;
  char parent_char;
  bool is_terminal;
  std::array<int, ALPHABET_SIZE> children_ids;
  std::array<int, ALPHABET_SIZE> go_ids;
  std::vector<int> string_ids;
};

class Trie {
 public:
  Trie() : nodes({Node(ROOT_ID, -1, 0, false)}) {}
  void AddString(const std::string &s, int id);
  Node *GetNode(int id) {
    assert(0 <= id && id < nodes.size());
    return nodes.data() + id;
  }
  Node *Root() { return GetNode(ROOT_ID); }
  int GetGo(int from_id, char c);
  int GetSuffLink(int from_id);
  int GetCompressedSuffLink(int from_id);

 private:
  std::vector<Node> nodes;
};

void Trie::AddString(const std::string &s, int id) {
  int cur_node_id = ROOT_ID;

  for (char c : s) {
    if (GetNode(cur_node_id)->children_ids[CharNum(c)] == -1) {
      GetNode(cur_node_id)->children_ids[CharNum(c)] = nodes.size();
      nodes.emplace_back(nodes.size(), cur_node_id, c, false);
    }

    cur_node_id = GetNode(cur_node_id)->children_ids[CharNum(c)];
  }

  GetNode(cur_node_id)->is_terminal = true;
  GetNode(cur_node_id)->string_ids.push_back(id);
}

int Trie::GetGo(int from_id, char c) {
  Node *from = GetNode(from_id);

  if (from->go_ids[CharNum(c)] == -1) {
    if (from->children_ids[CharNum(c)] != -1)
      from->go_ids[CharNum(c)] = from->children_ids[CharNum(c)];
    else if (from_id == ROOT_ID)
      from->go_ids[CharNum(c)] = ROOT_ID;
    else
      from->go_ids[CharNum(c)] = GetGo(GetSuffLink(from_id), c);
  }

  return from->go_ids[CharNum(c)];
}

int Trie::GetSuffLink(int from_id) {
  Node *from = GetNode(from_id);

  if (from->suff_link_id == -1) {
    if (from_id == ROOT_ID || from->parent_id == ROOT_ID)
      from->suff_link_id = ROOT_ID;
    else
      from->suff_link_id = GetGo(GetSuffLink(from->parent_id), from->parent_char);
  }

  return from->suff_link_id;
}

int Trie::GetCompressedSuffLink(int from_id) {
  Node *from = GetNode(from_id);

  if (from->compressed_suff_link_id == -1) {
    int suff_link_id = GetSuffLink(from_id);

    if (GetNode(suff_link_id)->is_terminal)
      from->compressed_suff_link_id = suff_link_id;
    else if (suff_link_id == ROOT_ID)
      from->compressed_suff_link_id = ROOT_ID;
    else
      from->compressed_suff_link_id = GetCompressedSuffLink(suff_link_id);
  }

  return from->compressed_suff_link_id;
}

template<typename Visitor>
void FindOccurences(const std::string &haystack,
                    const std::vector<std::string> &needles,
                    Visitor &&visitor) {
  Trie trie;

  for (int i = 0; i < needles.size(); ++i)
    trie.AddString(needles[i], i);

  int cur_node_id = ROOT_ID;

  for (int i = 0; i < haystack.size(); ++i) {
    char c = haystack[i];
    cur_node_id = trie.GetGo(cur_node_id, c);
    int terminal_node_id =
        trie.GetNode(cur_node_id)->is_terminal ? cur_node_id : trie.GetCompressedSuffLink(
            cur_node_id);

    while (terminal_node_id != ROOT_ID) {
      for (int needle_id : trie.GetNode(terminal_node_id)->string_ids)
        visitor(i - needles[needle_id].size() + 1, needle_id);

      terminal_node_id = trie.GetCompressedSuffLink(terminal_node_id);
    }
  }
}

std::tuple<std::vector<std::string>, std::vector<int>> SplitRegex(const std::string &regex) {
  std::vector<std::string> needles;
  std::vector<int> regex_positions;
  std::string cur_needle;

  for (int i = 0; i < regex.size(); ++i) {
    char c = regex[i];

    if (c == '?') {
      if (!cur_needle.empty()) {
        needles.push_back(cur_needle);
        regex_positions.push_back(i - cur_needle.size());
        cur_needle.clear();
      }
    } else {
      cur_needle.push_back(c);
    }
  }

  if (!cur_needle.empty()) {
    needles.push_back(cur_needle);
    regex_positions.push_back(regex.size() - cur_needle.size());
  }

  return std::tuple(needles, regex_positions);
}

template<typename Visitor>
void FindRegexMatches(const std::string &haystack, const std::string &regex, Visitor &&visitor) {
  auto[needles, regex_positions] = SplitRegex(regex);
  std::vector<int> needle_counts(haystack.size(), 0);

  if (!needles.empty()) {
    FindOccurences(haystack, needles, [&](int hay_position, int needle_id) {
      int index = hay_position - regex_positions[needle_id];

      if (index >= 0)
        ++needle_counts[index];
    });

    for (int i = 0; i < haystack.size(); ++i)
      if (needle_counts[i] == needles.size() && haystack.size() - i >= regex.size())
        visitor(i);
  } else {
    // only '?' in regex
    for (int i = 0; i < static_cast<int>(haystack.size()) - static_cast<int>(regex.size()) + 1; ++i)
      visitor(i);
  }
}

int main() {
  std::string haystack, regex;
  std::cin >> regex >> haystack;
  FindRegexMatches(haystack, regex, [](int index) { std::cout << index << ' '; });
}

