#pragma once
#include "resources.h"
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "input.h"
#include <unordered_map>

const float DEFAULT_VOLUME = 10.0f;
const int MAX_CHANNELS = 10;

class Audio {
public:
    Audio();
    ~Audio();

    // will play given sound in a free buffer
    void playSound(sf::SoundBuffer& sb);

    // sound will play and loop as long as this is called every frame
    // only one instance of each soundbuffer can be looped like this at a time
    void playSoundSingle(sf::SoundBuffer& sb);

    void update(GLfloat delta);

    void changeVolume(float delta);
private:
    std::vector<sf::Sound> sounds;

    int getNextFreeSound();

    float volume = DEFAULT_VOLUME;      // master game volume
    float oldVolume = DEFAULT_VOLUME;
    bool changedOldVolume = false;
    bool muted = false;

    std::unordered_map<sf::SoundBuffer*, int> loopingSounds;
    std::vector<bool> heardFromThisFrame;
};

// Global variable for Audio 
extern Audio* AudioInstance;