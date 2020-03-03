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
	return S_OK;
}
