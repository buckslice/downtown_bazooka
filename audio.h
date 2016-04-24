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
    // volume is value from 0-1 representing percent of master volume
    void playSound(sf::SoundBuffer& sb, float volume = 1.0f);

    // sound will play and loop as long as this is called every frame
    // only one instance of a particular soundbuffer can be looped like this at a time
    // volume is value from 0-1 representing percent of master volume
    void playSoundSingle(sf::SoundBuffer& sb, float volume = 1.0f);

    void update(GLfloat delta);

    void changeVolume(float delta);
private:
    int getNextFreeSound();

    std::vector<sf::Sound> sounds;
    std::unordered_map<sf::SoundBuffer*, int> loopingSounds;
    std::vector<bool> heardFromThisFrame;

    float masterVolume = DEFAULT_VOLUME;      // master game volume
    float oldVolume = DEFAULT_VOLUME;
    bool changedOldVolume = false;
    bool muted = false;

};

// Global variable for Audio 
extern Audio* AudioInstance;