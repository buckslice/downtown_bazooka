#include "gameWindow.h"
const int WIDTH = 1024;
const int HEIGHT = 768;
int main() {
	GameWindow gameWindow(WIDTH, HEIGHT);
	gameWindow.mainLoop();

    return 0;
}