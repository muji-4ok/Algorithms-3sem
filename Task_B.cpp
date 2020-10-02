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

std::vector<int> countOddPalindromes(const std::string &s) {
  int n = s.size();
  std::vector<int> palidromeLength(n, 0);
  SubPalindrome rightMostBlock{0, 0};

  for (int i = 0; i < n; ++i) {
    int blockLeft = rightMostBlock.center - rightMostBlock.length;
    int blockRight = rightMostBlock.center + rightMostBlock.length;
    int length = 0;

    if (i <= blockRight) {
      int mirroredI = 2 * rightMostBlock.center - i;

      if (blockLeft < mirroredI - palidromeLength[mirroredI]) {
        palidromeLength[i] = palidromeLength[mirroredI];
        continue;
      }

      length = std::min(palidromeLength[mirroredI], blockRight - i);
    }

    while (0 <= i - length - 1 && i + length + 1 < s.size()
        && s[i - length - 1] == s[i + length + 1])
      ++length;

    palidromeLength[i] = length;
    rightMostBlock = {i, length};
  }

  return palidromeLength;
}

std::vector<int> countEvenPalindromes(const std::string &s) {
  int n = s.size();
  std::vector<int> palidromeLength(n, 0);
  // Center is lower
  SubPalindrome rightMostBlock{-1, 0};

  for (int i = 0; i < n - 1; ++i) {
    int blockLeft = rightMostBlock.center - rightMostBlock.length + 1;
    int blockRight = rightMostBlock.center + rightMostBlock.length;
    int length = 0;

    if (i <= blockRight) {
      int mirroredI = 2 * rightMostBlock.center - i;

      if (blockLeft < mirroredI - palidromeLength[mirroredI] + 1) {
        palidromeLength[i] = palidromeLength[mirroredI];
        continue;
      }

      if (mirroredI >= 0)
        length = std::max(0, std::min(palidromeLength[mirroredI], blockRight - i - 1));
    }

    while (0 <= i - length && i + length + 1 < s.size()
        && s[i - length] == s[i + length + 1])
      ++length;

    palidromeLength[i] = length;
    rightMostBlock = {i, length};
  }

  return palidromeLength;
}

long long countSubPalidromes(const std::string &s) {
  std::vector<int> oddLengths = countOddPalindromes(s);
  std::vector<int> evenLengths = countEvenPalindromes(s);
  long long result = 0;

  for (int l : oddLengths)
    result += l;

  for (int l : evenLengths)
    result += l;

  return result;
}

int main() {
  std::string s;
  std::cin >> s;
  std::cout << countSubPalidromes(s) << '\n';
}
