#pragma once

#include <Windows.h>
#include <WaaSApi.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <thread>
#include <string>

#include "AudioSink.h"

#define AC_REFTIMES_PER_SEC 10000
#define AC_REFTIMES_PER_MSEC 10000

class AudioCapture
{
public:
    AudioCapture();
    ~AudioCapture();

    HRESULT OpenDevice(AudioSink *audioSink, bool bLoopback = false);
    HRESULT StartCapture();

    HRESULT Stop();

    bool isActive() { return bRunning; }

    std::wstring GetDeviceName();

private:
    HRESULT GetStream();
    void Release();

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

    bool bRunning = false;
    AudioSink* activeAudioSink = nullptr;
    std::thread captureThread;

    PROPERTYKEY keyDevFriendlyName;
    GUID IDevice_FriendlyName;
};