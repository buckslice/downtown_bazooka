#include "game.h"
const int WIDTH = 1024;
const int HEIGHT = 768;
int main() {
	Game game(WIDTH, HEIGHT);
    game.mainLoop();

    return 0;
}