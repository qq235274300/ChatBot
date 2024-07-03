// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GoogleTTS.h"
#include "Sound/SoundWave.h"
#include "GoogleSpeechFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class GOOGLESPEECHKIT_API UGoogleSpeechFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:

    UFUNCTION(BlueprintPure, Category="GoogleSpeech")
    // Percentage based string comparison. Returns value from 0.0 to 1.0
    static float CompareStrings(const FString& source, const FString& target);

    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech")
    static void SetUseApiKeyFromEnvironmentVars(bool bUseEnvVariable);

    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech|TTS|Caching")
    static void SetTTSCachingEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "GoogleSpeech|TTS|Caching")
    static bool isTTSCachingEnabled();

    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech|TTS|Caching")
    static bool wipeTTSCache();

    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech|TTS|Caching")
    static void resetTTSCacheRootDirectory();

    UFUNCTION(BlueprintPure, Category = "GoogleSpeech|TTS|Caching")
        static FString getTTSCacheRootDirectory();

    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech|TTS|Caching")
    static void SetTTSCacheRootDirectory(const FString& path);

    static bool GetUseApiKeyFromEnvironmentVars();

    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech")
    static void SetApiKey(const FString& apiKey);

    UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "==", Keywords = "== equal"), Category = "GoogleSpeech|TTS")
    static bool CompareSynthesysParameters(const FGoogleSpeechSynthesisParams& lhs, const FGoogleSpeechSynthesisParams& rhs);

    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech")
    static USoundWave* SoundWaveFromRawSamples(const TArray<uint8>& rawSamples,
                                               int32 sampleRate=16000,
                                               int32 numChannels=1,
                                               bool generateHeader=false);

    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech")
    static TArray<uint8> ReadWaveFile(const FString &AbsFilePath);


    UFUNCTION(BlueprintCallable, Category = "GoogleSpeech")
    static void GrantMicrophonPermissionsApple();
    
    static FString GetApiKey();

};
