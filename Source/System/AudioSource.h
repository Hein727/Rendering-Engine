#pragma once
#include <memory>
#include <xaudio2.h>
#include "AudioResource.h"

class AudioSource
{
public :
	AudioSource(IXAudio2* xaudio, std::shared_ptr<AudioResource>& resources);
	~AudioSource();

	void Play(bool loop);

	void Stop();	

	void SetVolume(float volume);	

private:
	IXAudio2SourceVoice* sourceVoice = nullptr;
	std::shared_ptr<AudioResource> resource;
};