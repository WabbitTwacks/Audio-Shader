#pragma once

#include <Windows.h>
#include <cstdint>
#include <atomic>
#include <mutex>

#include "Wave.h"

class AudioSink
{
public:
	AudioSink();
	~AudioSink();

	HRESULT SetFormat(WAVEFORMATEX* pwfx);
	HRESULT CopyData(BYTE *pData, UINT32 numFramesAvailable, BOOL *bDone);

	int32_t GetFramesCount();
	uint8_t GetChannels() { return wfx.nChannels;  }
	uint8_t GetFrameSize() { return wfx.nBlockAlign; }
	uint32_t GetSampleRate() { return wfx.nSamplesPerSec; }
	uint16_t GetBitRate() { return wfx.wBitsPerSample; }
	uint32_t GetByteRate() { return wfx.nAvgBytesPerSec; }
	void GetUnreadBuffer(uint8_t* pReadBuff, uint32_t nFrames);
	void GetLastFrames(uint8_t* pReadBuff, uint32_t nFrames);

	template <class typeT>
	void GetLastFrames(typeT* pReadBuff, uint32_t nFrames)
	{
		if (nFrames < 1)
			return;

		if (sizeof(typeT) > wfx.wBitsPerSample / 8) //typeT does not match the data alignment
			return;

		if (nFrames > nBufferSize / wfx.nBlockAlign)
			nFrames = nBufferSize / wfx.nBlockAlign;

		int32_t nBufferReadPos = 0;
		int32_t nToRead = nFrames * wfx.nBlockAlign;
		int32_t nDataSize = sizeof(typeT);
		int32_t nOutputSize = nToRead / nDataSize;

		const std::lock_guard<std::mutex> lock(mtxBuffer);
		nBufferReadPos = (nBufferSize + (nBufferPos - nToRead)) % nBufferSize;		

		if (pBuffer == nullptr)
			return;

		for (int i = 0; i < nOutputSize; i++)
		{
			typeT* sample = (typeT*)&pBuffer[nBufferReadPos];
			pReadBuff[i] = *sample;

			nBufferReadPos += nDataSize;
			nBufferReadPos %= nBufferSize;
		}
	}



private:
	WAVEFORMATEX wfx;
	BYTE* pBuffer = nullptr;
	int32_t nBufferPos = 0;
	int32_t nBufferSize = 0;
	std::atomic<std::int32_t> numFrames = 0;
	std::mutex mtxBuffer;

	Wave wavFile;
};