#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

struct ZBlock {
  int left;
  int right;
};

std::vector<int> z_func(const std::string &s) {
  int n = s.size();
  std::vector<int> z_value(n, 0);
  z_value[0] = n;
  ZBlock right_most_block{0, 0};

  for (int i = 1; i < n; ++i) {
    int j = i;

    if (i <= right_most_block.right) {
      if (z_value[i - right_most_block.left] + i < right_most_block.right) {
        z_value[i] = z_value[i - right_most_block.left];
        continue;
      }

      j = std::min(right_most_block.right + 1, n);
    }
    while (j < s.size() && s[j] == s[j - i])
      ++j;

    z_value[i] = j - i;
    right_most_block = {i, j - 1};
  }

  return z_value;
}

template<typename Visitor>
void find_matches(const std::string &haystack,
                  const std::string &needle,
                  Visitor &&visitor) {
  int n = haystack.size();
  std::vector<int> needle_z_value = z_func(needle);
  ZBlock right_most_block{0, -1};

  for (int i = 0; i < n; ++i) {
    int j = i;

    if (i <= right_most_block.right) {
      if (needle_z_value[i - right_most_block.left] + i < right_most_block.right) {
        if (needle_z_value[i - right_most_block.left] == needle.size())
          visitor(i);
        continue;
      }

      j = std::min(right_most_block.right + 1, n);
    }

    while (j < n && j - i < needle.size() && haystack[j] == needle[j - i])
      ++j;

    if (j - i == needle.size())
      visitor(i);

    right_most_block = {i, j - 1};
  }
}

int main() {
  std::string needle, haystack;
  std::cin >> needle >> haystack;
  find_matches(haystack, needle, [](int i) { std::cout << i << ' '; });
  std::cout << '\n';

  return 0;
}
