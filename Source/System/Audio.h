#pragma once
#pragma comment(lib, "xaudio2.lib")
#include <xaudio2.h>
#include <x3daudio.h>
#include <memory>
#include "AudioSource.h"

class Audio
{
public:
	Audio() = default;
	~Audio();
	void Initialize();
	std::unique_ptr<AudioSource> LoadAudioSource(const char* filename);

private:
	IXAudio2* xaudio = nullptr;
	IXAudio2MasteringVoice* masteringVoice = nullptr;
	X3DAUDIO_HANDLE x3dAudioHandle;
};