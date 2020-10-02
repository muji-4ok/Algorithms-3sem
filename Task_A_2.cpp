#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

struct ZBlock {
  int left;
  int right;
};

std::vector<int> zFunc(const std::string &s) {
  int n = s.size();
  std::vector<int> zValue(n, 0);
  zValue[0] = n;
  ZBlock rightMostBlock{0, 0};

  for (int i = 1; i < n; ++i) {
    int j = i;

    if (i <= rightMostBlock.right) {
      if (zValue[i - rightMostBlock.left] + i < rightMostBlock.right) {
        zValue[i] = zValue[i - rightMostBlock.left];
        continue;
      }

      j = std::min(rightMostBlock.right + 1, n);
    }
    while (j < s.size() && s[j] == s[j - i])
      ++j;

    zValue[i] = j - i;
    rightMostBlock = {i, j - 1};
  }

  return zValue;
}

template<typename Visitor>
void findMatches(const std::string &haystack,
                 const std::string &needle,
                 Visitor &&visitor) {
  int n = haystack.size();
  std::vector<int> needleZValue = zFunc(needle);
  ZBlock rightMostBlock{0, -1};

  for (int i = 0; i < n; ++i) {
    int j = i;

    if (i <= rightMostBlock.right) {
      if (needleZValue[i - rightMostBlock.left] + i < rightMostBlock.right) {
        if (needleZValue[i - rightMostBlock.left] == needle.size())
          visitor(i);
        continue;
      }

      j = std::min(rightMostBlock.right + 1, n);
    }

    while (j < n && j - i < needle.size() && haystack[j] == needle[j - i])
      ++j;

    if (j - i == needle.size())
      visitor(i);

    rightMostBlock = {i, j - 1};
  }
}

int main() {
  std::string needle, haystack;
  std::cin >> needle >> haystack;
  findMatches(haystack, needle, [](int i) { std::cout << i << ' '; });
  std::cout << '\n';

  return 0;
}
