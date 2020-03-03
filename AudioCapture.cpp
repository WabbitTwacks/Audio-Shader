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
}

AudioCapture::~AudioCapture()
{
	Release();
}

void AudioCapture::Release()
{
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pCaptureClient);
}

HRESULT AudioCapture::GetStream(AudioSink* audioSink)
{
	BOOL bDone = FALSE;

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

	hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice); //use eMultimedia for audio recording
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
		0,
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

	hnsActualDuration = (double)AC_REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

	hr = pAudioClient->Start();
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	while (bDone == FALSE)
	{
		Sleep(hnsActualDuration / AC_REFTIMES_PER_MSEC / 2);

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
		}
		if (FAILED(hr))
		{
			Release();
			return hr;
		}

		if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
			pData = NULL; //write silence

		hr = audioSink->CopyData(
			pData, numFramesAvailable, &bDone
		);
		if (FAILED(hr))
		{
			Release();
			return hr;
		}
	}

	return hr;
}
