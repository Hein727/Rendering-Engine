#pragma once

#include <vector>
#include <Windows.h>

class AudioResource
{
public:
	AudioResource(const char* filename);
	~AudioResource() = default;

	UINT8* GetAudioData() { return data.data(); }

	UINT32 GetAudioBytes() const { return static_cast<UINT32>(data.size()); }

	const WAVEFORMATEX& GetWaveFormat() const { return wfx; }	

private:

	struct Riff
	{
		UINT32 tag;
		UINT32 size;
		UINT32 type;
	};

	struct Chunk
	{
		UINT32 tag;
		UINT32 size;
	};

	struct Fmt
	{
		UINT16 fmtId;
		UINT16 channel;
		UINT32 sampleRate;
		UINT32 transRate;
		UINT16 blockSize;
		UINT16 quantumBits;
	};

	Riff riff;
	Fmt fmt;
	std::vector<UINT8> data;
	WAVEFORMATEX wfx;
};