#include "audio.h"

Audio* AudioInstance; //extern Audio var from Audio.h

Audio::Audio() {
    AudioInstance = this;
    sounds.resize(MAX_CHANNELS);
    heardFromThisFrame.resize(MAX_CHANNELS);
    Resources::get().mainTrack.setVolume(volume);
    Resources::get().menuTrack.setVolume(volume);
}

Audio::~Audio() {
}

int Audio::getNextFreeSound() {
    for (int i = 0; i < MAX_CHANNELS; ++i) {
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
    sound.setLoop(false);
    sound.setBuffer(sb);
    sound.play();
}

void Audio::playSoundSingle(sf::SoundBuffer& sb) {
    if (loopingSounds.count(&sb) == 0) {
        int i = getNextFreeSound();
        if (i < 0 || muted) {
            return;
        }
        sf::Sound& sound = sounds[i];
        sound.setVolume(volume);
        sound.setLoop(true);
        sound.setBuffer(sb);
        sound.play();
        loopingSounds[&sb] = i;
        heardFromThisFrame[i] = true;
    } else {
        int i = loopingSounds[&sb];
        sounds[i].setVolume(volume);
        heardFromThisFrame[i] = true;
    }

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
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (sounds[i].getLoop()) {
                if (!heardFromThisFrame[i]) {
                    sounds[i].setLoop(false);
                    sounds[i].stop();
                    loopingSounds.erase((sf::SoundBuffer*)(sounds[i].getBuffer()));
                }
            }
            heardFromThisFrame[i] = false;
        }

    }
    if (Input::justPressed(sf::Keyboard::M)) {
        muted = !muted;
        if (!muted) {
            volume = oldVolume;
        }
        changedOldVolume = false;
    }
    Resources::get().mainTrack.setVolume(volume*0.5f);
    Resources::get().menuTrack.setVolume(volume*0.5f);
}

void Audio::changeVolume(float delta) {
    volume += delta;
    if (volume < 0.0f) {
        volume = 0.0f;
    } else if (volume > 100.0f) {
        volume = 100.0f;
    }
}
