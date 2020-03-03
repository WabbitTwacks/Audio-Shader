#include "AudioSink.h"

AudioSink::AudioSink()
{
}

AudioSink::~AudioSink()
{
}

HRESULT AudioSink::SetFormat(WAVEFORMATEX* wfx)
{
	return E_NOTIMPL;
}

HRESULT AudioSink::CopyData(BYTE* pData, UINT32 numFramesAvailable, BOOL* bDone)
{
	return E_NOTIMPL;
}
