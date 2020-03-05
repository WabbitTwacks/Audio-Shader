#include "AudioSink.h"

AudioSink::AudioSink()
{
}

AudioSink::~AudioSink()
{
	if (pBuffer != nullptr)
		delete[] pBuffer;
}

HRESULT AudioSink::SetFormat(WAVEFORMATEX* pwfx)
{
	wfx = *pwfx;

	wavFile.Init(wfx.nChannels, wfx.nSamplesPerSec, wfx.nAvgBytesPerSec, wfx.nBlockAlign, wfx.wBitsPerSample);

	return S_OK;
}

HRESULT AudioSink::CopyData(BYTE* pData, UINT32 numFramesAvailable, BOOL* bDone)
{
	if (numFramesAvailable < 1)
		return S_FALSE;

	numFrames = numFramesAvailable;

	const std::lock_guard<std::mutex> lock(mtxBuffer);

	if (pBuffer != nullptr)
		delete[] pBuffer;

	uint32_t nTotalBuffSize = numFramesAvailable * wfx.nBlockAlign;

	pBuffer = new BYTE[nTotalBuffSize];

	for (unsigned int i = 0; i < nTotalBuffSize; i++)
	{		
		if (pData != NULL)
		{
			pBuffer[i] = *pData;
			pData++;
		}
		else
		{
			pBuffer[i] = 0;
		}
	}

	//write to wav file for testing
	wavFile.writeFile("test.wav", pBuffer, nTotalBuffSize, true);

	return S_OK;
}

uint32_t AudioSink::GetFramesCount()
{
	return numFrames;
}

void AudioSink::GetBuffer(uint8_t* pReadBuff, uint32_t nFrames)
{
	if (numFrames.load() < nFrames)
		nFrames = numFrames.load();

	const std::lock_guard<std::mutex> lock(mtxBuffer);

	if (pBuffer == nullptr || numFrames.load() == 0)
		return;

	uint32_t nDataSize = nFrames * wfx.nBlockAlign;

	memcpy_s(pReadBuff, nDataSize, pBuffer, nDataSize);

	numFrames.store(numFrames.load() - nFrames);

	if (numFrames < 0)
		numFrames = 0;
}
