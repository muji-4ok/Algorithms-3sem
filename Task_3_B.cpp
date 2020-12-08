#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <array>

struct Position {
  int x;
  int y;

  bool operator==(const Position &other) const { return x == other.x && y == other.y; }
  bool operator!=(const Position &other) const { return !(*this == other); }
};

std::ostream &operator<<(std::ostream &out, const Position &pos) {
  out << "(" << pos.x << ", " << pos.y << ")";
  return out;
}

// From the perspective of the runner
enum GameResult {
  Wins,
  Loses,
  Draws,
  Empty
};

struct State {
  Position runner;
  Position terminator;
  bool runners_turn;

  bool operator==(const State &other) const {
    return runner == other.runner && terminator == other.terminator
        && runners_turn == other.runners_turn;
  }

  bool operator!=(const State &other) const {
    return !(*this == other);
  }
};

std::ostream &operator<<(std::ostream &out, const State &state) {
  out << state.runner << " " << state.terminator << " " << (state.runners_turn ? "R" : "T");
  return out;
}

struct VisitInfo {
  int visited;
  int max;
  GameResult result;
};

namespace std {

template<>
struct hash<Position> {
  size_t operator()(const Position &position) const {
    // just some random prime
    return static_cast<size_t>(position.x) * 199933 + static_cast<size_t>(position.y);
  }
};

template<>
struct hash<State> {
  size_t operator()(const State &state) const {
    // same
    return hash<Position>()(state.runner) * 37 + hash<Position>()(state.terminator)
        + state.runners_turn;
  }
};
}

bool TerminatorCanShoot(State state, const std::unordered_set<Position> &walls) {
  // doesn't matter whose turn it is

  if (state.runner.y == state.terminator.y) {
    int min_x = std::min(state.runner.x, state.terminator.x);
    int max_x = std::max(state.runner.x, state.terminator.x);

    for (int x = min_x; x < max_x; ++x)
      if (walls.find({x, state.runner.y}) != walls.end())
        return false;

    return true;
  } else if (state.runner.x == state.terminator.x) {
    int min_y = std::min(state.runner.y, state.terminator.y);
    int max_y = std::max(state.runner.y, state.terminator.y);

    for (int y = min_y; y < max_y; ++y)
      if (walls.find({state.runner.x, y}) != walls.end())
        return false;

    return true;
  } else if (std::abs(state.runner.x - state.terminator.x)
      == std::abs(state.runner.y - state.terminator.y)) {
    int diff_x = state.runner.x < state.terminator.x ? 1 : -1;
    int diff_y = state.runner.y < state.terminator.y ? 1 : -1;

    for (int x = state.runner.x, y = state.runner.y; x != state.terminator.x;
         x += diff_x, y += diff_y)
      if (walls.find({x, y}) != walls.end())
        return false;

    return true;
  } else {
    return false;
  }
}

bool RunnerCanEscape(State state) {
  return state.runners_turn && state.runner.y == 7;
}

bool InBounds(Position move) {
  return 0 <= move.x && move.x <= 7 && 0 <= move.y && move.y <= 7;
}

std::array<Position, 8> CalculateMoves(Position pos) {
  return std::array<Position, 8>{Position{pos.x + 1, pos.y},
                                 Position{pos.x - 1, pos.y},
                                 Position{pos.x, pos.y + 1},
                                 Position{pos.x, pos.y - 1},
                                 Position{pos.x + 1, pos.y + 1},
                                 Position{pos.x + 1, pos.y - 1},
                                 Position{pos.x - 1, pos.y + 1},
                                 Position{pos.x - 1, pos.y - 1}};
}

bool StateValid(State state, const std::unordered_set<Position> &walls) {
  return state.runner != state.terminator && InBounds(state.runner) && InBounds(state.terminator)
      && walls.find(state.runner) == walls.end() && walls.find(state.terminator) == walls.end();
}

std::vector<State> CalculateTransitions(State state,
                                        const std::unordered_set<Position> &walls,
                                        bool inverse = false) {
  std::vector<State> states;
  bool runner_moves = (!inverse && state.runners_turn) || (inverse && !state.runners_turn);
  Position moving_player = runner_moves ? state.runner : state.terminator;

  for (Position move : CalculateMoves(moving_player)) {
    State next_state = runner_moves ?
                       State{move, state.terminator, !state.runners_turn} :
                       State{state.runner, move, !state.runners_turn};

    if (StateValid(next_state, walls))
      states.push_back(next_state);
  }

  return states;
}

std::unordered_map<State, VisitInfo> InitializeStates(const std::unordered_set<Position> &walls) {
  std::unordered_map<State, VisitInfo> states_info;

  for (int runners_turn = 0; runners_turn < 2; ++runners_turn)
    for (int term_y = 0; term_y < 8; ++term_y)
      for (int term_x = 0; term_x < 8; ++term_x)
        for (int runn_y = 0; runn_y < 8; ++runn_y)
          for (int runn_x = 0; runn_x < 8; ++runn_x) {
            State state{{runn_x, runn_y}, {term_x, term_y}, static_cast<bool>(runners_turn)};

            if (StateValid(state, walls)) {
              bool terminator_wins = TerminatorCanShoot(state, walls);
              bool runner_wins = !terminator_wins && RunnerCanEscape(state);
              GameResult result = Empty;

              if (terminator_wins)
                result = Loses;
              else if (runner_wins)
                result = Wins;

              states_info[state] = {0, (int) CalculateTransitions(state, walls).size(), result};
            }
          }

  return states_info;
}

void PrintFullState(State state, const std::unordered_set<Position> &walls) {
  bool has_wall[8][8] = {};

  for (const auto &it : walls)
    has_wall[it.y][it.x] = true;

  for (int y = 5; y < 8; ++y) {
    for (int x = 0; x < 4; ++x) {
      Position pos{x, y};

      if (pos == state.runner)
        std::cout << '2';
      else if (pos == state.terminator)
        std::cout << '3';
      else if (has_wall[y][x])
        std::cout << '1';
      else
        std::cout << '0';
    }

    std::cout << '\n';
  }
}

GameResult ResolveGame(State start, const std::unordered_set<Position> &walls) {
  std::unordered_map<State, VisitInfo> states_info = InitializeStates(walls);
  std::vector<State> stack;

  for (const auto &it : states_info)
    if (it.second.result == Wins || it.second.result == Loses) {
      stack.push_back(it.first);

//      if (it.first == start)
//      std::cout << it.first << " --- " << (it.second.result == Wins ? "W" : "L") << '\n';

//      std::cout << "--------------\n";
//      std::cout << "Result: " << (it.second.result == Wins ? "W" : "L") << '\n';
//      PrintFullState(it.first, walls);
//      std::cout << '\n';
    }

  while (!stack.empty()) {
    State top = stack.back();
    stack.pop_back();

    for (State prev_state : CalculateTransitions(top, walls, true)) {
      if (states_info[prev_state].result != Empty)
        continue;

      GameResult cur_player_win = prev_state.runners_turn ? Wins : Loses;
      GameResult cur_player_loss = prev_state.runners_turn ? Loses : Wins;

      if (states_info[top].result == cur_player_loss) {
        ++states_info[prev_state].visited;

        if (states_info[prev_state].visited == states_info[prev_state].max) {
          states_info[prev_state].result = cur_player_loss;
          stack.push_back(prev_state);
        }
      } else {
        states_info[prev_state].result = cur_player_win;
        stack.push_back(prev_state);
      }
    }
  }

  for (auto &it : states_info)
    if (it.second.result == Empty)
      it.second.result = Draws;

  return states_info[start].result;
}

int main() {
  std::unordered_set<Position> walls;
  State state;
  state.runners_turn = true;

  for (int y = 0; y < 8; ++y) {
    std::string line;
    std::cin >> line;

    for (int x = 0; x < 8; ++x) {
      if (line[x] == '1')
        walls.insert({x, y});
      else if (line[x] == '2')
        state.runner = {x, y};
      else if (line[x] == '3')
        state.terminator = {x, y};
    }
  }

  GameResult result = ResolveGame(state, walls);
  std::cout << (result == Wins ? 1 : -1) << '\n';
}