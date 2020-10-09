//
// Created by egork on 24.09.2020.
//
#include <iostream>
#include <string>
#include <vector>
#include <numeric>

struct SubPalindrome {
  int center;
  int length;
};

enum PalindromeType {
  kOdd,
  kEven
};

std::vector<int> CountSubPalindromes(const std::string &s, PalindromeType type) {
  int n = s.size();
  std::vector<int> palidrome_length(n, 0);
  SubPalindrome right_most_block{-1, 0};

  auto left = [type](int center, int length) -> int {
    return center - length + (type == kEven);
  };
  auto right = [type](int center, int length) -> int {
    return center + length;
  };

  for (int i = 0; i < n; ++i) {
    int block_left = left(right_most_block.center, right_most_block.length);
    int block_right = right(right_most_block.center, right_most_block.length);
    int length = 0;

    if (i <= block_right) {
      int mirrored_i = 2 * right_most_block.center - i;

      if (block_left < left(mirrored_i, palidrome_length[mirrored_i])) {
        palidrome_length[i] = palidrome_length[mirrored_i];
        continue;
      }

      if (mirrored_i >= 0) {
        if (type == kOdd)
          length = std::min(palidrome_length[mirrored_i], block_right - i);
        else
          length = std::max(0, std::min(palidrome_length[mirrored_i], block_right - i - 1));
      }
    }

    while (0 <= left(i, length + 1) && right(i, length + 1) < s.size()
        && s[left(i, length + 1)] == s[right(i, length + 1)])
      ++length;

    palidrome_length[i] = length;
    right_most_block = {i, length};
  }

  return palidrome_length;
}

long long CountSubPalidromes(const std::string &s) {
  std::vector<int> odd_lengths = CountSubPalindromes(s, kOdd);
  std::vector<int> even_lengths = CountSubPalindromes(s, kEven);
  long long result = 0;

  for (int l : odd_lengths)
    result += l;

  for (int l : even_lengths)
    result += l;

  return result;
}

int main() {
  std::string s;
  std::cin >> s;
  std::cout << CountSubPalidromes(s) << '\n';
}
