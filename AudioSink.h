#pragma once

#include <Windows.h>

class AudioSink
{
public:
	AudioSink();
	~AudioSink();

	HRESULT SetFormat(WAVEFORMATEX* pwfx);
	HRESULT CopyData(BYTE *pData, UINT32 numFramesAvailable, BOOL *bDone);

private:
	WAVEFORMATEX wfx;
};