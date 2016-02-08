#include "game.h"
const GLuint WIDTH = 1024;
const GLuint HEIGHT = 768;
int main() {
	Game game(WIDTH, HEIGHT);
    game.mainLoop();

    return 0;
}