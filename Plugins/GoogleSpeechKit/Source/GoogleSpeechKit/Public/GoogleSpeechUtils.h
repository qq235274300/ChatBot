// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace GoogleSpeechKitUtils
{

/**
    * Converts byte array to USoundWave ready to be performed by AudioComponent
    */
GOOGLESPEECHKIT_API class USoundWave* CreateSoundFromWaveDataWithHeader(const TArray<uint8> &rawData);

GOOGLESPEECHKIT_API class USoundWave* CreateSoundFromWaveDataWithoutHeader(const TArray<uint8> &rawData, uint16 InSampleRate=48000, uint8 InNumChannels=2);

/**
    * Adds wave header
    */
const TArray<uint8> GenerateWaveHeader(const TArray<uint8> &rawData, uint16 InSampleRate=16000, uint8 InNumChannels=1);


/**
    * Wraps raw audio samples with wave header and saves to a file
    */
static bool WriteRawAudioToWavFile(const TArray<uint8> &CaptureData, FString Path, uint16 InSampleRate=16000, uint8 InNumChannels=2, bool generateHeader=true);

static FString GetEnvironmentVariable(FString key);

static const TArray<uint8> ReadWavFileToBytes(const FString &AbsFilePath);
}