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
#include <random>
#include <chrono>

struct Vector {
  int64_t x;
  int64_t y;

  int64_t Dot(const Vector &other) const {
    return x * other.x + y * other.y;
  }

  int64_t Cross(const Vector &other) const {
    return x * other.y - y * other.x;
  }

  int64_t Orientation(const Vector &other) const {
    int64_t prod = Cross(other);

    if (prod > 0)
      return 1;
    else if (prod < 0)
      return -1;
    else
      return 0;
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

bool IsInside(int64_t x, int64_t start, int64_t end) {
  return start <= x && x <= end;
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

  // if < 0 then lower
  // if = 0 then equal
  // if > 0 then greater
  int64_t CompareYAt(int64_t x, int64_t target_y) const {
    if (DiffX() <= 0)
      throw std::runtime_error("diff <= 0");

    // y = start.y + (x - start.x) / DiffX() * DiffY()
    // y < target_y   <=>   DiffX() * y < target_y * DiffX()
    //   =                              =
    //   >                              >
    return (start.y * DiffX() + (x - start.x) * DiffY()) - target_y * DiffX();
  }

  // the same as above
  int64_t CompareY(const Segment &other) const {
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

  int64_t DiffX() const {
    return end.x - start.x;
  }

  int64_t DiffY() const {
    return end.y - start.y;
  }

  int64_t LowY() const {
    return std::min(start.y, end.y);
  }

  int64_t HighY() const {
    return std::max(start.y, end.y);
  }

  Vector dir() const {
    return end - start;
  }

  bool Intersects(Segment other) const {
    if (end.x < other.start.x || other.end.x < start.x || HighY() < other.LowY()
        || other.HighY() < LowY())
      return false;

    return dir().Orientation(other.start - start) * dir().Orientation(other.end - start) <= 0
        && other.dir().Orientation(start - other.start) * other.dir().Orientation(end - other.start)
            <= 0;
  }

  bool operator<(const Segment &other) const {
    int64_t y_comparison = CompareY(other);
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

bool SortCorrect(const std::vector<Event> &events, const std::vector<Segment> &segments) {
  std::set<Segment> open_segments;

  for (Event e : events)
    if (e.start) {
      if (!open_segments.insert(segments[e.id]).second)
        return false;
    } else {
      auto it = open_segments.find(segments[e.id]);

      if (it == open_segments.end())
        return false;

      open_segments.erase(it);
    }

  return open_segments.empty();
}

std::optional<std::pair<int, int>> FindIntersecting(const std::vector<Segment> &segments) {
  std::vector<Event> events;
  std::set<Segment> open_segments;

  for (Segment s : segments) {
    events.push_back({s.start, s.id, true});
    events.push_back({s.end, s.id, false});
  }

  std::sort(events.begin(), events.end());

//  if (!SortCorrect(events, segments)) {
//    throw std::runtime_error("Incorrect sort");
//    while (true) {
//      std::cout << "asdkaj\n";
//    }
//  }

  for (Event e : events)
    if (e.start) {
      auto it = open_segments.insert(segments[e.id]).first;
      auto intersection = IntersectAdjacent(it, open_segments);

      if (intersection.has_value())
        return intersection.value();
    } else {
      auto it = open_segments.find(segments[e.id]);

      if (it == open_segments.end()) {
//        while (true) {
//          std::cout << "asdkaj\n";
//        }
        throw std::runtime_error("tried to delete a non-existent segment");
      }

      auto intersection = IntersectAdjacent(it, open_segments);

      if (intersection.has_value())
        return intersection.value();
      else
        open_segments.erase(it);
    }

  return std::nullopt;
}

std::optional<std::pair<int, int>> FindIntersectingNaive(const std::vector<Segment> &segments) {
  for (int i = 0; i < segments.size(); ++i)
    for (int j = i + 1; j < segments.size(); ++j)
      if (segments[i].Intersects(segments[j]))
        return std::make_pair(i, j);

  return std::nullopt;
}

void RandomTest() {
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<int64_t> distribution(-10, 10);

  auto random_segment = [&distribution, &gen](int id) -> Segment {
    return {{distribution(gen), distribution(gen)}, {distribution(gen), distribution(gen)}, id};
  };

  for (int test_id = 0; test_id < 1000000; ++test_id) {
    std::cout << "Test number " << test_id << ":\n";
    std::vector<Segment> segments;

    for (int i = 0; i < 3; ++i) {
      Segment segment = random_segment(i);
      std::cout << segment.start.x << ' ' << segment.start.y << ' ' << segment.end.x << ' '
                << segment.end.y << '\n';
      segments.push_back(segment);
    }

    auto result = FindIntersecting(segments);
    auto real_result = FindIntersectingNaive(segments);

    if (result.has_value() != real_result.has_value() || (result.has_value()
        && !(segments[result.value().first].Intersects(segments[result.value().second]))))
      throw std::runtime_error("WRONG");
  }
}

int main() {
//  RandomTest();
//  return 0;

  int n;
  std::cin >> n;
  std::vector<Segment> segments;

  for (int i = 0; i < n; ++i) {
    int64_t x1, y1, x2, y2;
    std::cin >> x1 >> y1 >> x2 >> y2;

    if (x1 == x2 && y1 == y2)
      throw std::runtime_error("zero-length segment???");

    segments.push_back({{x1, y1}, {x2, y2}, i});

    if (segments.back() < segments.back())
      throw std::runtime_error("a < a???");
  }

  auto intersection = FindIntersecting(segments);
//  auto real_intersection = FindIntersectingNaive(segments);

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
