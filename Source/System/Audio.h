#pragma once

#include <xaudio2.h>
#include "AudioSource.h"

class Audio
{
public:
	Audio() = default;
	~Audio() = default;

	void Initialize();
	void Finalize();
	AudioSource* LoadAudioSource(const char* filename);

private:
	IXAudio2* xaudio = nullptr;
	IXAudio2MasteringVoice* masteringVoice = nullptr;
};