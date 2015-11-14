#pragma once

#include "Player.h"

class Game {
public:
	Game(int numPlayers) {
		started = false;
		this->numPlayers = numPlayers;
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

	void start() {
		started = true;
	}

	bool hasStarted() {
		return started;
	}

	void addPlayer(Player p) {
		//TODO
	}
	
	void removePlayer(Player p) {
		//TODO
	}

private:
	int numPlayers;
	Player* players;
	bool gameOver;
	bool started;
};