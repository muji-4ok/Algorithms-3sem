//
// Created by egork on 28.10.2020.
//
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <set>

struct Vector {
  int x;
  int y;
  int z;
  int id = -1;

  Vector Cross(const Vector &other) const {
    return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
  }

  int Dot(const Vector &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  int SqrDist() const {
    return Dot(*this);
  }

  // if this and other are collinear, then this tells whether or not they have the same direction
  // if any vector is zero, direction is undefined
  bool SameDir(const Vector &other) const {
    return Dot(other) > 0;
  }

  bool operator==(const Vector &other) const {
    return x == other.x && y == other.y && z == other.z;
  }
  bool operator!=(const Vector &other) const {
    return !(*this == other);
  }
};

Vector operator-(const Vector &left, const Vector &right) {
  return {left.x - right.x, left.y - right.y, left.z - right.z};
}

Vector operator+(const Vector &left, const Vector &right) {
  return {left.x + right.x, left.y + right.y, left.z + right.z};
}

struct Segment {
  Vector start;
  Vector end;

  Vector Dir() const { return end - start; }

  bool operator==(const Segment &other) const {
    return start == other.start && end == other.end;
  }

  bool operator!=(const Segment &other) const {
    return !(*this == other);
  }
};

namespace std {

template<>
struct hash<Vector> {
  size_t operator()(const Vector &vector) const {
    // just some random prime
    return static_cast<size_t>(vector.x) * 199933 + static_cast<size_t>(vector.y);
  }
};

template<>
struct hash<Segment> {
  size_t operator()(const Segment &segment) const {
    // same
    return hash<Vector>()(segment.start) * 37 + hash<Vector>()(segment.end);
  }
};
}

struct Face {
  /*
   * p3
   * |  \
   * |    \
   * |      \
   * p1 ---- p2
   *
   * p1 -> p2 -> p3
   *
   */
  Face(Vector p1, Vector p2, Vector p3) {
    Vector lowest = std::min({p1, p2, p3}, [](const Vector &left, const Vector &right) -> bool {
      return left.id < right.id;
    });

    this->p1 = lowest;

    if (lowest == p1) {
      this->p2 = p2;
      this->p3 = p3;
    } else if (lowest == p2) {
      this->p2 = p3;
      this->p3 = p1;
    } else {
      this->p2 = p1;
      this->p3 = p2;
    }
  }

  Vector p1;
  Vector p2;
  Vector p3;

  bool operator<(const Face &other) const {
    return p1.id < other.p1.id || (p1.id == other.p1.id
        && (p2.id < other.p2.id || (p2.id == other.p2.id && p3.id < other.p3.id)));
  }
};

std::ostream &operator<<(std::ostream &out, const Vector &vector) {
  out << vector.id;
  return out;
}

std::ostream &operator<<(std::ostream &out, const Face &face) {
  out << "3 " << face.p1 << " " << face.p2 << " " << face.p3;
  return out;
}

Vector FoldPoint(Segment edge, const std::vector<Vector> &points) {
  Vector best = edge.start;

  for (Vector p : points) {
    if (p == edge.start || p == edge.end)
      continue;

    if (best == edge.start) {
      best = p;
      continue;
    }

    if (best != p && edge.Dir().SameDir((best - edge.start).Cross(p - edge.start)))
      best = p;
  }

  return best;
}

Segment FindLowestEdge(const std::vector<Vector> &points) {
  Vector lowest = points.front();
  for (Vector p : points)
    if (p.z < lowest.z)
      lowest = p;

  for (Vector p : points)
    if (p != lowest && p.z == lowest.z)
      return {lowest, p};

  Vector towards_x{lowest.x + 1, lowest.y, lowest.z};

  return {lowest, FoldPoint({lowest, towards_x}, points)};
}

std::set<Face> BuildConvexHull(const std::vector<Vector> &points) {
  std::vector<Segment> edges_left{FindLowestEdge(points)};
  std::unordered_set<Segment> used_edges;
  std::set<Face> faces;

  while (!edges_left.empty()) {
    Segment edge = edges_left.back();
    edges_left.pop_back();

    if (used_edges.find(edge) != used_edges.end())
      continue;

    used_edges.insert(edge);

    Vector folding_point = FoldPoint(edge, points);
    Face face(edge.start, edge.end, folding_point);

    if (faces.insert(face).second) {
      edges_left.push_back({edge.start, folding_point});
      edges_left.push_back({folding_point, edge.end});
    }
  }

  return faces;
}

int main() {
  int m;
  std::cin >> m;

  for (int test_id = 0; test_id < m; ++test_id) {
    int n;
    std::cin >> n;

    std::vector<Vector> points;

    for (int i = 0; i < n; ++i) {
      int x, y, z;
      std::cin >> x >> y >> z;
      points.push_back({x, y, z, i});
    }

    std::set<Face> hull = BuildConvexHull(points);

    std::cout << hull.size() << '\n';

    for (Face f : hull)
      std::cout << f << '\n';
  }
}
