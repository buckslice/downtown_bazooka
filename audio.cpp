#include "audio.h"

Audio* AudioInstance; //extern Audio var from Audio.h

Audio::Audio() {
	AudioInstance = this;
	// set up music
	Resources::get().mainTrack.setLoop(true);
	Resources::get().mainTrack.setVolume(volume);
	sound.setVolume(volume);
}


Audio::~Audio() {
}

void Audio::playMainTrack() {
	if (!muted) {
		Resources::get().mainTrack.play(); // ENSIFERUM
	}

}

void Audio::playSound(SoundEffect effect) {
	if (!muted) {
		switch (effect) {
		case JUMP:
			// play jump sound
			sound.setBuffer(Resources::get().jumpSound);
			sound.play();
			break;
		case SHOOT:
			// play shoot sound
			sound.setBuffer(Resources::get().shootSound);
			sound.play();
			break;
		case DAMAGE:
			// play damage sound
			sound.setBuffer(Resources::get().damageSound);
			sound.play();
			break;
		case PICKUP:
			// play item sound
			sound.setBuffer(Resources::get().itemSound);
			sound.play();
			break;
		case MENU_SELECT:
			// play menu select sound
			sound.setBuffer(Resources::get().menuSelectSound);
			sound.play();
			break;
		case MENU_MOVE:
			// play menu move sound
			sound.setBuffer(Resources::get().menuMoveSound);
			sound.play();
			break;
		case SHOT_EXPLOSION:
			// play menu move sound
			sound.setBuffer(Resources::get().explosionSound);
			sound.play();
			break;
		default:
			std::cout << "ERROR::AUDIO::PLAY_SOUND::FAILED TO PLAY SOUND" << std::endl;
			break;
		}
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
	}
	if (Input::justPressed(sf::Keyboard::M)) {
		muted = !muted;
		if (!muted) {
			volume = oldVolume;
		}
		changedOldVolume = false;
	}
	Resources::get().mainTrack.setVolume(volume);
	sound.setVolume(volume);
}

void Audio::changeVolume(float delta) {
	volume += delta;
	if (volume > 100.0f) {
		volume = 100.0f;
	}
	if (volume < 0.0f) {
		volume = 0.0f;
	}
}
