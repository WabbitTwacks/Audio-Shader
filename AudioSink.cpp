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
	memset(pBuffer, 0, nBufferSize);
	nBufferPos = 0;

	wavFile.Init(wfx.nChannels, wfx.nSamplesPerSec, wfx.nAvgBytesPerSec, wfx.nBlockAlign, wfx.wBitsPerSample);

	return S_OK;
}

HRESULT AudioSink::CopyData(BYTE* pData, UINT32 numFramesAvailable, BOOL* bDone)
{
	if (numFramesAvailable < 1)
		return S_FALSE;

	numFrames += numFramesAvailable;	
	if (numFrames > nBufferSize)
		numFrames = nBufferSize;

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

int32_t AudioSink::GetFramesCount()
{
	return numFrames;
}

void AudioSink::GetUnreadBuffer(uint8_t* pReadBuff, uint32_t nFrames)
{
	if (numFrames < nFrames)
		nFrames = numFrames;

	const std::lock_guard<std::mutex> lock(mtxBuffer);

	if (pBuffer == nullptr || numFrames == 0)
		return;

	uint32_t nDataSize = nFrames * wfx.nBlockAlign;

	//memcpy_s(pReadBuff, nDataSize, pBuffer, nDataSize);
	int32_t nBuffReadPos = 0;
	int32_t nUnreadData = numFrames * wfx.nBlockAlign;
	
	nBuffReadPos = (nBufferSize + (nBufferPos - nUnreadData)) % nBufferSize;

	for (int i = 0; i < nDataSize; i++)
	{
		pReadBuff[i] = pBuffer[nBuffReadPos];

		nBuffReadPos++;
		nBuffReadPos %= nBufferSize;
	}

	numFrames -= nFrames;

	if (numFrames < 0)
		numFrames = 0;
}

void AudioSink::GetLastFrames(uint8_t* pReadBuff, uint32_t nFrames)
{
	if (nFrames < 1)
		return;

	if (nFrames > nBufferSize / wfx.nBlockAlign)
		nFrames = nBufferSize / wfx.nBlockAlign;

	int32_t nBufferReadPos = 0;
	int32_t nToRead = nFrames * wfx.nBlockAlign;

	const std::lock_guard<std::mutex> lock(mtxBuffer);

	nBufferReadPos = (nBufferSize + (nBufferPos - nToRead)) % nBufferSize;	

	for (int i = 0; i < nToRead; i++)
	{
		pReadBuff[i] = pBuffer[nBufferReadPos];

		nBufferReadPos++;
		nBufferReadPos %= nBufferSize;
	}
}
