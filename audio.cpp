#include "audio.h"

Audio* AudioInstance; //extern Audio var from Audio.h

Audio::Audio() {
    AudioInstance = this;
    sounds.resize(10);
}

Audio::~Audio() {
}

int Audio::getNextFreeSound() {
    for (size_t i = 0; i < sounds.size(); ++i) {
        if (sounds[i].getStatus() != sf::SoundSource::Status::Playing) {
            return i;
        }
    }
    return -1;
}

void Audio::playSound(sf::SoundBuffer& sb) {
    int i = getNextFreeSound();
    if (i < 0 || muted) {
        return;
    }
    sf::Sound& sound = sounds[i];
    sound.setVolume(volume);
    sound.setBuffer(sb);
    sound.play();
}

void Audio::update(GLfloat delta) {
    // updates the volume of the sounds and music
    if (muted) {
        if (!changedOldVolume) {
            oldVolume = volume;
            volume = 0.0f;
            changedOldVolume = true;
        }
    } else {
        if (Input::pressed(sf::Keyboard::Dash)) {
            changeVolume(-10.0f * delta);
        }
        if (Input::pressed(sf::Keyboard::Equal)) {
            changeVolume(10.0f * delta);
        }
    }
    if (Input::justPressed(sf::Keyboard::M)) {
        muted = !muted;
        if (!muted) {
            volume = oldVolume;
        }
        changedOldVolume = false;
    }
    Resources::get().mainTrack.setVolume(volume);
    Resources::get().menuTrack.setVolume(volume);
}

void Audio::changeVolume(float delta) {
    volume += delta;
    if (volume < 0.0f) {
        volume = 0.0f;
    } else if (volume > 100.0f) {
        volume = 100.0f;
    }
}
