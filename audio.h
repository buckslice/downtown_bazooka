#pragma once
#include "resources.h"
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "input.h"

const float DEFAULT_VOLUME = 20.0f;

class Audio {
public:
    Audio();
    ~Audio();

    void playSound(sf::SoundBuffer& sb);

    void update(GLfloat delta);

    void changeVolume(float delta);
private:
    std::vector<sf::Sound> sounds;

    int getNextFreeSound();

    float volume = DEFAULT_VOLUME;      // master game volume
    float oldVolume = DEFAULT_VOLUME;
    bool changedOldVolume = false;
    bool muted = false;
};

// Global variable for Audio 
extern Audio* AudioInstance;