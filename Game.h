#pragma once

#include "Player.h"

class Game {
public:
	Game() {

	}

	int getNumPlayers() {
		return numPlayers;
	}

	Player* getPlayers() {
		return players;
	}

	bool gameIsOver() {
		return gameOver;
	}
	
private:
	int numPlayers;
	Player* players;
	bool gameOver;
};