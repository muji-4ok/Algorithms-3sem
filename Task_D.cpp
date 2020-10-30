//
// Created by egork on 14.10.2020.
//
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <unordered_set>

constexpr char kFirstLetter = 'a' - 1;
constexpr size_t kAlphabetSize = 27;

size_t IndexOf(char c) { return c - kFirstLetter; }

//         suffix_array       , classes            , class_count
std::tuple<std::vector<size_t>, std::vector<size_t>, size_t> SortFirstLetter(const std::string &s) {
  std::vector<size_t> suffix_array(s.size());
  std::vector<size_t> classes(s.size());
  std::vector<size_t> counts(kAlphabetSize);

  for (char c : s)
    ++counts[IndexOf(c)];

  for (size_t i = 1; i < kAlphabetSize; ++i)
    counts[i] += counts[i - 1];

  for (size_t i = 0; i < s.size(); ++i)
    suffix_array[--counts[IndexOf(s[i])]] = i;

  // The first suffix will always be the one that starts right on the last character,
  // because it's the lowest one
  classes[0] = 0;
  size_t next_class = 0;

  for (size_t i = 1; i < s.size(); ++i) {
    if (s[suffix_array[i]] != s[suffix_array[i - 1]])
      ++next_class;

    classes[suffix_array[i]] = next_class;
  }

  return {suffix_array, classes, next_class + 1};
}

// Assuming that s ends in the character that is right before 'a' in the encoding table ('a' - 1)
std::vector<size_t> BuildSuffixArray(const std::string &s) {
  auto[suffix_array, classes, classes_count] = SortFirstLetter(s);
  std::vector<size_t> new_suffix_array(s.size());
  std::vector<size_t> new_classes(s.size());

  for (size_t power = 0; (1ull << power) < s.size(); ++power) {
    for (size_t i = 0; i < s.size(); ++i)
      new_suffix_array[i] = (s.size() + suffix_array[i] - (1ull << power)) % s.size();

    std::vector<size_t> counts(classes_count);

    for (size_t class_id : classes)
      ++counts[class_id];

    for (size_t i = 1; i < classes_count; ++i)
      counts[i] += counts[i - 1];

    // Need stability
    for (ssize_t i = s.size() - 1; i >= 0; --i)
      suffix_array[--counts[classes[new_suffix_array[i]]]] = new_suffix_array[i];

    // The first suffix will always be the one that starts right on the last character,
    // because it's the lowest one
    new_classes[0] = 0;
    size_t next_class = 0;

    for (size_t i = 1; i < s.size(); ++i) {
      size_t prev_class_1 = classes[suffix_array[i - 1]];
      size_t prev_class_2 = classes[(suffix_array[i - 1] + (1ull << power)) % s.size()];
      size_t cur_class_1 = classes[suffix_array[i]];
      size_t cur_class_2 = classes[(suffix_array[i] + (1ull << power)) % s.size()];

      if (cur_class_1 != prev_class_1 || cur_class_2 != prev_class_2)
        ++next_class;

      new_classes[suffix_array[i]] = next_class;
    }

    classes_count = next_class + 1;
    classes = new_classes;
  }

  return suffix_array;
}

std::vector<size_t> ReverseSuffixArray(const std::vector<size_t> &suffix_array) {
  std::vector<size_t> result(suffix_array.size());

  for (int i = 0; i < suffix_array.size(); ++i)
    result[suffix_array[i]] = i;

  return result;
}

// result[i] = LCP(suffix_array[i], suffix_array[i + 1])
std::vector<size_t> Kasai(const std::string &s, const std::vector<size_t> &suffix_array) {
  std::vector<size_t> suffix_order = ReverseSuffixArray(suffix_array);
  std::vector<size_t> lcp(s.size() - 1);
  size_t cur_lcp = 0;

  for (size_t i = 0; i < s.size() - 1; ++i) {
    if (suffix_order[i] == s.size() - 1) {
      cur_lcp = 0;
      continue;
    }

    size_t j = suffix_array[suffix_order[i] + 1];

    while (s[i + cur_lcp] == s[j + cur_lcp])
      ++cur_lcp;

    lcp[suffix_order[i]] = cur_lcp;

    if (cur_lcp > 0)
      --cur_lcp;
  }

  return lcp;
}

size_t CountSubStrings(const std::string &s,
                       const std::vector<size_t> &suffix_array,
                       const std::vector<size_t> &lcp) {
  size_t result = 0;

  for (size_t suff_index : suffix_array)
    result += s.size() - suff_index - 1; // not countring the very last sentinel character

  for (size_t lcp_size : lcp)
    result -= lcp_size;

  return result;
}

size_t CountSubStrings(std::string s) {
  s += kFirstLetter;
  std::vector<size_t> suffix_array = BuildSuffixArray(s);
  std::vector<size_t> lcp = Kasai(s, suffix_array);
  return CountSubStrings(s, suffix_array, lcp);
}

int main() {
  std::string s;
  std::cin >> s;
  std::cout << CountSubStrings(s) << '\n';
}

