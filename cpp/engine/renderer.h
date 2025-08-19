#pragma once
#include <iostream>
#include "engine/terminal.h"
#include "game/entities.h"

namespace eng {

class Renderer {
    Terminal& term_;
    game::World& world_;

    static inline int rowOf(int y) { return y + 2; }
    static inline int colOf(int x) { return x * 2 + 3; } // double-width cells

    // ANSI colors
    static constexpr const char* WHITE   = "\033[97m";
    static constexpr const char* MAGENTA = "\033[95m";
    static constexpr const char* RESET   = "\033[0m";

public:
    Renderer(Terminal& t, game::World& w) : term_(t), world_(w) {}

    void drawFence() {
        const int W = world_.N + 2;
        Terminal::gotoRC(1, 1);
        for (int x = 0; x < W; ++x) std::cout << "##";
        for (int y = 0; y < world_.N; ++y) {
            int r = 2 + y;
            Terminal::gotoRC(r, 1);               std::cout << "##";
            Terminal::gotoRC(r, (W - 1) * 2 + 1); std::cout << "##";
        }
        Terminal::gotoRC(W, 1);
        for (int x = 0; x < W; ++x) std::cout << "##";
        std::cout.flush();
    }

    inline void cell(int x, int y, const char* two) {
        Terminal::gotoRC(rowOf(y), colOf(x));
        std::cout << two;
    }
    inline void erase(int x, int y) { cell(x, y, "  "); }

    inline void eraseBlock(const game::Block& bl) {
        erase(bl.x,     bl.y);
        erase(bl.x + 1, bl.y);
    }

    // Colored 2-char cell helper (prints color, then text, then RESET)
    inline void cellColored(int x, int y, const char* color, const char* two) {
        Terminal::gotoRC(rowOf(y), colOf(x));
        std::cout << color << two << RESET;
    }

    void hud() {
        const int W = world_.N + 2;
        Terminal::gotoRC(W + 1, 1);
        Terminal::clearLine();
        std::cout << "Score: " << world_.score << "    Round: " << world_.round << std::flush;
    }

    void paddle(const game::Paddle& p, int prev_x=-1, int prev_y=-1) {
        if (prev_x >= 0 && prev_y >= 0) {
            for (int dx = -p.half; dx <= p.half; ++dx) {
                int ex = prev_x + dx;
                if (ex >= 0 && ex < world_.N) erase(ex, prev_y);
            }
        }
        for (int dx = -p.half; dx <= p.half; ++dx) {
            int cx = p.x + dx;
            if (cx >= 0 && cx < world_.N) cellColored(cx, p.y, WHITE, "██");
        }
        std::cout.flush();
    }

    void ball(const game::Ball& b, bool erase_before=false, int prev_x=-1, int prev_y=-1) {
        if (erase_before && prev_x>=0 && prev_y>=0) erase(prev_x, prev_y);
        cell(b.x, b.y, "()");
        std::cout.flush();
    }

    void block(const game::Block& bl) {
        if (!bl.alive) return;
        cellColored(bl.x,     bl.y, MAGENTA, "██");
        cellColored(bl.x + 1, bl.y, MAGENTA, "██");
    }
    void blocks() {
        for (const auto& b : world_.blocks) block(b);
        std::cout.flush();
    }
};

} // namespace eng
