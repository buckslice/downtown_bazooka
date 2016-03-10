#pragma once
#include <SFML/Audio.hpp>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "input.h"
const float DEFAULT_VOLUME = 20.0f;

enum SoundEffect {
	JUMP,
	SHOOT,
	DAMAGE,
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
	sf::Music mainTrack;
	float volume = DEFAULT_VOLUME;
	float oldVolume = DEFAULT_VOLUME;
	bool changedOldVolume = false;
	bool muted = false;
};

