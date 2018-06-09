#include "Engine/Sound/SoundSystem.h"

#include <vorbis/vorbisfile.h>
#include <cstdio>
#include <iostream>
#include <vector>

namespace NovaEngine {
#define BUFFER_SIZE   32768     // 32 KB buffers

	void SoundSystem::loadOGG(const char* fileName, std::vector<char>& buffer, ALenum& format, ALsizei& freq) {
		int endian = 0;             // 0 for Little-Endian, 1 for Big-Endian
		int bitStream;
		long bytes;
		char array[BUFFER_SIZE];    // Local fixed size array
		FILE *f;

		// Open for binary reading
		f = fopen(fileName, "rb");

		vorbis_info *pInfo;
		OggVorbis_File oggFile;

		ov_open(f, &oggFile, NULL, 0);

		// Get some information about the OGG file
		pInfo = ov_info(&oggFile, -1);

		// Check the number of channels... always use 16-bit samples
		if (pInfo->channels == 1)
			format = AL_FORMAT_MONO16;
		else
			format = AL_FORMAT_STEREO16;
		// end if

		// The frequency of the sampling rate
		freq = pInfo->rate;

		do {
			// Read up to a buffer's worth of decoded sound data
			bytes = ov_read(&oggFile, array, BUFFER_SIZE, endian, 2, 1, &bitStream);
			// Append to end of buffer
			buffer.insert(buffer.end(), array, array + bytes);
		} while (bytes > 0);

		ov_clear(&oggFile);
	}

	SoundSystem::~SoundSystem() {
		for (auto& i = _soundCache.begin(); i != _soundCache.end(); i++) {
			delete i->second;
		}
		alutExit();
	}

	SoundSystem::SoundSystem() {
		if (alutInit(0, nullptr)) {
			std::cout << "Alut initialized." << std::endl;
		}

	}

	Sound* SoundSystem::loadSound(const File& file) {
		ALuint bufferID;            // The OpenAL sound buffer ID
		ALenum format;              // The sound data format
		ALsizei freq;               // The frequency of the sound data
		std::vector<char> bufferData; // The sound buffer data from file
		loadOGG(file.getPath().c_str(), bufferData, format, freq);

		alGenBuffers(1, &bufferID);
		alBufferData(bufferID, format, &bufferData[0], static_cast < ALsizei > (bufferData.size()), freq);

		Sound* sound = new Sound(this, bufferID);

		ALuint error = alGetError();
		if (error != ALUT_ERROR_NO_ERROR) {
			fprintf(stderr, "%s\n", alGetString(error));
		}

		_soundCache[file.getPath()] = sound;
		return sound;
	}

	void SoundSystem::update() {
	}
}
