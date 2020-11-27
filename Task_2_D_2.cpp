//
// Created by egork on 10.11.2020.
//
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using Coordinate = long long;

struct Vector {
  Coordinate x;
  Coordinate y;

  Coordinate Cross(const Vector &other) const {
    return x * other.y - y * other.x;
  }

  long double Angle() const {
    long double atan2 = std::atan2(static_cast<long double>(y), static_cast<long double>(x));
    return atan2 >= 0.0 ? atan2 : (atan2 + 2.0l * M_PIl);
  }

  Vector operator+(const Vector &other) const {
    return {x + other.x, y + other.y};
  }

  Vector operator-(const Vector &other) const {
    return {x - other.x, y - other.y};
  }
};

std::vector<Vector> ReadPolygon() {
  size_t n;
  std::cin >> n;

  std::vector<Vector> polygon(n);

  for (size_t i = 0; i < n; ++i) {
    Coordinate x, y;
    std::cin >> x >> y;
    polygon[i] = {x, y};
  }

  return polygon;
}

size_t BottomLeftPoint(const std::vector<Vector> &polygon) {
  size_t best = 0;

  for (size_t i = 0; i < polygon.size(); ++i)
    if (polygon[i].y < polygon[best].y
        || (polygon[i].y == polygon[best].y && polygon[i].x < polygon[best].x))
      best = i;

  return best;
}

std::vector<Vector> MinkovskiSum(const std::vector<Vector> &polygon1,
                                 const std::vector<Vector> &polygon2) {
  std::vector<Vector> result;
  size_t i = BottomLeftPoint(polygon1);
  size_t j = BottomLeftPoint(polygon2);
  size_t count1 = 0;
  size_t count2 = 0;

  while (count1 < polygon1.size() && count2 < polygon2.size()) {
    result.push_back(polygon1[i] + polygon2[j]);
    size_t i_next = (i + 1) % polygon1.size();
    size_t j_next = (j + 1) % polygon2.size();

    long double angle_i = (polygon1[i_next] - polygon1[i]).Angle();
    long double angle_j = (polygon2[j_next] - polygon2[j]).Angle();

    if (angle_i < angle_j) {
      i = i_next;
      ++count1;
    } else if (angle_i > angle_j) {
      j = j_next;
      ++count2;
    } else {
      i = i_next;
      ++count1;
      j = j_next;
      ++count2;
    }
  }

  for (; count1 < polygon1.size(); i = (i + 1) % polygon1.size(), ++count1)
    result.push_back(polygon1[i] + polygon2[j]);

  for (; count2 < polygon2.size(); j = (j + 1) % polygon2.size(), ++count2)
    result.push_back(polygon1[i] + polygon2[j]);

  return result;
}

long double PolygonArea(const std::vector<Vector> &polygon) {
  Coordinate oriented_area = 0;
  Vector first = polygon.front();

  for (size_t i = 1; i < polygon.size() - 1; ++i)
    oriented_area += (polygon[i] - first).Cross(polygon[i + 1] - first);

  return oriented_area / 2.0l;
}

int main() {
  std::vector<Vector> polygon1 = ReadPolygon();
  std::vector<Vector> polygon2 = ReadPolygon();
  std::vector<Vector> minkovski_sum = MinkovskiSum(polygon1, polygon2);
  std::cout << std::setprecision(6) << std::setiosflags(std::ios_base::fixed)
            << (PolygonArea(minkovski_sum) - PolygonArea(polygon1) - PolygonArea(polygon2)) / 2.0l
            << '\n';
}
