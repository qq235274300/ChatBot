// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GoogleTTS.h"
#include "Modules/ModuleManager.h"

class FGoogleSpeechKitModule : public IModuleInterface
{
    friend class UGoogleSpeechFunctionLibrary;
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static void saveData(const TMap<FGoogleSpeechSynthesisParams, FString>& data, const FString& filePath);
    static TMap<FGoogleSpeechSynthesisParams, FString> loadData(const FString& filePath);

    TArray<uint8> findTTSCache(const FGoogleSpeechSynthesisParams params, bool &bFound);
    bool cacheTTS(const FString& filePath, FGoogleSpeechSynthesisParams params, TArray<uint8>& rawData);
    void setTTSCacheRootDirectory(const FString& newRoot);
    void resetTTSCacheRootDirectory();
    bool wipeCache();
    FString getTTSCacheRootDirectory() { return _ttsCacheRoot; };
    bool isTTSCachingEnabled() { return bCacheEnabled; };
    void setTTSCachingEnabled(bool bEnabled) { bCacheEnabled = bEnabled; };

private:
    FString getCacheDataPath();
    bool bUseApiKeyFromEnvironment = false;
    bool bCacheEnabled = true;
    FString _apiKey = "";
    FString _ttsCacheRoot;
};
