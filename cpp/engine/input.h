#pragma once
#include <unistd.h>

namespace eng {

    enum class Key { None, Up, Down, Left, Right, Quit, Other };

    struct Input {
        Key poll() const {
            char buf[3];
            int n = ::read(STDIN_FILENO, buf, sizeof(buf));
            if (n <= 0) return Key::None;

            if (buf[0]=='q'||buf[0]=='Q') return Key::Quit;
            if (buf[0]=='w'||buf[0]=='W') return Key::Up;
            if (buf[0]=='s'||buf[0]=='S') return Key::Down;
            if (buf[0]=='a'||buf[0]=='A') return Key::Left;
            if (buf[0]=='d'||buf[0]=='D') return Key::Right;

            // Arrows: ESC [ A/B/C/D
            if (buf[0]=='\033' && n>=3 && buf[1]=='[') {
                switch (buf[2]) {
                    case 'A': return Key::Up;
                    case 'B': return Key::Down;
                    case 'C': return Key::Right;
                    case 'D': return Key::Left;
                }
            }
            return Key::Other;
        }
    };

} // namespace eng
