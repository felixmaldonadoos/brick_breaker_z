#pragma once
#include <chrono>
#include <thread>
#include <algorithm>
#include "engine/terminal.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/physics.h"
#include "game/entities.h"

namespace game {

class Game {
    eng::Terminal term_;
    eng::Input    input_;
    World         world_;
    eng::Renderer renderer_;

    Paddle paddle_;
    Ball   ball_;

    int prev_px{-1}, prev_py{-1};
    int prev_bx{-1}, prev_by{-1};

public:
    Game(int N=27)
      : world_{N},
        renderer_(term_, world_) {
        paddle_.x = world_.N/2;
        paddle_.y = world_.N-1;
        paddle_.half = 1;

        ball_.x = world_.N/2;
        ball_.y = world_.N/2 - 2;
        ball_.vx = 1; ball_.vy = -1;
        ball_.speed = 6.0f; // start faster; try 6–10
        ball_.step_ms = 220;
    }

    void init() {
        term_.enableRaw();
        renderer_.drawFence();
        buildLevel(/*rows=*/3, /*start_row=*/1);
        renderer_.blocks();
        renderer_.hud();

        renderer_.ball(ball_);
        prev_bx = ball_.x;          // <-- add
        prev_by = ball_.y;          // <-- add

        renderer_.paddle(paddle_);
        prev_px = paddle_.x;        // <-- add (optional but consistent)
        prev_py = paddle_.y;        // <-- add
    }

    void run() {
        init();

        using clock = std::chrono::steady_clock;
        auto last = clock::now();
        float acc_cells = 0.0f;            // how many whole-cell moves are queued
        const int frame_ms = 30;

        while (true) {
            // timing
            auto now = clock::now();
            float dt = std::chrono::duration<float>(now - last).count(); // seconds
            last = now;
            acc_cells += ball_.speed * dt; // cells worth of movement to do

            // input
            switch (input_.poll()) {
                case eng::Key::Quit: return teardown("Quit.");
                case eng::Key::Left:  --paddle_.x; break;
                case eng::Key::Right: ++paddle_.x; break;
                case eng::Key::Up:    --paddle_.y; break;
                case eng::Key::Down:  ++paddle_.y; break;
                default: break;
            }
            clampPaddle();
            renderer_.paddle(paddle_, prev_px, prev_py);
            prev_px = paddle_.x; prev_py = paddle_.y;

            // ball cadence
            int steps = static_cast<int>(acc_cells);
            if (steps > 0) {
                // optional safety to avoid huge bursts after a lag spike
                if (steps > 20) steps = 20;

                for (int i = 0; i < steps; ++i) {
                    if (prev_bx >= 0) renderer_.erase(prev_bx, prev_by);

                    auto res = phys::stepBall(world_, ball_, paddle_);

                    // score ONLY for brick destroys
                    if (res.destroyedIndex >= 0) {
                        renderer_.eraseBlock(world_.blocks[res.destroyedIndex]);
                        world_.score += world_.round;

                        // tiny speed bump per brick (optional)
                        if (ball_.speed < 18.0f) ball_.speed += 0.15f;

                        renderer_.hud();
                    }

                    renderer_.ball(ball_);
                    prev_bx = ball_.x; prev_by = ball_.y;

                    if (phys::isOut(ball_, paddle_)) {
                        return teardown("GAME OVER!");
                    }

                    if (phys::cleared(world_)) {
                        ++world_.round;
                        // bigger speed jump each round
                        ball_.speed = std::min(ball_.speed + 1.0f, 24.0f);
                        renderer_.hud();
                        buildLevel(/*rows=*/3 + (world_.round - 1) % 2, /*start_row=*/1);
                        renderer_.blocks();
                    }
                }
                acc_cells -= steps;  // keep the fractional remainder
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(frame_ms));
        }
    }

private:
    void clampPaddle() {
        if (paddle_.x < paddle_.half) paddle_.x = paddle_.half;
        if (paddle_.x > world_.N - 1 - paddle_.half) paddle_.x = world_.N - 1 - paddle_.half;
        if (paddle_.y < 0) paddle_.y = 0;
        if (paddle_.y > world_.N - 1) paddle_.y = world_.N - 1;
    }

    void buildLevel(int rows, int start_row) {
        world_.blocks.clear();
        const int step_x = 3; // 2 cells + 1 gap
        const int step_y = 2; // 1 row + 1 gap
        const int max_cols = (world_.N + 1) / step_x;

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < max_cols; ++c) {
                int bx = c * step_x;
                int by = start_row + r * step_y;
                if (bx + 1 < world_.N) world_.blocks.push_back({bx, by, true});
            }
        }
    }

    void teardown(const char* msg) {
        const int W = world_.N + 2;
        eng::Terminal::gotoRC(W + 2, 1);
        eng::Terminal::hideCursor(false);
        std::cout << "\n" << msg << "  Final Score: " << world_.score
                  << "  Round: " << world_.round << "\n";
    }
};

} // namespace game
