//
// Created by egork on 27.10.2020.
//
#include <iostream>

using Coordinate = int;

enum Orientation : int {
  CounterClockwise = -1,
  SameAngle = 0,
  Clockwise = 1
};

struct Vector {
  Coordinate x;
  Coordinate y;

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
};

Vector operator-(const Vector &left, const Vector &right) {
  return {left.x - right.x, left.y - right.y};
}

Vector operator+(const Vector &left, const Vector &right) {
  return {left.x + right.x, left.y + right.y};
}

struct Segment {
  Vector start;
  Vector end;

  Coordinate LowX() const {
    return std::min(start.x, end.x);
  }

  Coordinate HighX() const {
    return std::max(start.x, end.x);
  }

  Coordinate LowY() const {
    return std::min(start.y, end.y);
  }

  Coordinate HighY() const {
    return std::max(start.y, end.y);
  }

  Vector Dir() const {
    return end - start;
  }

  bool Intersects(const Segment &other) const {
    if (HighX() < other.LowX() || other.HighX() < LowX() || HighY() < other.LowY()
        || other.HighY() < LowY())
      return false;

    return Dir().Orient(other.start - start) * Dir().Orient(other.end - start) <= 0
        && other.Dir().Orient(start - other.start) * other.Dir().Orient(end - other.start) <= 0;
  }
};

std::istream &operator>>(std::istream &in, Segment &segment) {
  Coordinate sx, sy, ex, ey;
  in >> sx >> sy >> ex >> ey;
  segment = {{sx, sy}, {ex, ey}};
  return in;
}

int main() {
  Segment path;
  std::cin >> path;

  int n;
  std::cin >> n;

  int intersects = 0;

  for (int i = 0; i < n; ++i) {
    Segment river;
    std::cin >> river;
    intersects += path.Intersects(river);
  }

  std::cout << intersects << '\n';
}
