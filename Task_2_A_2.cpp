//
// Created by egork on 27.10.2020.
//
#include <iostream>

struct Vector {
  int x;
  int y;

  int dot(const Vector &other) const {
    return x * other.x + y * other.y;
  }

  int cross(const Vector &other) const {
    return x * other.y - y * other.x;
  }

  int orientation(const Vector &other) const {
    int prod = cross(other);

    if (prod > 0)
      return 1;
    else if (prod < 0)
      return -1;
    else
      return 0;
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

  int LowX() const {
    return std::min(start.x, end.x);
  }

  int HighX() const {
    return std::max(start.x, end.x);
  }

  int LowY() const {
    return std::min(start.y, end.y);
  }

  int HighY() const {
    return std::max(start.y, end.y);
  }

  Vector dir() const {
    return end - start;
  }

  bool Intersects(Segment other) const {
    if (!((HighX() >= other.LowX() || other.HighX() >= LowX())
        && (HighY() >= other.LowY() || other.HighY() >= LowY())))
      return false;

    return dir().orientation(other.start - start) * dir().orientation(other.end - start) <= 0
        && other.dir().orientation(start - other.start) * other.dir().orientation(end - other.start)
            <= 0;
  }
};

std::istream &operator>>(std::istream &in, Segment &segment) {
  int sx, sy, ex, ey;
  in >> sx >> sy >> ex >> ey;
  segment = {{sx, sy}, {ex, ey}};
  return in;
}

int main() {
  Segment path{};
  std::cin >> path;

  int n;
  std::cin >> n;

  int intersects = 0;

  for (int i = 0; i < n; ++i) {
    Segment river{};
    std::cin >> river;
    intersects += path.Intersects(river);
  }

  std::cout << intersects << '\n';
}
