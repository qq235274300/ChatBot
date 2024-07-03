// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#include "GoogleTTS.h"
#include "Http.h"
#include "GoogleSpeechUtils.h"
#include "GoogleSpeechFunctionLibrary.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Base64.h"
#include "GoogleSpeechKit.h"


DEFINE_LOG_CATEGORY(LogGoogleTTS)


UGoogleTTS* UGoogleTTS::GoogleTTS(FGoogleSpeechSynthesisParams params)
{
    UGoogleTTS* BPNode = NewObject<UGoogleTTS>();

    // according to the api https://cloud.google.com/text-to-speech/docs/reference/rest/v1beta1/text/synthesize#AudioConfig
    params.Pitch = FMath::Clamp<float>(params.Pitch, -20.f, 20.f);
    params.SpeakingRate = FMath::Clamp<float>(params.SpeakingRate, 0.25f, 4.f);

    BPNode->params = params;

    return BPNode;
}


void UGoogleTTS::Activate()
{
    if (params.Text.IsEmpty())
    {
        Finished.Broadcast(nullptr, TArray<uint8>(), false);
        return;
    }

    // check cache
    FGoogleSpeechKitModule& mod = FModuleManager::Get().LoadModuleChecked<FGoogleSpeechKitModule>("GoogleSpeechKit");

    bool cacheFound = false;
    TArray<uint8> rawData = mod.findTTSCache(params, cacheFound);
    if (cacheFound)
    {
        USoundWave* cachedSound = GoogleSpeechKitUtils::CreateSoundFromWaveDataWithHeader(rawData);
        Finished.Broadcast(cachedSound, rawData, true);
        return;
    }

    FString _apiKey;
    if (UGoogleSpeechFunctionLibrary::GetUseApiKeyFromEnvironmentVars())
        _apiKey = GoogleSpeechKitUtils::GetEnvironmentVariable(TEXT("GOOGLE_API_KEY"));
    else
        _apiKey = UGoogleSpeechFunctionLibrary::GetApiKey();

    if (_apiKey.IsEmpty())
    {
        UE_LOG(LogGoogleTTS, Error, TEXT("Api key is not set"));
        Finished.Broadcast(nullptr, TArray<uint8>(), false);
        return;
    }

    auto HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(TEXT("https://texttospeech.googleapis.com/v1/text:synthesize"));
    HttpRequest->SetHeader(TEXT("X-Goog-Api-Key"), _apiKey);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));

    TSharedPtr<FJsonObject> _payloadJson = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> _input = MakeShareable(new FJsonObject());
    if (params.bUseSsml)
        _input->SetStringField("ssml", params.Text.ToString());
    else
        _input->SetStringField("text", params.Text.ToString());

    TSharedPtr<FJsonObject> _voice = MakeShareable(new FJsonObject());
    FString _voiceName = params.Language;
    FString _voiceCode = _voiceName.Mid(0, 5);
    _voice->SetStringField("languageCode", _voiceCode);
    _voice->SetStringField("name", _voiceName);

    TSharedPtr<FJsonObject> _audioConfig = MakeShareable(new FJsonObject());
    _audioConfig->SetStringField("audioEncoding", "LINEAR16");
    _audioConfig->SetNumberField("pitch", params.Pitch);
    _audioConfig->SetNumberField("speakingRate", params.SpeakingRate);
    _audioConfig->SetNumberField("sampleRateHertz", params.SampleRateHertz);

    TArray<TSharedPtr<FJsonValue>> audioEffects;
    for (EAudioEffectsProfile effect : params.AudioEffectsStack)
    {
        FString profileString = _audioProfilesValues[effect];
        audioEffects.Add(MakeShareable(new FJsonValueString(profileString)));
    }
    _audioConfig->SetArrayField("effectsProfileId", audioEffects);

    _payloadJson->SetObjectField("input", _input);
    _payloadJson->SetObjectField("voice", _voice);
    _payloadJson->SetObjectField("audioConfig", _audioConfig);

    FString _payload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&_payload);
    FJsonSerializer::Serialize(_payloadJson.ToSharedRef(), Writer);

    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetContentAsString(_payload);

    if (HttpRequest->ProcessRequest())
    {
        HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGoogleTTS::OnResponse);
    }
    else
    {
        UE_LOG(LogGoogleTTS, Warning, TEXT("Error sending request"));
        Finished.Broadcast(nullptr, TArray<uint8>(), false);
    }
}


void UGoogleTTS::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
    if (!WasSuccessful)
    {
        UE_LOG(LogGoogleTTS, Warning, TEXT("Error processing request.\n%s\n%s"), *Response->GetContentAsString(), *Response->GetURL());
        if (Finished.IsBound())
        {
            Finished.Broadcast(nullptr, TArray<uint8>(), false);
        }

        return;
    }

    // check error message. If not enough rights, generate new IAM token
    // or return false
    TSharedPtr<FJsonObject> response;
    TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (FJsonSerializer::Deserialize(reader, response))
    {
        bool err = response->HasField("error");

        if (err)
        {
            UE_LOG(LogGoogleTTS, Warning, TEXT("api error: %s"), *Response->GetContentAsString());
            return;
        }

        FString audioContentb64 = response->GetStringField("audioContent");
        TArray<uint8> rawData;

        // according to docs (https://cloud.google.com/text-to-speech/docs/reference/rest/v1beta1/text/synthesize#response-body)
        // wave already contains header info and it is encoded to base64
        FBase64::Decode(audioContentb64, rawData);
        sWave = GoogleSpeechKitUtils::CreateSoundFromWaveDataWithHeader(rawData);
        if (sWave != nullptr)
        {
            if (Finished.IsBound())
            {
                FGoogleSpeechKitModule& mod = FModuleManager::Get().LoadModuleChecked<FGoogleSpeechKitModule>("GoogleSpeechKit");
                if (mod.isTTSCachingEnabled())
                {
                    FString relativeFilePath = FString::Printf(TEXT("%s.wav"), *FGuid().NewGuid().ToString());
                    if (mod.cacheTTS(relativeFilePath, params, rawData))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Cache been saved to %s"), *relativeFilePath);
                    }
                }
                Finished.Broadcast(sWave, rawData, WasSuccessful);
            }
        }
    }
}


UGoogleTTS::UGoogleTTS()
{
    _audioProfilesValues.Add(EAudioEffectsProfile::Wearable, TEXT("wearable-class-device"));
    _audioProfilesValues.Add(EAudioEffectsProfile::Handset, TEXT("handset-class-device"));
    _audioProfilesValues.Add(EAudioEffectsProfile::Headphone, TEXT("headphone-class-device"));
    _audioProfilesValues.Add(EAudioEffectsProfile::SmallBluetoothSpeaker, TEXT("small-bluetooth-speaker-class-device"));
    _audioProfilesValues.Add(EAudioEffectsProfile::MediumBluetoothSpeaker, TEXT("medium-bluetooth-speaker-class-device"));
    _audioProfilesValues.Add(EAudioEffectsProfile::LargeHomeEntertainment, TEXT("large-home-entertainment-class-device"));
    _audioProfilesValues.Add(EAudioEffectsProfile::LargeAutomotive, TEXT("large-automotive-class-device"));
    _audioProfilesValues.Add(EAudioEffectsProfile::Telephony, TEXT("telephony-class-application"));
}
