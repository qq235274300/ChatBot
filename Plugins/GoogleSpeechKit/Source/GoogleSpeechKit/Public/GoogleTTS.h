// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#pragma once
#define GOOGLE_SPEECH_PROVIDER

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GoogleTTS.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogGoogleTTS, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeechSynthesisFinishedPin, USoundWave*, Sound, const TArray<uint8>&, RawSamples, bool, Success);


/**
 * Api available audio profiles
 */
UENUM(BlueprintType)
enum class EAudioEffectsProfile : uint8
{
    Wearable = 0 UMETA(DisplayName = "Wearable", ToolTip="Smart watches and other wearables, like Apple Watch, Wear OS watch."),
    Handset = 1 UMETA(DisplayName = "Handset", ToolTip = "Smartphones, like Google Pixel, Samsung Galaxy, Apple iPhone."),
    Headphone = 2 UMETA(DisplayName = "Headphone", ToolTip = "Earbuds or headphones for audio playback, like Sennheiser headphones."),
    SmallBluetoothSpeaker = 3 UMETA(DisplayName = "SmallBluetoothSpeaker", ToolTip = "Small home speakers, like Google Home Mini."),
    MediumBluetoothSpeaker = 4 UMETA(DisplayName = "MediumBluetoothSpeaker", ToolTip = "Smart home speakers, like Google Home."),
    LargeHomeEntertainment = 5 UMETA(DisplayName = "LargeHomeEntertainment", ToolTip = "Home entertainment systems or smart TVs, like Google Home Max, LG TV."),
    LargeAutomotive = 6 UMETA(DisplayName = "LargeAutomotive", ToolTip = "Car speakers."),
    Telephony = 7 UMETA(DisplayName = "Telephony", ToolTip = "Interactive Voice Response (IVR) systems.")
};


/**
 * Synthesis parameters
 */
USTRUCT(BlueprintType)
struct FGoogleSpeechSynthesisParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|TTS")
        FText Text;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|TTS")
        bool bUseSsml = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|TTS")
        float Pitch = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|TTS")
        float SpeakingRate = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|TTS")
        TArray<EAudioEffectsProfile> AudioEffectsStack;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|TTS")
        FString Language;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|TTS")
        int32 SampleRateHertz = 24000;

    inline bool operator==(const FGoogleSpeechSynthesisParams& other) const
    {
        return Text.EqualTo(other.Text) &&
            bUseSsml == other.bUseSsml &&
            Pitch == other.Pitch &&
            SpeakingRate == other.SpeakingRate &&
            Language == other.Language &&
            AudioEffectsStack == other.AudioEffectsStack;
    }

};

FORCEINLINE uint32 GetTypeHash(const FGoogleSpeechSynthesisParams& params)
{
    uint32 Hash = 0;
    Hash += GetTypeHash(params.Text.ToString());
    Hash += GetTypeHash(int32(params.bUseSsml));
    Hash += GetTypeHash(params.Pitch);
    Hash += GetTypeHash(params.SpeakingRate);
    Hash += GetTypeHash(params.Language);
    for (EAudioEffectsProfile effect : params.AudioEffectsStack)
    {
        Hash += GetTypeHash(effect);
    }
    return Hash;
}

FORCEINLINE FArchive& operator<<(FArchive& Ar, FGoogleSpeechSynthesisParams params)
{
    Ar << params.Text;
    Ar << params.bUseSsml;
    Ar << params.Pitch;
    Ar << params.SpeakingRate;
    Ar << params.AudioEffectsStack;
    Ar << params.Language;

    return Ar;
}


UCLASS()
class UGoogleTTS final : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UGoogleTTS();

    FGoogleSpeechSynthesisParams params;

    UPROPERTY(BlueprintAssignable, Category = "GoogleSpeechKit|TTS")
        FOnSpeechSynthesisFinishedPin Finished;

private:
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "GoogleSpeech")
    /*
    params.Pitch will be clamped to [-20.0 : 20.0]
    params.SpeakingRate will be clamped to [0.25 : 4.0]
    */
    static UGoogleTTS* GoogleTTS(FGoogleSpeechSynthesisParams params);

    virtual void Activate() override;
    void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);

    UPROPERTY()
        USoundWave* sWave;

    TMap<EAudioEffectsProfile, FString> _audioProfilesValues;
};
