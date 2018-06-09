#include "Engine/Sound/Sound.h"
#include "Engine/Sound/SoundSystem.h"

namespace NovaEngine {

	Sound::Sound(SoundSystem* soundSystem, ALuint bufferId) : _system(soundSystem), _bufferId(bufferId) {
		setName("Sound Emitter");
		_activePlayer = new SoundPlayer();
		_activePlayer->setSound(this);
	}

	Sound::~Sound() {
		alDeleteBuffers(1, &_bufferId);
		delete _activePlayer;
	}

	void Sound::play() {
		//_activePlayer->play();
		SoundPlayer* p = new SoundPlayer();
		p->setSound(this);
		p->play();
	}

	void Sound::stop() {

	}

	void Sound::pause() {

	}
	void Sound::update() {
		Spatial::update();


	}
	ALuint Sound::getBufferId() const {
		return _bufferId;
	}
}