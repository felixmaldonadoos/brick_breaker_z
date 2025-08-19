#pragma once
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

namespace eng {

class Terminal {
    termios orig_{};
    bool raw_{false};

public:
    Terminal() = default;
    ~Terminal() { disableRaw(); }

    void enableRaw() {
        if (raw_) return;
        tcgetattr(STDIN_FILENO, &orig_);
        termios raw = orig_;
        raw.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
        raw_ = true;
        hideCursor(true);
        clear();
        home();
    }

    void disableRaw() {
        if (!raw_) return;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_);
        raw_ = false;
        hideCursor(false);
    }

    static void gotoRC(int r, int c) { std::cout << "\033[" << r << ";" << c << "H"; }
    static void home()               { std::cout << "\033[H"; }
    static void clear()              { std::cout << "\033[2J"; }
    static void clearLine()          { std::cout << "\033[2K"; }
    static void hideCursor(bool h)   { std::cout << (h ? "\033[?25l" : "\033[?25h"); }
};

} // namespace eng
