#include "Misc.h"
#include "Audio.h"

void Audio::Initialize()
{
	HRESULT hr;

	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	UINT32 createFlags = 0;

	hr = XAudio2Create(&xaudio, createFlags);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	hr = xaudio->CreateMasteringVoice(&masteringVoice);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	DWORD channelMask;
	masteringVoice->GetChannelMask(&channelMask);

	float soundSpeed = 343.5f;

	X3DAudioInitialize(channelMask, soundSpeed, x3dAudioHandle);
}

Audio::~Audio()
{
	if (masteringVoice != nullptr)
	{
		masteringVoice->DestroyVoice();
		masteringVoice = nullptr;
	}

	if (xaudio != nullptr)
	{
		xaudio->Release();
		xaudio = nullptr;
	}

	CoUninitialize();
}

std::unique_ptr<AudioSource> Audio::LoadAudioSource(const char* filename)
{
	std::shared_ptr<AudioResource> resource = std::make_shared<AudioResource>(filename);
	return std::make_unique<AudioSource>(xaudio, resource);
}