#pragma once
#include <vector>

namespace game {

struct Block {
    int x{0};     // left cell
    int y{0};     // row
    bool alive{true};
};

struct Paddle {
    int x{0};     // center
    int y{0};
    int half{1};  // 3-wide => half=1
};

struct Ball {
    int x{0}, y{0};
    int vx{1}, vy{-1};
    int step_ms{10};
    float speed{6.0f};
};

struct World {
    int N{27};                // inner NxN
    std::vector<Block> blocks;
    int score{0};
    int round{1};
};

} // namespace game
