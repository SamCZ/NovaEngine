#include "Engine/Sound/SoundPlayer.h"
#include "Engine/Sound/Sound.h"

namespace NovaEngine {
	SoundPlayer::SoundPlayer() {
		alGenSources(1, &_sourceId);

		alSourcef(_sourceId, AL_GAIN, 1.0f);
		alSourcef(_sourceId, AL_PITCH, 1.0f);
		alSource3f(_sourceId, AL_POSITION, 0, 0, 0);
	}

	SoundPlayer::~SoundPlayer() {
		alDeleteSources(1, &_sourceId);
	}

	void SoundPlayer::setSound(Sound* sound) {
		alSourcei(_sourceId, AL_BUFFER, sound->getBufferId());
	}

	void SoundPlayer::play() {
		alSourcePlay(_sourceId);
	}

	void SoundPlayer::pause() {

	}

	void SoundPlayer::stop() {

	}

	bool SoundPlayer::isPlaying() {
		ALint state;
		//AL_STOPPED
		alGetSourcei(_sourceId, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}
}