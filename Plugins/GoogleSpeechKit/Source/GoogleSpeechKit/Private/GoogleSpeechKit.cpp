// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#include "GoogleSpeechKit.h"
#include "GoogleSpeechUtils.h"
#include "Sound/SoundWave.h"
#include "HAL/PlatformFilemanager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Audio.h"

#define LOCTEXT_NAMESPACE "FGoogleSpeechKitModule"

void FGoogleSpeechKitModule::StartupModule()
{
    resetTTSCacheRootDirectory();
}

void FGoogleSpeechKitModule::ShutdownModule()
{

}

void FGoogleSpeechKitModule::saveData(const TMap<FGoogleSpeechSynthesisParams, FString>& data, const FString& filePath)
{
    FBufferArchive ar;
    for (auto pair : data)
    {
        ar << pair.Key.Text;
        ar << pair.Key.bUseSsml;
        ar << pair.Key.Pitch;
        ar << pair.Key.SpeakingRate;
        ar << pair.Key.AudioEffectsStack;
        ar << pair.Key.Language;
        ar << pair.Value;
    }
    if (ar.Num() <= 0)
    {
        return;
    }

    if (FFileHelper::SaveArrayToFile(ar, *filePath))
    {
        ar.FlushCache();
        ar.Empty();
    }
}

TMap<FGoogleSpeechSynthesisParams, FString> FGoogleSpeechKitModule::loadData(const FString& filePath)
{
    TMap<FGoogleSpeechSynthesisParams, FString> result;

    TArray<uint8> rawData;
    if (FFileHelper::LoadFileToArray(rawData, *filePath))
    {
        FMemoryReader memReader = FMemoryReader(rawData);

        while (!memReader.AtEnd())
        {
            FGoogleSpeechSynthesisParams _params;

            FText _text;
            bool _bUseSsml;
            float _pitch, _speakingRate;
            TArray<EAudioEffectsProfile> _audioEffectsStack;
            FString _lang;

            FString path;
            memReader << _text << _bUseSsml << _pitch << _speakingRate << _audioEffectsStack << _lang;
            memReader << path;

            _params.Text = _text;
            _params.bUseSsml = _bUseSsml;
            _params.Pitch = _pitch;
            _params.SpeakingRate = _speakingRate;
            _params.AudioEffectsStack = _audioEffectsStack;
            _params.Language = _lang;

            result.Add(_params, path);
        }

        if (!memReader.Close())
        {
            UE_LOG(LogTemp, Error, TEXT("Bad archive, failed to write speech parameters."));
        }
    }
    return result;
}

TArray<uint8> FGoogleSpeechKitModule::findTTSCache(const FGoogleSpeechSynthesisParams params, bool& bFound)
{
    if (!bCacheEnabled)
    {
        bFound = false;
        return TArray<uint8>();
    }

    bFound = false;

    auto ttsCache = loadData(getCacheDataPath());

    if (ttsCache.Contains(params))
    {
        IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
        FString cacheFileName = ttsCache[params];
        FString absFilePath = FPaths::Combine(_ttsCacheRoot, cacheFileName);
        if (platformFile.FileExists(*absFilePath))
        {
            TArray<uint8> fileRawData;
            if (FFileHelper::LoadFileToArray(fileRawData, *absFilePath))
            {
                bFound = true;
                return fileRawData;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Cache was found but file does not exists!\nRoot dir: %s\n File name: %s"),
                   *_ttsCacheRoot, *absFilePath);
        }
    }
    return TArray<uint8>();
}


bool FGoogleSpeechKitModule::cacheTTS(const FString& filePath, FGoogleSpeechSynthesisParams params, TArray<uint8>& rawData)
{
    FString absolutePath = FPaths::Combine(_ttsCacheRoot, filePath);

    FWaveModInfo waveInfo;
    uint8* waveData = const_cast<uint8*>(rawData.GetData());
    if (waveInfo.ReadWaveInfo(waveData, rawData.Num()))
    {
        if (GoogleSpeechKitUtils::WriteRawAudioToWavFile(rawData, absolutePath, *waveInfo.pSamplesPerSec, *waveInfo.pChannels, false))
        {
            IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
            FString cacheDataFilePath = getCacheDataPath();
            if (!platformFile.FileExists(*cacheDataFilePath))
            {
                TMap<FGoogleSpeechSynthesisParams, FString> data = {
                    {params, filePath}
                };

                saveData(data, cacheDataFilePath);
            }
            else
            {
                auto data = loadData(cacheDataFilePath);
                FString relativePath = FPaths::Combine("GoogleTTSCache", filePath);
                data.Add(params, filePath);
                saveData(data, cacheDataFilePath);
            }
            return true;
        }
        return false;
    }

    return false;
}

void FGoogleSpeechKitModule::setTTSCacheRootDirectory(const FString& newRoot)
{
    _ttsCacheRoot = FPaths::Combine(newRoot, TEXT("GoogleTTSCache"));
}

void FGoogleSpeechKitModule::resetTTSCacheRootDirectory()
{
    _ttsCacheRoot = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GoogleTTSCache"));
}

bool FGoogleSpeechKitModule::wipeCache()
{
    IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (platformFile.DirectoryExists(*_ttsCacheRoot))
        return platformFile.DeleteDirectoryRecursively(*_ttsCacheRoot);
    return false;
}

FString FGoogleSpeechKitModule::getCacheDataPath()
{
    return FPaths::Combine(_ttsCacheRoot, TEXT("TTSCache.sav"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGoogleSpeechKitModule, GoogleSpeechKit)