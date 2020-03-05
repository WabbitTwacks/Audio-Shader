#include <fstream>
#include <intrin.h>
#include <memory>

#include "Wave.h"
using namespace std;

Wave::Wave()
{
	nSize = 0;
}

Wave::Wave(uint16_t nChannels, uint32_t nSampleRate, uint32_t nByteRate, uint16_t nFrameSize, uint16_t nBitRate)
{
	Init(nChannels, nSampleRate, nByteRate, nFrameSize, nBitRate);
}

Wave::~Wave()
{
	if (pBuffer != nullptr)
		delete[] pBuffer;
}

void Wave::Init(uint16_t nChannels, uint32_t nSampleRate, uint32_t nByteRate, uint16_t nFrameSize, uint16_t nBitRate)
{
	nSize = 0;

	fmt.nChannels = nChannels;
	fmt.nSampleRate = nSampleRate;
	fmt.nByteRate = nByteRate;
	fmt.nBlockAlign = nFrameSize;
	fmt.nBitsPerSample = nBitRate;
}

void Wave::loadData(BYTE* pData, uint64_t nLength)
{
	if (pData == nullptr)
		return;

	if (pBuffer != nullptr)
		delete[] pBuffer;

	pBuffer = new BYTE[nLength];
	memcpy_s(pBuffer, nLength, pData, nLength);

	nSize = nLength;
}

bool Wave::writeFile(const char* filename, bool bAppend)
{
	if (!bAppend) //new file
	{
		riff.nSize = 36 + nSize;

		DATA_Chunk data;
		data.nSize = nSize;

		ofstream waveFile;
		waveFile.open(filename, ios::out | ios::binary);
		waveFile.write(reinterpret_cast<const char*>(&riff), sizeof(riff));
		waveFile.write(reinterpret_cast<const char*>(&fmt), sizeof(fmt));
		waveFile.write(reinterpret_cast<const char*>(&data), sizeof(data));

		uint8_t sampleSize = fmt.nBlockAlign / fmt.nChannels;
		BYTE* p = pBuffer;

		for (unsigned int i = 0; i < nSize; i += sampleSize)
		{
			float* sample = (float*)p;
			p += sampleSize;

			waveFile.write(reinterpret_cast<const char*>(sample), sampleSize);
		}
		
		waveFile.flush();
		waveFile.close();		
	}
	else //append
	{
		fstream waveFile;
		waveFile.open(filename, ios::in | ios::out | ios::binary | ios::ate);
		int32_t nFileSize = waveFile.tellp();

		waveFile.seekp(4, ios::beg);
		int32_t newSize = nFileSize + nSize - 8;
		waveFile.write(reinterpret_cast<const char*>(&newSize), 4); //update file size 

		waveFile.seekp(40, ios::beg);
		newSize = nFileSize + nSize - 44;
		waveFile.write(reinterpret_cast<const char*>(&newSize), 4); //update data size

		waveFile.seekp(0, ios::end);
		uint8_t sampleSize = fmt.nBlockAlign / 2;
		BYTE* p = pBuffer;

		for (unsigned int i = 0; i + sampleSize < (uint32_t)nSize; i += sampleSize)
		{
			float* sample = (float*)p;
			p += sampleSize;

			waveFile.write(reinterpret_cast<const char*>(sample), sampleSize);
		}

		waveFile.flush();
		waveFile.close();
	}

	return true;
}

bool Wave::writeFile(const char* filename, BYTE* pData, uint32_t nLength, bool bAppend)
{
	if (pData == nullptr || nLength == 0)
		return false;

	if (!bAppend) //new file
	{
		riff.nSize = 36 + nLength;

		DATA_Chunk data;
		data.nSize = nLength;

		ofstream waveFile;
		waveFile.open(filename, ios::out | ios::binary);
		waveFile.write(reinterpret_cast<const char*>(&riff), sizeof(riff));
		waveFile.write(reinterpret_cast<const char*>(&fmt), sizeof(fmt));
		waveFile.write(reinterpret_cast<const char*>(&data), sizeof(data));

		uint8_t sampleSize = fmt.nBlockAlign / fmt.nChannels;
		//uint8_t* sample = new uint8_t[sampleSize];
		BYTE* p = pData;
		
		for (unsigned int i = 0; i < nLength; i += sampleSize)
		{			
			//for (int s = 0; s < sampleSize; s++)
			//{
			//	sample[s] = pData[i + sampleSize - s - 1]; //swap byte order
			//}

			float* sample = (float*)p;
			p += sampleSize;

			waveFile.write(reinterpret_cast<const char*>(sample), sampleSize);			
		}
		//delete[] sample;

		waveFile.flush();
		waveFile.close();
	}
	else //append
	{
		fstream waveFile;
		waveFile.open(filename, ios::in | ios::out | ios::binary | ios::ate);
		uint32_t nFileSize = waveFile.tellp();

		waveFile.seekp(4, ios::beg);
		uint32_t newSize = nFileSize + nLength - 8;
		waveFile.write(reinterpret_cast<const char*>(&newSize), 4); //update file size 

		waveFile.seekp(40, ios::beg);
		newSize = nFileSize + nLength - 44;
		waveFile.write(reinterpret_cast<const char*>(&newSize), 4); //update data size

		waveFile.seekp(0, ios::end);
		uint32_t sampleSize = fmt.nBlockAlign / 2;
		//BYTE* sample = new BYTE[sampleSize];

		BYTE* p = pData;
		
		for (unsigned int i = 0; i + sampleSize < (uint32_t)nLength; i += sampleSize)
		{
			float* sample = (float*)p;
			p += sampleSize;
			//BYTE* sample = new BYTE[sampleSize];
			//unique_ptr<BYTE> sample = make_unique<BYTE>(sampleSize);
			//for (int s = 0; s < sampleSize; s++)
			//{
			//	//sample[s] = pData[i + sampleSize - s - 1]; //swap byte order
			//	sample[s] = pData[i + s];
			//}

			waveFile.write(reinterpret_cast<const char*>(sample), sampleSize);
			/*const char a[4] = "abc";
			waveFile.write(a, sampleSize);*/

			//delete[] sample;
		}

		//delete[] sample;

		waveFile.flush();
		waveFile.close();
	}

	return true;
}

void Wave::clear()
{
	delete[] pBuffer;
	pBuffer = nullptr;
}
