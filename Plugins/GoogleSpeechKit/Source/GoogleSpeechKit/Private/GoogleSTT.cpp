// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#include "GoogleSTT.h"
#include "Http.h"
#include "GoogleSpeechUtils.h"
#include "GoogleSpeechFunctionLibrary.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Base64.h"

DEFINE_LOG_CATEGORY(LogGoogleSTT)


UGoogleSTT* UGoogleSTT::GoogleSTT(FGoogleSpeechRecognitionParams params)
{
    UGoogleSTT* BPNode = NewObject<UGoogleSTT>();
    BPNode->params = params;
    return BPNode;
}


void UGoogleSTT::Activate()
{
    FString _apiKey;
    if (UGoogleSpeechFunctionLibrary::GetUseApiKeyFromEnvironmentVars())
        _apiKey = GoogleSpeechKitUtils::GetEnvironmentVariable(TEXT("GOOGLE_API_KEY"));
    else
        _apiKey = UGoogleSpeechFunctionLibrary::GetApiKey();

    if (_apiKey.IsEmpty())
    {
        UE_LOG(LogGoogleSTT, Error, TEXT("Api key is not set"));
        Finished.Broadcast(TEXT(""), false, TEXT("Api key is not set"));
        return;
    }

    // read wave header data
    FWaveModInfo wave_info;
    FString err;
    if (!wave_info.ReadWaveInfo(params.RawSamples.GetData(), params.RawSamples.Num(), &err)) {
        FString error = FString::Printf(TEXT("Unable to read wave info: %s"), *err);
        UE_LOG(LogGoogleSTT, Error, TEXT("%s"), *error);
        Finished.Broadcast(TEXT(""), false, *error);
        return;
    }

    const uint32 sample_rate = *wave_info.pSamplesPerSec;
    const bool is_mono = *wave_info.pChannels == 1;

    auto HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(TEXT("https://speech.googleapis.com/v1/speech:recognize"));
    HttpRequest->SetHeader(TEXT("X-Goog-Api-Key"), _apiKey);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));

    TSharedPtr<FJsonObject> _payloadJson = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> _config = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> _audio = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> _meta = MakeShareable(new FJsonObject());

    _meta->SetStringField("interactionType", metadataValues._interactionTypeValues[params.RecognitionMetadata.InteractionType]);
    _meta->SetStringField("microphoneDistance", metadataValues._microphoneDistanceValues[params.RecognitionMetadata.MicrophoneDistance]);
    _meta->SetStringField("originalMediaType", metadataValues._originalMediaTypeValues[params.RecognitionMetadata.OriginalMediaType]);
    _meta->SetStringField("recordingDeviceType", metadataValues._recordingDeviceTypeValues[params.RecognitionMetadata.RecordingDeviceType]);
    _meta->SetStringField("audioTopic", params.RecognitionMetadata.AudioTopic);

    _config->SetStringField("encoding", "LINEAR16");
    _config->SetNumberField("sampleRateHertz", sample_rate);
    _config->SetStringField("languageCode", metadataValues._languageValues[params.Language]);
    _config->SetBoolField("profanityFilter", params.bProfanityFilter);
    _config->SetBoolField("enableAutomaticPunctuation", params.bEnableAutomaticPunctuation);
    _config->SetBoolField("useEnhanced", params.bUseEnhancedModels);
    _config->SetStringField("model", metadataValues._recognitionModelValues[params.model]);
    _config->SetObjectField("metadata", _meta);
    //audioChannelCount
    _config->SetNumberField("audioChannelCount", is_mono ? 1 : 2);

    TSharedPtr<FJsonObject> _context = MakeShareable(new FJsonObject());
    TArray<TSharedPtr<FJsonValue>> phrases;

    for (FString phrase : params.speechContexts)
        phrases.AddUnique(MakeShareable(new FJsonValueString(phrase)));

    _context->SetArrayField("phrases", phrases);

    TArray<TSharedPtr<FJsonValue>> _contextsArray;
    _contextsArray.Add(MakeShareable(new FJsonValueObject(_context)));
    _config->SetArrayField("speechContexts", _contextsArray);

    TArray<uint8> samples_no_header(params.RawSamples.GetData(), params.RawSamples.Num());
    for (int i = 0; i < 44; ++i) samples_no_header.RemoveAt(0);

    FString encodedAudio = FBase64::Encode(samples_no_header);
    _audio->SetStringField("content", encodedAudio);

    _payloadJson->SetObjectField("config", _config);
    _payloadJson->SetObjectField("audio", _audio);

    FString _payload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&_payload);
    FJsonSerializer::Serialize(_payloadJson.ToSharedRef(), Writer);

    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetContentAsString(_payload);

    if (HttpRequest->ProcessRequest())
    {
        HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGoogleSTT::OnResponse);
    }
    else
    {
        UE_LOG(LogGoogleSTT, Warning, TEXT("Error sending request"));
    }
}

void UGoogleSTT::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
    if (!WasSuccessful)
    {
        UE_LOG(LogGoogleSTT, Warning, TEXT("Error processing request.\n%s\n%s"), *Response->GetContentAsString(), *Response->GetURL());
        if (Finished.IsBound())
        {
            Finished.Broadcast(TEXT(""), false, TEXT("Error processing request"));
        }
        return;
    }

    if (Response->GetResponseCode() == 200)
    {
        TSharedPtr<FJsonObject> res;
        TSharedRef<TJsonReader<> > reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

        if (FJsonSerializer::Deserialize(reader, res))
        {
            if (res->HasField("results"))
            {
                const TArray<TSharedPtr<FJsonValue>> &results = res->GetArrayField("results");
                if (results.Num() > 0)
                {
                    TSharedPtr<FJsonObject> result = results[0]->AsObject();
                    const TArray<TSharedPtr<FJsonValue>> alternatives = result->GetArrayField("alternatives");
                    TSharedPtr<FJsonObject> alternative = alternatives[0]->AsObject();
                    FString transcript = alternative->GetStringField("transcript");
                    float confidence = alternative->GetNumberField("confidence");
                    Finished.Broadcast(transcript, true, TEXT(""));
                    return;
                }
            }
            else
            {
                Finished.Broadcast(TEXT(""), false, TEXT("No results! Try to speak louder"));
                return;
            }
        }
    }

    // process other response codes
    // ...

    FString error = FString::Printf(TEXT("Response Code: %d"), Response->GetResponseCode());
    Finished.Broadcast(TEXT(""), false, error);
}

FGoogleMetadataValues::FGoogleMetadataValues()
{
    _languageValues.Add(EGoogleSTTLanguage::Afrikaans, TEXT("af-ZA"));
    _languageValues.Add(EGoogleSTTLanguage::Amharic, TEXT("am-ET"));
    _languageValues.Add(EGoogleSTTLanguage::Armenian, TEXT("hy-AM"));
    _languageValues.Add(EGoogleSTTLanguage::Azerbaijani, TEXT("az-AZ"));
    _languageValues.Add(EGoogleSTTLanguage::Indonesian, TEXT("id-ID"));
    _languageValues.Add(EGoogleSTTLanguage::Malay, TEXT("ms-MY"));
    _languageValues.Add(EGoogleSTTLanguage::BengaliBangladesh, TEXT("bn-BD"));
    _languageValues.Add(EGoogleSTTLanguage::BengaliIndia, TEXT("bn-IN"));
    _languageValues.Add(EGoogleSTTLanguage::Catalan, TEXT("ca-ES"));
    _languageValues.Add(EGoogleSTTLanguage::Czech, TEXT("cs-CZ"));
    _languageValues.Add(EGoogleSTTLanguage::Danish, TEXT("da-DK"));
    _languageValues.Add(EGoogleSTTLanguage::German, TEXT("de-DE"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishAustralia, TEXT("en-AU"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishCanada, TEXT("en-CA"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishGhana, TEXT("en-GH"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishUK, TEXT("en-GB"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishIndia, TEXT("en-IN"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishIreland, TEXT("en-IE"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishKenya, TEXT("en-KE"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishNewZealand, TEXT("en-NZ"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishNigeria, TEXT("en-NG"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishPhilippines, TEXT("en-PH"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishSouthAfrica, TEXT("en-ZA"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishTanzania, TEXT("en-TZ"));
    _languageValues.Add(EGoogleSTTLanguage::EnglishUnitedStates, TEXT("en-US"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishArgentina, TEXT("es-AR"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishBolivia, TEXT("es-BO"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishChile, TEXT("es-CL"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishColombia, TEXT("es-CO"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishCostaRica, TEXT("es-CR"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishEcuador, TEXT("es-EC"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishElSalvador, TEXT("es-SV"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishSpain, TEXT("es-ES"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishUnitedStates, TEXT("es-US"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishGuatemala, TEXT("es-GT"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishHonduras, TEXT("es-HN"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishMexico, TEXT("es-MX"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishNicaragua, TEXT("es-NI"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishPanama, TEXT("es-PA"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishParaguay, TEXT("es-PY"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishPeru, TEXT("es-PE"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishPuertoRico, TEXT("es-PR"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishDominicanRepublic, TEXT("es-DO"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishUruguay, TEXT("es-UY"));
    _languageValues.Add(EGoogleSTTLanguage::SpanishVenezuela, TEXT("es-VE"));
    _languageValues.Add(EGoogleSTTLanguage::BasqueSpain, TEXT("eu-ES"));
    _languageValues.Add(EGoogleSTTLanguage::FilipinoPhilippines, TEXT("fil-PH"));
    _languageValues.Add(EGoogleSTTLanguage::FrenchCanada, TEXT("fr-CA"));
    _languageValues.Add(EGoogleSTTLanguage::FrenchFrance, TEXT("fr-FR"));
    _languageValues.Add(EGoogleSTTLanguage::GalicianSpain, TEXT("gl-ES"));
    _languageValues.Add(EGoogleSTTLanguage::GeorgianGeorgia, TEXT("ka-GE"));
    _languageValues.Add(EGoogleSTTLanguage::GujaratiIndia, TEXT("gu-IN"));
    _languageValues.Add(EGoogleSTTLanguage::CroatianCroatia, TEXT("hr-HR"));
    _languageValues.Add(EGoogleSTTLanguage::ZuluSouthAfrica, TEXT("zu-ZA"));
    _languageValues.Add(EGoogleSTTLanguage::IcelandicIceland, TEXT("is-IS"));
    _languageValues.Add(EGoogleSTTLanguage::ItalianItaly, TEXT("it-IT"));
    _languageValues.Add(EGoogleSTTLanguage::JavaneseIndonesia, TEXT("jv-ID"));
    _languageValues.Add(EGoogleSTTLanguage::KannadaIndia, TEXT("kn-IN"));
    _languageValues.Add(EGoogleSTTLanguage::KhmerCambodia, TEXT("km-KH"));
    _languageValues.Add(EGoogleSTTLanguage::LaoLaos, TEXT("lo-LA"));
    _languageValues.Add(EGoogleSTTLanguage::LatvianLatvia, TEXT("lv-LV"));
    _languageValues.Add(EGoogleSTTLanguage::LithuanianLithuania, TEXT("lt-LT"));
    _languageValues.Add(EGoogleSTTLanguage::HungarianHungary, TEXT("hu-HU"));
    _languageValues.Add(EGoogleSTTLanguage::MalayalamIndia, TEXT("ml-IN"));
    _languageValues.Add(EGoogleSTTLanguage::MarathiIndia, TEXT("mr-IN"));
    _languageValues.Add(EGoogleSTTLanguage::DutchNetherlands, TEXT("nl-NL"));
    _languageValues.Add(EGoogleSTTLanguage::NepaliNepal, TEXT("ne-NP"));
    _languageValues.Add(EGoogleSTTLanguage::NorwegianBokmalNorway, TEXT("nb-NO"));
    _languageValues.Add(EGoogleSTTLanguage::PolishPoland, TEXT("pl-PL"));
    _languageValues.Add(EGoogleSTTLanguage::PortugueseBrazil, TEXT("pt-BR"));
    _languageValues.Add(EGoogleSTTLanguage::PortuguesePortugal, TEXT("pt-PT"));
    _languageValues.Add(EGoogleSTTLanguage::RomanianRomania, TEXT("ro-RO"));
    _languageValues.Add(EGoogleSTTLanguage::SinhalaSriLanka, TEXT("si-LK"));
    _languageValues.Add(EGoogleSTTLanguage::SlovakSlovakia, TEXT("sk-SK"));
    _languageValues.Add(EGoogleSTTLanguage::SlovenianSlovenia, TEXT("sl-SI"));
    _languageValues.Add(EGoogleSTTLanguage::SundaneseIndonesia, TEXT("su-ID"));
    _languageValues.Add(EGoogleSTTLanguage::SwahiliTanzania, TEXT("sw-TZ"));
    _languageValues.Add(EGoogleSTTLanguage::SwahiliKenya, TEXT("sw-KE"));
    _languageValues.Add(EGoogleSTTLanguage::FinnishFinland, TEXT("fi-FI"));
    _languageValues.Add(EGoogleSTTLanguage::SwedishSweden, TEXT("sv-SE"));
    _languageValues.Add(EGoogleSTTLanguage::TamilIndia, TEXT("ta-IN"));
    _languageValues.Add(EGoogleSTTLanguage::TamilSingapore, TEXT("ta-SG"));
    _languageValues.Add(EGoogleSTTLanguage::TamilSriLanka, TEXT("ta-LK"));
    _languageValues.Add(EGoogleSTTLanguage::TamilMalaysia, TEXT("ta-MY"));
    _languageValues.Add(EGoogleSTTLanguage::TeluguIndia, TEXT("te-IN"));
    _languageValues.Add(EGoogleSTTLanguage::VietnameseVietnam, TEXT("vi-VN"));
    _languageValues.Add(EGoogleSTTLanguage::TurkishTurkey, TEXT("tr-TR"));
    _languageValues.Add(EGoogleSTTLanguage::UrduPakistan, TEXT("ur-PK"));
    _languageValues.Add(EGoogleSTTLanguage::UrduIndia, TEXT("ur-IN"));
    _languageValues.Add(EGoogleSTTLanguage::GreekGreece, TEXT("el-GR"));
    _languageValues.Add(EGoogleSTTLanguage::BulgarianBulgaria, TEXT("bg-BG"));
    _languageValues.Add(EGoogleSTTLanguage::RussianRussia, TEXT("ru-RU"));
    _languageValues.Add(EGoogleSTTLanguage::SerbianSerbia, TEXT("sr-RS"));
    _languageValues.Add(EGoogleSTTLanguage::UkrainianUkraine, TEXT("uk-UA"));
    _languageValues.Add(EGoogleSTTLanguage::HebrewIsrael, TEXT("he-IL"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicIsrael, TEXT("ar-IL"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicJordan, TEXT("ar-JO"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicUnitedArabEmirates, TEXT("ar-AE"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicBahrain, TEXT("ar-BH"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicAlgeria, TEXT("ar-DZ"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicSaudiArabia, TEXT("ar-SA"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicIraq, TEXT("ar-IQ"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicKuwait, TEXT("ar-KW"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicMorocco, TEXT("ar-MA"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicTunisia, TEXT("ar-TN"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicOman, TEXT("ar-OM"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicStateofPalestine, TEXT("ar-PS"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicQatar, TEXT("  ar-QA"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicLebanon, TEXT("ar-LB"));
    _languageValues.Add(EGoogleSTTLanguage::ArabicEgypt, TEXT("ar-EG"));
    _languageValues.Add(EGoogleSTTLanguage::PersianIran, TEXT("fa-IR"));
    _languageValues.Add(EGoogleSTTLanguage::HindiIndia, TEXT("hi-IN"));
    _languageValues.Add(EGoogleSTTLanguage::ThaiThailand, TEXT("th-TH"));
    _languageValues.Add(EGoogleSTTLanguage::KoreanSouthKorea, TEXT("ko-KR"));
    _languageValues.Add(EGoogleSTTLanguage::ChineseMandarinTraditionalTaiwan, TEXT("zh-TW"));
    _languageValues.Add(EGoogleSTTLanguage::ChineseCantoneseTraditionalHongKong, TEXT("yue-Hant-HK"));
    _languageValues.Add(EGoogleSTTLanguage::JapaneseJapan, TEXT("ja-JP"));
    _languageValues.Add(EGoogleSTTLanguage::ChineseMandarinSimplifiedHongKong, TEXT("zh-HK"));
    _languageValues.Add(EGoogleSTTLanguage::ChineseMandarinSimplifiedChina, TEXT("zh"));

    _recognitionModelValues.Add(EGoogleRecognitionModel::Video, "video");
    _recognitionModelValues.Add(EGoogleRecognitionModel::PhoneCall, "phone_call");
    _recognitionModelValues.Add(EGoogleRecognitionModel::CommandAndSearch, "command_and_search");
    _recognitionModelValues.Add(EGoogleRecognitionModel::Default, "default");

    _interactionTypeValues.Add(EGoogleInteractionType::INTERACTION_TYPE_UNSPECIFIED, "INTERACTION_TYPE_UNSPECIFIED");
    _interactionTypeValues.Add(EGoogleInteractionType::DISCUSSION, "DISCUSSION");
    _interactionTypeValues.Add(EGoogleInteractionType::PRESENTATION, "PRESENTATION");
    _interactionTypeValues.Add(EGoogleInteractionType::PHONE_CALL, "PHONE_CALL");
    _interactionTypeValues.Add(EGoogleInteractionType::VOICEMAIL, "VOICEMAIL");
    _interactionTypeValues.Add(EGoogleInteractionType::PROFESSIONALLY_PRODUCED, "PROFESSIONALLY_PRODUCED");
    _interactionTypeValues.Add(EGoogleInteractionType::VOICE_SEARCH, "VOICE_SEARCH");
    _interactionTypeValues.Add(EGoogleInteractionType::VOICE_COMMAND, "VOICE_COMMAND");
    _interactionTypeValues.Add(EGoogleInteractionType::DICTATION, "DICTATION");

    _microphoneDistanceValues.Add(EGoogleMicrophoneDistance::MICROPHONE_DISTANCE_UNSPECIFIED, "MICROPHONE_DISTANCE_UNSPECIFIED");
    _microphoneDistanceValues.Add(EGoogleMicrophoneDistance::NEARFIELD, "NEARFIELD");
    _microphoneDistanceValues.Add(EGoogleMicrophoneDistance::MIDFIELD, "MIDFIELD");
    _microphoneDistanceValues.Add(EGoogleMicrophoneDistance::FARFIELD, "FARFIELD");

    _originalMediaTypeValues.Add(EGoogleOriginalMediaType::ORIGINAL_MEDIA_TYPE_UNSPECIFIED, "ORIGINAL_MEDIA_TYPE_UNSPECIFIED");
    _originalMediaTypeValues.Add(EGoogleOriginalMediaType::AUDIO, "AUDIO");
    _originalMediaTypeValues.Add(EGoogleOriginalMediaType::VIDEO, "VIDEO");

    _recordingDeviceTypeValues.Add(EGoogleRecordingDeviceType::RECORDING_DEVICE_TYPE_UNSPECIFIED, "RECORDING_DEVICE_TYPE_UNSPECIFIED");
    _recordingDeviceTypeValues.Add(EGoogleRecordingDeviceType::SMARTPHONE, "SMARTPHONE");
    _recordingDeviceTypeValues.Add(EGoogleRecordingDeviceType::PC, "PC");
    _recordingDeviceTypeValues.Add(EGoogleRecordingDeviceType::PHONE_LINE, "PHONE_LINE");
    _recordingDeviceTypeValues.Add(EGoogleRecordingDeviceType::VEHICLE, "VEHICLE");
    _recordingDeviceTypeValues.Add(EGoogleRecordingDeviceType::OTHER_OUTDOOR_DEVICE, "OTHER_OUTDOOR_DEVICE");
    _recordingDeviceTypeValues.Add(EGoogleRecordingDeviceType::OTHER_INDOOR_DEVICE, "OTHER_INDOOR_DEVICE");
}



// Variants based TTS

UGoogleSTTVariants* UGoogleSTTVariants::GoogleSTTVariants(FGoogleSpeechRecognitionParams params, int32 maxAlternatives)
{
    UGoogleSTTVariants* BPNode = NewObject<UGoogleSTTVariants>();
    BPNode->SetMaxAlternatives(maxAlternatives);
    BPNode->params = params;
    return BPNode;
}

void UGoogleSTTVariants::Activate()
{
    FString _apiKey;
    if (UGoogleSpeechFunctionLibrary::GetUseApiKeyFromEnvironmentVars())
        _apiKey = GoogleSpeechKitUtils::GetEnvironmentVariable(TEXT("GOOGLE_API_KEY"));
    else
        _apiKey = UGoogleSpeechFunctionLibrary::GetApiKey();

    if (_apiKey.IsEmpty())
    {
        UE_LOG(LogGoogleSTT, Error, TEXT("Api key is not set"));
        Finished.Broadcast({}, false, TEXT("Api key is not set"));
        return;
    }

    auto HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(TEXT("https://speech.googleapis.com/v1/speech:recognize"));
    HttpRequest->SetHeader(TEXT("X-Goog-Api-Key"), _apiKey);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));

    TSharedPtr<FJsonObject> _payloadJson = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> _config = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> _audio = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> _meta = MakeShareable(new FJsonObject());

    _meta->SetStringField("interactionType", metadataValues._interactionTypeValues[params.RecognitionMetadata.InteractionType]);
    _meta->SetStringField("microphoneDistance", metadataValues._microphoneDistanceValues[params.RecognitionMetadata.MicrophoneDistance]);
    _meta->SetStringField("originalMediaType", metadataValues._originalMediaTypeValues[params.RecognitionMetadata.OriginalMediaType]);
    _meta->SetStringField("recordingDeviceType", metadataValues._recordingDeviceTypeValues[params.RecognitionMetadata.RecordingDeviceType]);
    _meta->SetStringField("audioTopic", params.RecognitionMetadata.AudioTopic);

    _config->SetStringField("encoding", "LINEAR16");
    _config->SetNumberField("sampleRateHertz", 16000.0);
    _config->SetStringField("languageCode", metadataValues._languageValues[params.Language]);

    _config->SetNumberField("maxAlternatives", _maxAlternatives);
    _config->SetBoolField("profanityFilter", false);
    _config->SetBoolField("useEnhanced", params.bUseEnhancedModels);
    _config->SetStringField("model", metadataValues._recognitionModelValues[params.model]);
    _config->SetObjectField("metadata", _meta);

    TSharedPtr<FJsonObject> _context = MakeShareable(new FJsonObject());
    TArray<TSharedPtr<FJsonValue>> phrases;

    for (FString phrase : params.speechContexts)
        phrases.AddUnique(MakeShareable(new FJsonValueString(phrase)));

    _context->SetArrayField("phrases", phrases);

    TArray<TSharedPtr<FJsonValue>> _contextsArray;
    _contextsArray.Add(MakeShareable(new FJsonValueObject(_context)));
    _config->SetArrayField("speechContexts", _contextsArray);

    FString encodedAudio = FBase64::Encode(params.RawSamples);
    _audio->SetStringField("content", encodedAudio);

    _payloadJson->SetObjectField("config", _config);
    _payloadJson->SetObjectField("audio", _audio);

    FString _payload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&_payload);
    FJsonSerializer::Serialize(_payloadJson.ToSharedRef(), Writer);

    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetContentAsString(_payload);

    if (HttpRequest->ProcessRequest())
    {
        HttpRequest->OnProcessRequestComplete().BindUObject(this, &UGoogleSTTVariants::OnResponse);
    }
    else
    {
        UE_LOG(LogGoogleSTT, Warning, TEXT("Error sending request"));
        Finished.Broadcast({}, false, TEXT("Error sending request"));
    }
}


void UGoogleSTTVariants::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
    if (!WasSuccessful)
    {
        UE_LOG(LogGoogleSTT, Warning, TEXT("Error processing request.\n%s\n%s"), *Response->GetContentAsString(), *Response->GetURL());
        if (Finished.IsBound())
        {
            Finished.Broadcast({}, false, TEXT(""));
        }
        return;
    }

    if (Response->GetResponseCode() == 200)
    {
        
        TSharedPtr<FJsonObject> res;
        TSharedRef<TJsonReader<> > reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        if (FJsonSerializer::Deserialize(reader, res))
        {
            if (res->HasField("results"))
            {
                const TArray<TSharedPtr<FJsonValue>>& results = res->GetArrayField("results");
                if (results.Num() > 0)
                {
                    TSharedPtr<FJsonObject> resultValue = results[0]->AsObject();
                    if (resultValue->HasField("alternatives"))
                    {
                        const TArray<TSharedPtr<FJsonValue>>& alternatives = resultValue->GetArrayField("alternatives");
                        TArray<FRecognitionVariant> variants;
                        for (const TSharedPtr<FJsonValue> &alternativeValue : alternatives)
                        {
                            const TSharedPtr<FJsonObject> &alternativeObject = alternativeValue->AsObject();
                            FString Text = alternativeObject->GetStringField("transcript");
                            float confidence = alternativeObject->GetNumberField("confidence");
                            FRecognitionVariant variant; variant.Text = Text; variant.Confidence = confidence;
                            variants.AddUnique(variant);
                        }
                        if (variants.Num() > 0)
                        {
                            Finished.Broadcast(variants, true, TEXT(""));
                            return;
                        }
                    }
                }
            }
            else
            {
                Finished.Broadcast({}, false, TEXT("No results! Try to speak louder"));
                return;
            }
        }
    }

    // process other response codes
    // ...

    Finished.Broadcast({}, false, TEXT(""));
}


void UGoogleSTTVariants::SetMaxAlternatives(int32 value)
{
    _maxAlternatives = FMath::Clamp<int32>(value, 0, 30);
}
