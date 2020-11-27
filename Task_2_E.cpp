//
// Created by egork on 10.11.2020.
//
#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>
#include <set>
#include <utility>
#include <tuple>
#include <cassert>

using Coordinate = long long;

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

  bool operator<(const Vector &other) const {
    return std::tie(x, y) < std::tie(other.x, other.y);
  }

  bool operator==(const Vector &other) const {
    return x == other.x && y == other.y;
  }

  bool operator!=(const Vector &other) const {
    return !(*this == other);
  }
};

Vector operator-(const Vector &left, const Vector &right) {
  return {left.x - right.x, left.y - right.y};
}

Vector operator+(const Vector &left, const Vector &right) {
  return {left.x + right.x, left.y + right.y};
}

struct Segment {
  Segment(Vector start, Vector end, int id) : id(id) {
    if (start.x > end.x)
      std::swap(start, end);

    this->start = start;
    this->end = end;
  }

  // start.x <= end.x
  Vector start;
  Vector end;
  int id;

  // Compare value of this segment's Y coordinate at X position 'x' to 'target_y'
  //
  // Return:
  // if (this.y < target_y) then < 0
  // if (this.y = target_y) then = 0
  // if (this.y > target_y) then > 0
  Coordinate CompareYAt(Coordinate x, Coordinate target_y) const {
    // y = start.y + (x - start.x) / DiffX() * DiffY()
    // y < target_y   <=>   DiffX() * y < target_y * DiffX()
    //   =                              =
    //   >                              >
    return (start.y * DiffX() + (x - start.x) * DiffY()) - target_y * DiffX();
  }

  // Compare two segments by their Y coordinate at X position max{this->start.x, other.start.x}
  // We want to keep everything in ints, and we know that at least one of the segments has an
  // int Y coordinate, but the other might not, which is why we have CompareYAt, which
  // uses a slightly different equation for comparison, so as to keep everything in ints
  //
  // Return:
  // if (*this < other) then < 0
  // if (*this = other) then = 0
  // if (*this > other) then > 0
  Coordinate CompareY(const Segment &other) const {
    if (DiffX() == 0 && other.DiffX() == 0)
      return LowY() - other.LowY();
    else if (DiffX() == 0)
      return -other.CompareYAt(start.x, LowY());
    else if (other.DiffX() == 0)
      return CompareYAt(other.start.x, other.LowY());
    else
      return start.x > other.start.x ? -other.CompareYAt(start.x, start.y) :
             CompareYAt(other.start.x, other.start.y);
  }

  Coordinate DiffX() const {
    return end.x - start.x;
  }

  Coordinate DiffY() const {
    return end.y - start.y;
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

  bool Intersects(Segment other) const {
    if (end.x < other.start.x || other.end.x < start.x || HighY() < other.LowY()
        || other.HighY() < LowY())
      return false;

    return Dir().Orient(other.start - start) * Dir().Orient(other.end - start) <= 0
        && other.Dir().Orient(start - other.start) * other.Dir().Orient(end - other.start) <= 0;
  }

  bool operator<(const Segment &other) const {
    Coordinate y_comparison = CompareY(other);
    return y_comparison < 0 || (y_comparison == 0
        && std::tie(start, end, id) < std::tie(other.start, other.end, other.id));
  }

  bool operator==(const Segment &other) const {
    return start == other.start && end == other.end && id == other.id;
  }

  bool operator!=(const Segment &other) const {
    return !(*this == other);
  }
};

struct Event {
  Vector point;
  int id;
  bool start;

  bool operator<(const Event &other) const {
    return point.x < other.point.x || (point.x == other.point.x
        && (start && !other.start || (start == other.start && id < other.id)));
  }
};

std::optional<std::pair<int, int>> IntersectAdjacent(const std::set<Segment>::iterator &it,
                                                     const std::set<Segment> &open_segments) {
  if (it != open_segments.begin()) {
    auto prev = std::prev(it);

    if (it->Intersects(*prev))
      return std::make_pair(it->id, prev->id);
  }

  if (it != --open_segments.end()) {
    auto next = std::next(it);

    if (it->Intersects(*next))
      return std::make_pair(it->id, next->id);
  }

  return std::nullopt;
}

std::optional<std::pair<int, int>> FindIntersecting(const std::vector<Segment> &segments) {
  std::vector<Event> events;
  std::set<Segment> open_segments;

  for (Segment s : segments) {
    events.push_back({s.start, s.id, true});
    events.push_back({s.end, s.id, false});
  }

  std::sort(events.begin(), events.end());

  for (Event e : events)
    if (e.start) {
      auto it = open_segments.insert(segments[e.id]).first;
      auto intersection = IntersectAdjacent(it, open_segments);

      if (intersection.has_value())
        return intersection.value();
    } else {
      auto it = open_segments.find(segments[e.id]);
      assert(it != open_segments.end());
      auto intersection = IntersectAdjacent(it, open_segments);

      if (intersection.has_value())
        return intersection.value();
      else
        open_segments.erase(it);
    }

  return std::nullopt;
}

int main() {
  int n;
  std::cin >> n;
  std::vector<Segment> segments;

  for (int i = 0; i < n; ++i) {
    Coordinate x1, y1, x2, y2;
    std::cin >> x1 >> y1 >> x2 >> y2;
    segments.push_back({{x1, y1}, {x2, y2}, i});
  }

  auto intersection = FindIntersecting(segments);

  if (intersection.has_value()) {
    std::pair<int, int> val = intersection.value();

    if (val.first > val.second)
      std::swap(val.first, val.second);

    std::cout << "YES\n";
    std::cout << (val.first + 1) << ' ' << (val.second + 1) << '\n';
  } else {
    std::cout << "NO\n";
  }
}
