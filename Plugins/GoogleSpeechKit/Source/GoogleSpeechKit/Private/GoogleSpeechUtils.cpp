// Copyright 2019 Ilgar Lunin. All Rights Reserved.


#include "GoogleSpeechUtils.h"
#include "Sound/SoundWave.h"
#include "Runtime/Launch/Resources/Version.h"

#if PLATFORM_WINDOWS
    #include "Runtime/Core/Public/Windows/WindowsPlatformMisc.h"
#endif

#if PLATFORM_MAC
    #include "Runtime/Core/Public/Apple/ApplePlatformMisc.h"
#endif

#if PLATFORM_LINUX
    #include "Runtime/Core/Public/Linux/LinuxPlatformMisc.h"
#endif

#include "Misc/FileHelper.h"


USoundWave* GoogleSpeechKitUtils::CreateSoundFromWaveDataWithHeader(const TArray<uint8> &rawData)
{
    USoundWave* sWave = nullptr;
    if (rawData.Num() == 0) return sWave;

	// reading wave information
	FWaveModInfo waveInfo;

    #if ENGINE_MINOR_VERSION > 19 && ENGINE_MAJOR_VERSION <= 4
        uint8* waveData = const_cast<uint8*>(rawData.GetData());
    #else
        const uint8* waveData = rawData.GetData();
    #endif

	if (waveInfo.ReadWaveInfo(waveData, rawData.Num()))
	{
	    // Construct USoundWave and feed received bytes
        sWave = NewObject<USoundWave>();

	    // apply wave info
	    int32 DurationDiv = *waveInfo.pChannels * *waveInfo.pBitsPerSample * *waveInfo.pSamplesPerSec;
	    if (DurationDiv)
		    sWave->Duration = *waveInfo.pWaveDataSize * 8.0f / DurationDiv;
	    else
		    sWave->Duration = 0.0f;
        sWave->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
        sWave->NumChannels = *waveInfo.pChannels;
        sWave->RawPCMDataSize = waveInfo.SampleDataSize;

        #if ENGINE_MINOR_VERSION < 19 && ENGINE_MAJOR_VERSION <= 4
            sWave->SampleRate = *waveInfo.pSamplesPerSec;
        #else
            sWave->SetSampleRate(*waveInfo.pSamplesPerSec);
        #endif

        sWave->bLooping = false;
        sWave->RawPCMData = (uint8*)FMemory::Malloc(sWave->RawPCMDataSize);
        const uint8 headerOffset = 44;
        FMemory::Memcpy(sWave->RawPCMData, rawData.GetData() + headerOffset, rawData.Num() - headerOffset);
	}

	return sWave;
}

class USoundWave* GoogleSpeechKitUtils::CreateSoundFromWaveDataWithoutHeader(const TArray<uint8> &rawData, uint16 InSampleRate/*=48000*/, uint8 InNumChannels/*=2*/)
{
    USoundWave* sWave = nullptr;
    if (rawData.Num() == 0) return sWave;

    // reading wave information
    FWaveModInfo waveInfo;

    #if ENGINE_MINOR_VERSION > 19 && ENGINE_MAJOR_VERSION <= 4
        uint8* waveData = const_cast<uint8*>(rawData.GetData());
    #else
        const uint8* waveData = rawData.GetData();
    #endif

    if (!waveInfo.ReadWaveInfo(waveData, rawData.Num()))
    {
        TArray<uint8> waveDataWithHeader = GenerateWaveHeader(rawData, InSampleRate, InNumChannels);

        FString errorMsg;
        if(waveInfo.ReadWaveInfo(waveDataWithHeader.GetData(), waveDataWithHeader.Num(), &errorMsg))
        {
            // Construct USoundWave and feed received bytes
            sWave = NewObject<USoundWave>();

            // apply wave info
            int32 DurationDiv = *waveInfo.pChannels * *waveInfo.pBitsPerSample * *waveInfo.pSamplesPerSec;
            if (DurationDiv)
                sWave->Duration = *waveInfo.pWaveDataSize * 8.0f / DurationDiv;
            else
                sWave->Duration = 0.0f;
            sWave->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
            sWave->NumChannels = *waveInfo.pChannels;
            sWave->RawPCMDataSize = waveInfo.SampleDataSize;

            #if ENGINE_MINOR_VERSION < 19 && ENGINE_MAJOR_VERSION <= 4
                sWave->SampleRate = *waveInfo.pSamplesPerSec;
            #else
                sWave->SetSampleRate(*waveInfo.pSamplesPerSec);
            #endif

            sWave->bLooping = false;
            sWave->RawPCMData = (uint8*)FMemory::Malloc(sWave->RawPCMDataSize);
            const uint8 headerOffset = 44;
            FMemory::Memcpy(sWave->RawPCMData, waveDataWithHeader.GetData() + headerOffset, waveDataWithHeader.Num() - headerOffset);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Error generating wave info\n%s"), *errorMsg);
        }
    }

    return sWave;
}

const TArray<uint8> GoogleSpeechKitUtils::GenerateWaveHeader(const TArray<uint8> &rawData, uint16 InSampleRate, uint8 InNumChannels)
{
    int  chunkSize = 36 + rawData.Num();
    int subchunk1Size = 16;
    short audioFormat = 1;
    short numChannels = InNumChannels;
    int sampleRate = InSampleRate;
    int byteRate = sampleRate * numChannels * rawData.GetTypeSize();
    short blockAlign = numChannels * rawData.GetTypeSize();
    short bitsPerSample = 16;
    int buffSize = rawData.Num();

    uint8 header[44];
    uint8* end = header;
    FMemory::Memcpy(end, "RIFF", 4);            end += 4;
    FMemory::Memcpy(end, &chunkSize, 4);        end += 4;
    FMemory::Memcpy(end, "WAVE", 4);            end += 4;
    FMemory::Memcpy(end, "fmt ", 4);            end += 4;
    FMemory::Memcpy(end, &subchunk1Size, 4);    end += 4;
    FMemory::Memcpy(end, &audioFormat, 2);      end += 2;
    FMemory::Memcpy(end, &numChannels, 2);      end += 2;
    FMemory::Memcpy(end, &sampleRate, 4);       end += 4;
    FMemory::Memcpy(end, &byteRate, 4);	        end += 4;
    FMemory::Memcpy(end, &blockAlign, 2);       end += 2;
    FMemory::Memcpy(end, &bitsPerSample, 2);    end += 2;
    FMemory::Memcpy(end, "data", 4);            end += 4;
    FMemory::Memcpy(end, &buffSize, 4);         end += 4;

    TArray<uint8> wav = TArray<uint8>(header, 44);
    wav.Append(rawData);
    return wav;
}


bool GoogleSpeechKitUtils::WriteRawAudioToWavFile(const TArray<uint8> &CaptureData, FString Path, uint16 InSampleRate, uint8 InNumChannels, bool generateHeader)
{
    FText err;
    if (FFileHelper::IsFilenameValidForSaving(Path, err))
    {
        if (generateHeader)
        {
            const TArray<uint8> wav = GenerateWaveHeader(CaptureData, InSampleRate, InNumChannels);
            return FFileHelper::SaveArrayToFile(wav, *Path);
        }
        else
        {
            return FFileHelper::SaveArrayToFile(CaptureData, *Path);
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Error saving capture data: %s"), *err.ToString());
    return false;
}

FString GoogleSpeechKitUtils::GetEnvironmentVariable(FString key)
{
    FString result;
    #if PLATFORM_WINDOWS
        result = FWindowsPlatformMisc::GetEnvironmentVariable(*key);
    #endif

    #if PLATFORM_MAC
        result = FApplePlatformMisc::GetEnvironmentVariable(*key);
    #endif

    #if PLATFORM_LINUX
        result = FLinuxPlatformMisc::GetEnvironmentVariable(*key);
    #endif

    return result;
}

static const TArray<uint8> GoogleSpeechKitUtils::ReadWavFileToBytes(const FString& AbsFilePath) {
    TArray<uint8> unprocessed_samples_ = {};
    bool loaded = FFileHelper::LoadFileToArray(unprocessed_samples_, *AbsFilePath);
    if (loaded) return unprocessed_samples_;
    return {};
}
