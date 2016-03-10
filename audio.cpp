#include "audio.h"



Audio::Audio() {
	// load music track	
	if (!mainTrack.openFromFile("assets/music/expl1.ogg")) {
		std::cout << "ERROR::MUSIC_LOAD_FAILURE" << std::endl;
	}
	// set up music
	mainTrack.setLoop(true);
	mainTrack.setVolume(volume);
	
}


Audio::~Audio() {
}

void Audio::playMainTrack() {
	if (!muted) {
		mainTrack.play(); // ENSIFERUM
	}

}

void Audio::playSound(SoundEffect effect) {
	if (!muted) {
		switch (effect) {
		case JUMP:
			// play jump sound
			break;
		case SHOOT:
			// play shoot sound
			break;
		case DAMAGE:
			// play damage sound
			break;
		default:
			break;
		}
	}
}

void Audio::update(GLfloat delta) {
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
	mainTrack.setVolume(volume);

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
