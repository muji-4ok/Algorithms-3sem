#include <iostream>
#include <vector>
#include <unordered_set>

std::vector<int> FindWinningStarts(int n) {
  std::vector<int> winning_starts;
  // from 0 to n
  std::vector<int> grandi_at(n + 1, 0);

  for (int size = 2; size <= n; ++size) {
    std::unordered_set<int> grandi_values;

    for (int pos = 0; pos < size; ++pos) {
      int size1 = pos;
      int size2 = size - 1 - pos;
      int grandi = grandi_at[size1] ^ grandi_at[size2];
      grandi_values.insert(grandi);

      if (size == n && grandi == 0)
        winning_starts.push_back(pos);
    }

    int new_value = 0;

    while (grandi_values.find(new_value) != grandi_values.end())
      ++new_value;

    grandi_at[size] = new_value;
  }

  return winning_starts;
}

int main() {
  int n;
  std::cin >> n;

  std::vector<int> winning_starts = FindWinningStarts(n);

  if (winning_starts.empty()) {
    std::cout << "Mueller\n";
  } else {
    std::cout << "Schtirlitz\n";

    for (int pos : winning_starts)
      std::cout << (pos + 1) << '\n';
  }
}