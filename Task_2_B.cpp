//
// Created by egork on 27.10.2020.
//
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>

using Coordinate = long long;

enum Orientation : int {
  CounterClockwise = -1,
  SameAngle = 0,
  Clockwise = 1
};

struct Vector {
  Coordinate x;
  Coordinate y;

  Coordinate SqrDist() const { return x * x + y * y; }

  double Dist() const { return std::sqrt(x * x + y * y); }

  Coordinate Dot(const Vector &other) const {
    return x * other.x + y * other.y;
  }

  Coordinate Cross(const Vector &other) const {
    return x * other.y - y * other.x;
  }

  Orientation Orient(const Vector &other) const {
    Coordinate prod = Cross(other);

    if (prod > 0)
      return Clockwise;
    else if (prod < 0)
      return CounterClockwise;
    else
      return SameAngle;
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
        Coordinate orientation = dir_left.Orient(dir_right);

        return orientation > 0 || (orientation == 0 && dir_left.SqrDist() < dir_right.SqrDist());
      }
  );

  std::vector<Vector> hull{lowest};

  for (const Vector &p : points) {
    if (p == hull.back())
      continue;

    while (hull.size() >= 2 && (hull.back() - hull[hull.size() - 2]).Orient(p - hull.back()) < 0)
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
    Coordinate x, y;
    std::cin >> x >> y;
    points.push_back({x, y});
  }

  std::cout << std::setprecision(10) << PolygonLength(BuildConvexHull(points)) << '\n';
}
