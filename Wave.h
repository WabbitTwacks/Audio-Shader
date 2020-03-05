#pragma once
#include <cstdint>
#include <Windows.h>

struct RIFF_Header
{
	const char id[4] = { 'R', 'I', 'F', 'F' };
	uint32_t nSize = 0;
	const char format[4] = { 'W', 'A', 'V', 'E' };
};

struct FMT_Chunk
{
	const char id[4] = { 'f', 'm', 't', ' ' };
	uint32_t nSize = 16;
	uint16_t nAudioFormat = 3; //floating point
	uint16_t nChannels = 2;
	uint32_t nSampleRate = 0;
	uint32_t nByteRate = 0;
	uint16_t nBlockAlign = 0;
	uint16_t nBitsPerSample = 0;
};

struct DATA_Chunk
{
	const char id[4] = { 'd', 'a', 't', 'a' };
	uint32_t nSize;
};

class Wave
{
public:
	Wave();
	Wave(uint16_t nChannels, uint32_t nSampleRate, uint32_t nByteRate, uint16_t nFrameSize, uint16_t nBitRate);
	~Wave();

	void Init(uint16_t nChannels, uint32_t nSampleRate, uint32_t nByteRate, uint16_t nFrameSize, uint16_t nBitRate);
	void loadData(BYTE* pData, uint64_t nSize);
	bool writeFile(const char* filename, bool bAppend = false);
	bool writeFile(const char* filename, BYTE *pData, uint32_t nLength, bool bAppend = false);
	void clear();

private:
	RIFF_Header riff;
	FMT_Chunk fmt;
	BYTE* pBuffer = nullptr;
	uint32_t nSize;
};