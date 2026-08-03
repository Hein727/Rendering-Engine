#include "AudioSource.h"
#include "Misc.h"

AudioSource::AudioSource(IXAudio2* xaudio, std::shared_ptr<AudioResource>& resource) : resource(resource)	
{
	HRESULT hr;

	hr = xaudio->CreateSourceVoice(&sourceVoice, &resource->GetWaveFormat());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
}

AudioSource::~AudioSource()
{
	if (sourceVoice != nullptr)
	{
		sourceVoice->DestroyVoice();
		sourceVoice = nullptr;
	}
}

void AudioSource::Play(bool loop)
{
	Stop();

	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = resource->GetAudioBytes();
	buffer.pAudioData = resource->GetAudioData();
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	sourceVoice->SubmitSourceBuffer(&buffer);

	HRESULT hr = sourceVoice->Start();
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	sourceVoice->SetVolume(1.0f);
}

void AudioSource::Stop()
{
	sourceVoice->FlushSourceBuffers();
	sourceVoice->Stop();
}

void AudioSource::SetVolume(float volume)
{
	sourceVoice->SetVolume(volume);
}