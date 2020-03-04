#include "AudioSink.h"

AudioSink::AudioSink()
{
}

AudioSink::~AudioSink()
{
}

HRESULT AudioSink::SetFormat(WAVEFORMATEX* pwfx)
{
	wfx = *pwfx;

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

	return S_OK;
}

uint32_t AudioSink::GetFramesCount()
{
	return numFrames;
}

void AudioSink::GetBuffer(uint8_t* pReadBuff, uint32_t nFrames)
{
	if (numFrames < nFrames)
		nFrames = numFrames;

	const std::lock_guard<std::mutex> lock(mtxBuffer);

	uint32_t nDataSize = nFrames * wfx.nBlockAlign;

	for (unsigned int i = 0; i < nDataSize; i++)
	{		
		pReadBuff[i] = pBuffer[i];			
	}

	numFrames -= nFrames;
}
