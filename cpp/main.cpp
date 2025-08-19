#include "engine/terminal.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/physics.h"
#include "game/entities.h"
#include "game/game.h"


int main() {
    game::Game g(/*N=*/27);
    g.run();
    return 0;
}
