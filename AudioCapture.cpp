#include "AudioCapture.h"

#define SAFE_RELEASE(p) \
			if ((p) != NULL) \
				{ (p)->Release(); (p) = NULL; }

AudioCapture::AudioCapture()
{
	bufferFrameCount = 0;
	flags = 0;
	hnsActualDuration = 0;
	hr = 0;
	numFramesAvailable = 0;
	pData = NULL;

	IDevice_FriendlyName = { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } };
	keyDevFriendlyName.pid = 14;
	keyDevFriendlyName.fmtid = IDevice_FriendlyName;
}

AudioCapture::~AudioCapture()
{
	Stop();
	Release();
}

HRESULT AudioCapture::Stop()
{
	captureThread.join();

	bRunning = false;

	hr = pAudioClient->Stop();
	if (FAILED(hr))
		Release();

	return hr;
}

void AudioCapture::Release()
{
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pCaptureClient);
}

HRESULT AudioCapture::OpenDevice(AudioSink *audioSink, bool bLoopback)
{
	if (audioSink == nullptr)
		return S_FALSE;

	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator
	);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	hr = pEnumerator->GetDefaultAudioEndpoint(bLoopback?eRender:eCapture,
													eMultimedia, //use eMultimedia for audio recording
													&pDevice); 
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	hr = pAudioClient->GetMixFormat(&pwfx);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	hr = pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		bLoopback?AUDCLNT_STREAMFLAGS_LOOPBACK:0, //loopback or input
		hnsRequestedDuration,
		0,
		pwfx,
		NULL
	);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	hr = pAudioClient->GetService(
		IID_IAudioCaptureClient,
		(void**)&pCaptureClient
	);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	hr = audioSink->SetFormat(pwfx);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	activeAudioSink = audioSink;

	return hr;
}

HRESULT AudioCapture::StartCapture()
{
	if (bRunning)
		Stop();

	hnsActualDuration = (double)AC_REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

	hr = pAudioClient->Start();
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	bRunning = true;
	captureThread = std::thread(&AudioCapture::GetStream, this);

	return hr;
}

std::wstring AudioCapture::GetDeviceName()
{
	if (activeAudioSink == NULL)
		return L"";

	IPropertyStore* pProperties = NULL;
	HRESULT hr = pDevice->OpenPropertyStore(STGM_READ, &pProperties);

	if (FAILED(hr))
	{
		return L"";
	}

	PROPVARIANT vName;
	PropVariantInit(&vName);
	pProperties->GetValue(keyDevFriendlyName, &vName);

	return vName.pwszVal;
}

HRESULT AudioCapture::GetStream()
{
	BOOL bDone = FALSE;
	
	while (bDone == FALSE)
	{
		//Sleep(hnsActualDuration / AC_REFTIMES_PER_MSEC / 2);
		Sleep(10);

		hr = pCaptureClient->GetNextPacketSize(&packetLength);
		if (FAILED(hr))
		{
			Release();
			return hr;
		}

		while (packetLength != 0)
		{
			hr = pCaptureClient->GetBuffer(
					&pData,
					&numFramesAvailable,
					&flags, NULL, NULL
			);

			if (FAILED(hr))
			{
				Release();
				return hr;
			}

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
				pData = NULL; //write silence

			hr = activeAudioSink->CopyData(
				pData, numFramesAvailable, &bDone
			);
			if (FAILED(hr))
			{
				Release();
				return hr;
			}

			hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
			if (FAILED(hr))
			{
				Release();
				return hr;
			}

			hr = pCaptureClient->GetNextPacketSize(&packetLength);
			if (FAILED(hr))
			{
				Release();
				return hr;
			}
		}		
	}

	return hr;
}
