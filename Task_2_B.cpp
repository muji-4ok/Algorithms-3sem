//
// Created by egork on 27.10.2020.
//
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>

struct Vector {
  long long x;
  long long y;

  long long SqrDist() const { return x * x + y * y; }

  double Dist() const { return std::sqrt((double) (x * x + y * y)); }

  long long Dot(const Vector &other) const {
    return x * other.x + y * other.y;
  }

  long long Cross(const Vector &other) const {
    return x * other.y - y * other.x;
  }

  long long Orientation(const Vector &other) const {
    long long prod = Cross(other);

    if (prod > 0)
      return 1;
    else if (prod < 0)
      return -1;
    else
      return 0;
  }

  bool operator==(const Vector &other) const {
    return x == other.x && y == other.y;
  }
};

Vector operator-(const Vector &left, const Vector &right) {
  return {left.x - right.x, left.y - right.y};
}

Vector operator+(const Vector &left, const Vector &right) {
  return {left.x + right.x, left.y + right.y};
}

std::vector<Vector> BuildConvexHull(std::vector<Vector> points) {
  Vector lowest = *std::min_element(
      points.begin(), points.end(),
      [](const Vector &left, const Vector &right) -> bool {
        return left.y < right.y;
      }
  );

  std::sort(
      points.begin(), points.end(),
      [&lowest](const Vector &left, const Vector &right) -> bool {
        Vector dir_left = left - lowest;
        Vector dir_right = right - lowest;
        long long orientation = dir_left.Orientation(dir_right);

        return orientation > 0 || (orientation == 0 && dir_left.SqrDist() < dir_right.SqrDist());
      }
  );

  std::vector<Vector> hull{lowest};

  for (const Vector &p : points) {
    if (p == hull.back())
      continue;

    while (hull.size() >= 2
        && (hull[hull.size() - 1] - hull[hull.size() - 2]).Orientation(p - hull[hull.size() - 1])
            < 0)
      hull.pop_back();

    hull.push_back(p);
  }

  return hull;
}

double PolygonLength(const std::vector<Vector> &points) {
  double length = 0.0;

  for (int i = 1; i < points.size(); ++i)
    length += (points[i] - points[i - 1]).Dist();

  length += (points.back() - points.front()).Dist();

  return length;
}

int main() {
  int n;
  std::cin >> n;
  std::vector<Vector> points;

  for (int i = 0; i < n; ++i) {
    long long x, y;
    std::cin >> x >> y;
    points.push_back({x, y});
  }

  std::cout << std::setprecision(10) << PolygonLength(BuildConvexHull(points)) << '\n';
}
