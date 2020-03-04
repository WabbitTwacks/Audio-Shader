#pragma once

#include <Windows.h>
#include <cstdint>
#include <atomic>
#include <mutex>

class AudioSink
{
public:
	AudioSink();
	~AudioSink();

	HRESULT SetFormat(WAVEFORMATEX* pwfx);
	HRESULT CopyData(BYTE *pData, UINT32 numFramesAvailable, BOOL *bDone);

	uint32_t GetFramesCount();
	uint8_t GetChannels() { return wfx.nChannels;  }
	uint8_t GetFrameSize() { return wfx.nBlockAlign; }
	uint16_t GetSampleRate() { return wfx.nSamplesPerSec; }
	void GetBuffer(uint8_t* pReadBuff, uint32_t nFrames);

private:
	WAVEFORMATEX wfx;
	BYTE* pBuffer = nullptr;
	std::atomic<std::uint32_t> numFrames = 0;
	std::mutex mtxBuffer;
};