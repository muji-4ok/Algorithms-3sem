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

std::vector<int> CountOddPalindromes(const std::string &s) {
  int n = s.size();
  std::vector<int> palidrome_length(n, 0);
  SubPalindrome right_most_block{0, 0};

  for (int i = 0; i < n; ++i) {
    int block_left = right_most_block.center - right_most_block.length;
    int block_right = right_most_block.center + right_most_block.length;
    int length = 0;

    if (i <= block_right) {
      int mirrored_i = 2 * right_most_block.center - i;

      if (block_left < mirrored_i - palidrome_length[mirrored_i]) {
        palidrome_length[i] = palidrome_length[mirrored_i];
        continue;
      }

      length = std::min(palidrome_length[mirrored_i], block_right - i);
    }

    while (0 <= i - length - 1 && i + length + 1 < s.size()
        && s[i - length - 1] == s[i + length + 1])
      ++length;

    palidrome_length[i] = length;
    right_most_block = {i, length};
  }

  return palidrome_length;
}

std::vector<int> CountEvenPalindromes(const std::string &s) {
  int n = s.size();
  std::vector<int> palidrome_length(n, 0);
  // Center is lower
  SubPalindrome right_most_block{-1, 0};

  for (int i = 0; i < n - 1; ++i) {
    int blockLeft = right_most_block.center - right_most_block.length + 1;
    int blockRight = right_most_block.center + right_most_block.length;
    int length = 0;

    if (i <= blockRight) {
      int mirroredI = 2 * right_most_block.center - i;

      if (blockLeft < mirroredI - palidrome_length[mirroredI] + 1) {
        palidrome_length[i] = palidrome_length[mirroredI];
        continue;
      }

      if (mirroredI >= 0)
        length = std::max(0, std::min(palidrome_length[mirroredI], blockRight - i - 1));
    }

    while (0 <= i - length && i + length + 1 < s.size()
        && s[i - length] == s[i + length + 1])
      ++length;

    palidrome_length[i] = length;
    right_most_block = {i, length};
  }

  return palidrome_length;
}

long long CountSubPalidromes(const std::string &s) {
  std::vector<int> odd_lengths = CountOddPalindromes(s);
  std::vector<int> even_lengths = CountEvenPalindromes(s);
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
