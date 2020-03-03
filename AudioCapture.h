#pragma once

#include <Windows.h>
#include <WaaSApi.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

#include "AudioSink.h"

#define AC_REFTIMES_PER_SEC 10000000
#define AC_REFTIMES_PER_MSEC 10000

class AudioCapture
{
public:
    AudioCapture();
    ~AudioCapture();

    HRESULT OpenDevice(AudioSink *audioSink);
    HRESULT StartCapture();
    HRESULT GetStream(AudioSink* audioSink);

    void Release();

private:
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = AC_REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    UINT32 packetLength = 0;
    BYTE* pData;
    DWORD flags;
    IPropertyStore* pProperties = NULL;
};