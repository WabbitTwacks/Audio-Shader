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

	if (pBuffer != nullptr)
		delete[] pBuffer;

	nBufferSize = wfx.nSamplesPerSec * wfx.nBlockAlign;
	pBuffer = new BYTE[nBufferSize];
	nBufferPos = 0;

	wavFile.Init(wfx.nChannels, wfx.nSamplesPerSec, wfx.nAvgBytesPerSec, wfx.nBlockAlign, wfx.wBitsPerSample);

	return S_OK;
}

HRESULT AudioSink::CopyData(BYTE* pData, UINT32 numFramesAvailable, BOOL* bDone)
{
	if (numFramesAvailable < 1)
		return S_FALSE;

	numFrames.store(numFrames.load() + numFramesAvailable);	
	if (numFrames.load() > wfx.nSamplesPerSec)
		numFrames.store(wfx.nSamplesPerSec);

	const std::lock_guard<std::mutex> lock(mtxBuffer);

	//if (pBuffer != nullptr)
		//delete[] pBuffer;

	uint32_t nCopySize = numFramesAvailable * wfx.nBlockAlign;

	//write to wav file for testing
	//wavFile.writeFile("test.wav", pData, nCopySize, true);

	//pBuffer = new BYTE[nCopySize];

	for (unsigned int i = 0; i < nCopySize; i++)
	{
		if (pData != NULL)
		{
			pBuffer[nBufferPos] = *pData;
			pData++;
		}
		else
		{
			pBuffer[nBufferPos] = 0;
		}

		nBufferPos++;
		nBufferPos %= nBufferSize;
	}

	return S_OK;
}

uint32_t AudioSink::GetFramesCount()
{
	return numFrames.load();
}

void AudioSink::GetBuffer(uint8_t* pReadBuff, uint32_t nFrames)
{
	if (numFrames.load() < nFrames)
		nFrames = numFrames.load();

	const std::lock_guard<std::mutex> lock(mtxBuffer);

	if (pBuffer == nullptr || numFrames.load() == 0)
		return;

	uint32_t nDataSize = nFrames * wfx.nBlockAlign;

	//memcpy_s(pReadBuff, nDataSize, pBuffer, nDataSize);
	uint32_t nBuffReadPos = 0;
	uint32_t nUnreadData = numFrames.load() * wfx.nBlockAlign;
	if (nBufferPos >= nUnreadData)
		nBuffReadPos = nBufferPos - nUnreadData;
	else
		nBuffReadPos = nUnreadData - nBufferPos;

	for (int i = 0; i < nDataSize; i++)
	{
		pReadBuff[i] = pBuffer[nBuffReadPos];

		nBuffReadPos++;
		nBuffReadPos %= nBufferSize;
	}

	numFrames.store(numFrames.load() - nFrames);

	if (numFrames < 0)
		numFrames = 0;
}
