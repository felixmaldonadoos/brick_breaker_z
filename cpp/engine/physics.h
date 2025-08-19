#pragma once
#include <algorithm>
#include "game/entities.h"

namespace phys {

struct StepResult {
    bool hitPaddle{false};
    bool hitWall{false};
    int  destroyedIndex{-1};   // -1 means none
};

// Returns index of destroyed block, or -1 if none.
// Destroys when the ball's NEXT cell enters the 2-cell brick from ANY direction.
// Bounces on the axis of approach (vertical if coming from above/below, horizontal if from side).
inline int collideBlocksAndBounce(game::World& w, game::Ball& b) {
    const int nx = b.x + b.vx;
    const int ny = b.y + b.vy;

    for (int i = 0; i < (int)w.blocks.size(); ++i) {
        auto& bl = w.blocks[i];
        if (!bl.alive) continue;

        // If next step ENTERS the brick area (two cells wide at row bl.y)
        if (ny == bl.y && (nx == bl.x || nx == bl.x + 1)) {
            bl.alive = false;

            // Choose axis to reflect: if currently on same row, it was a side hit -> flip vx,
            // otherwise a top/bottom hit -> flip vy.
            if (b.y == bl.y) b.vx = -b.vx;
            else             b.vy = -b.vy;

            return i;
        }

        // (Optional) Edge grazes where we stop one cell before entering:
        if (b.vx > 0 && nx == bl.x - 1 && b.y == bl.y) {
            bl.alive = false; b.vx = -b.vx; return i;
        }
        if (b.vx < 0 && nx == bl.x + 2 && b.y == bl.y) {
            bl.alive = false; b.vx = -b.vx; return i;
        }
    }
    return -1;
}

inline StepResult stepBall(game::World& w, game::Ball& b, const game::Paddle& p) {
    StepResult res{};

    // Brick collisiFons FIRST
    if (int idx = collideBlocksAndBounce(w, b); idx >= 0) {
        res.destroyedIndex = idx;
    }

    int nx = b.x + b.vx;
    int ny = b.y + b.vy;

    // Walls (top/left/right)
    if (nx < 0 || nx >= w.N) { b.vx = -b.vx; nx = b.x + b.vx; res.hitWall = true; }
    if (ny < 0)              { b.vy = -b.vy; ny = b.y + b.vy; res.hitWall = true; }

    // Paddle (width = 2*half+1)
    if (ny == p.y && nx >= p.x - p.half && nx <= p.x + p.half) {
        b.vy = -b.vy;
        if (nx < p.x)      b.vx = -1;
        else if (nx > p.x) b.vx =  1;
        ny = b.y + b.vy;
        res.hitPaddle = true;
    }

    b.x = nx; b.y = ny;
    return res;
}

inline bool isOut(const game::Ball& b, const game::Paddle& p) { return b.y > p.y; }

inline bool cleared(const game::World& w) {
    for (const auto& bl : w.blocks) if (bl.alive) return false;
    return true;
}

} // namespace phys
