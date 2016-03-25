#pragma once
#include "resources.h"
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "input.h"
const float DEFAULT_VOLUME = 20.0f;

enum SoundEffect {
	JUMP,
	SHOOT,
	DAMAGE,
	PICKUP,
	MENU_SELECT,
	MENU_MOVE,
	SHOT_EXPLOSION
};
class Audio {
public:
	Audio();
	~Audio();

	void playMainTrack();

	void playSound(SoundEffect effect);

	void update(GLfloat delta);

	void changeVolume(float delta);
private:
	sf::Sound sound;
	float volume = DEFAULT_VOLUME;
	float oldVolume = DEFAULT_VOLUME;
	bool changedOldVolume = false;
	bool muted = false;
};

// Global variable for Audio 
extern Audio* AudioInstance;