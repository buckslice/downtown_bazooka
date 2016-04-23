#include "audio.h"

Audio* AudioInstance; //extern Audio var from Audio.h

Audio::Audio() {
    AudioInstance = this;
    sounds.resize(MAX_CHANNELS);
    heardFromThisFrame.resize(MAX_CHANNELS);
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

void Audio::playSound(sf::SoundBuffer& sb, float volume) {
    int i = getNextFreeSound();
    if (i < 0 || muted) {
        return;
    }
    sf::Sound& sound = sounds[i];
    sound.setVolume(masterVolume * volume);
    sound.setLoop(false);
    sound.setBuffer(sb);
    sound.play();
}

void Audio::playSoundSingle(sf::SoundBuffer& sb, float volume) {
    if (loopingSounds.count(&sb) == 0) {
        int i = getNextFreeSound();
        if (i < 0 || muted) {
            return;
        }
        sf::Sound& sound = sounds[i];
        sound.setVolume(masterVolume * volume);
        sound.setLoop(true);
        sound.setBuffer(sb);
        sound.play();
        loopingSounds[&sb] = i;
        heardFromThisFrame[i] = true;
    } else {
        int i = loopingSounds[&sb];
        sounds[i].setVolume(masterVolume * volume);
        heardFromThisFrame[i] = true;
    }

}

void Audio::update(GLfloat delta) {
    // updates the volume of the sounds and music
    if (muted) {
        if (!changedOldVolume) {
            oldVolume = masterVolume;
            masterVolume = 0.0f;
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
            masterVolume = oldVolume;
        }
        changedOldVolume = false;
    }
    // set music to half master volume because it drowns out sound effects otherwise
    Resources::get().mainTrack.setVolume(masterVolume*0.5f);
    Resources::get().bossTrack.setVolume(masterVolume*0.5f);
}

void Audio::changeVolume(float delta) {
    masterVolume += delta;
    if (masterVolume < 0.0f) {
        masterVolume = 0.0f;
    } else if (masterVolume > 100.0f) {
        masterVolume = 100.0f;
    }
}
