#ifndef N_SOUND_H
#define N_SOUND_H

#include <AL/alut.h>
#include "Engine/Scene/Spatial.h"

namespace NovaEngine {
	class SoundSystem;
	class SoundPlayer;

	class Sound : Spatial {
	private:
		SoundSystem* _system;
		ALuint _bufferId;
		SoundPlayer* _activePlayer = nullptr;
	public:
		Sound(SoundSystem* soundSystem, ALuint bufferId);
		~Sound();

		void play();
		void stop();
		void pause();

		void update() override;

		ALuint getBufferId() const;
	};
}

#endif // !N_SOUND_H