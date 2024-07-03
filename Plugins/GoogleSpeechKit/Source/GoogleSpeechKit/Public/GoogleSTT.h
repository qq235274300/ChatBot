// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#pragma once
#define GOOGLE_SPEECH_PROVIDER

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GoogleSTT.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogGoogleSTT, Log, All)


USTRUCT(BlueprintType)
struct FRecognitionVariant
{

    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GoogleSpeechKit|STT")
        FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoogleSpeechKit|STT")
        float Confidence;

    FORCEINLINE bool operator==(const FRecognitionVariant& Other) const
    {
        return Text.Equals(Other.Text) && (Confidence == Other.Confidence);
    }
};


/**
 * Api stt languages
 */
UENUM(BlueprintType)
enum class EGoogleSTTLanguage : uint8
{
    Afrikaans                           = 0, // af-ZA
    Amharic                             = 1, // am-ET
    Armenian                            = 2, // hy-AM
    Azerbaijani                         = 3, // az-AZ
    Indonesian                          = 4, // id-ID
    Malay                               = 5, // ms-MY
    BengaliBangladesh                   = 6, // bn-BD
    BengaliIndia                        = 7, // bn-IN
    Catalan                             = 8, // ca-ES
    Czech                               = 9, // cs-CZ
    Danish                              = 10, // da-DK
    German                              = 11, // de-DE
    EnglishAustralia                    = 12, // en-AU
    EnglishCanada                       = 13, // en-CA
    EnglishGhana                        = 14, // en-GH
    EnglishUK                           = 15, // en-GB
    EnglishIndia                        = 16, // en-IN
    EnglishIreland                      = 17, // en-IE
    EnglishKenya                        = 18, // en-KE
    EnglishNewZealand                   = 19, // en-NZ
    EnglishNigeria                      = 20, // en-NG
    EnglishPhilippines                  = 21, // en-PH
    EnglishSouthAfrica                  = 22, // en-ZA
    EnglishTanzania                     = 23, // en-TZ
    EnglishUnitedStates                 = 24, // en-US
    SpanishArgentina                    = 25, // es-AR
    SpanishBolivia                      = 26, // es-BO
    SpanishChile                        = 27, // es-CL
    SpanishColombia                     = 28, // es-CO
    SpanishCostaRica                    = 29, // es-CR
    SpanishEcuador                      = 30, // es-EC
    SpanishElSalvador                   = 31, // es-SV
    SpanishSpain                        = 32, // es-ES
    SpanishUnitedStates                 = 33, // es-US
    SpanishGuatemala                    = 34, // es-GT
    SpanishHonduras                     = 35, // es-HN
    SpanishMexico                       = 36, // es-MX
    SpanishNicaragua                    = 37, // es-NI
    SpanishPanama                       = 38, // es-PA
    SpanishParaguay                     = 39, // es-PY
    SpanishPeru                         = 40, // es-PE
    SpanishPuertoRico                   = 41, // es-PR
    SpanishDominicanRepublic            = 42, // es-DO
    SpanishUruguay                      = 43, // es-UY
    SpanishVenezuela                    = 44, // es-VE
    BasqueSpain                         = 45, // eu-ES
    FilipinoPhilippines                 = 46, // fil-PH
    FrenchCanada                        = 47, // fr-CA
    FrenchFrance                        = 48, // fr-FR
    GalicianSpain                       = 49, // gl-ES
    GeorgianGeorgia                     = 50, // ka-GE
    GujaratiIndia                       = 51, // gu-IN
    CroatianCroatia                     = 52, // hr-HR
    ZuluSouthAfrica                     = 53, // zu-ZA
    IcelandicIceland                    = 54, // is-IS
    ItalianItaly                        = 55, // it-IT
    JavaneseIndonesia                   = 56, // jv-ID
    KannadaIndia                        = 57, // kn-IN
    KhmerCambodia                       = 58, // km-KH
    LaoLaos                             = 59, // lo-LA
    LatvianLatvia                       = 60, // lv-LV
    LithuanianLithuania                 = 61, // lt-LT
    HungarianHungary                    = 62, // hu-HU
    MalayalamIndia                      = 63, // ml-IN
    MarathiIndia                        = 64, // mr-IN
    DutchNetherlands                    = 65, // nl-NL
    NepaliNepal                         = 66, // ne-NP
    NorwegianBokmalNorway               = 67, // nb-NO
    PolishPoland                        = 68, // pl-PL
    PortugueseBrazil                    = 69, // pt-BR
    PortuguesePortugal                  = 70, // pt-PT
    RomanianRomania                     = 71, // ro-RO
    SinhalaSriLanka                     = 72, // si-LK
    SlovakSlovakia                      = 73, // sk-SK
    SlovenianSlovenia                   = 74, // sl-SI
    SundaneseIndonesia                  = 75, // su-ID
    SwahiliTanzania                     = 76, // sw-TZ
    SwahiliKenya                        = 77, // sw-KE
    FinnishFinland                      = 78, // fi-FI
    SwedishSweden                       = 79, // sv-SE
    TamilIndia                          = 80, // ta-IN
    TamilSingapore                      = 81, // ta-SG
    TamilSriLanka                       = 82, // ta-LK
    TamilMalaysia                       = 83, // ta-MY
    TeluguIndia                         = 84, // te-IN
    VietnameseVietnam                   = 85, // vi-VN
    TurkishTurkey                       = 86, // tr-TR
    UrduPakistan                        = 87, // ur-PK
    UrduIndia                           = 88, // ur-IN
    GreekGreece                         = 89, // el-GR
    BulgarianBulgaria                   = 90, // bg-BG
    RussianRussia                       = 91, // ru-RU
    SerbianSerbia                       = 92, // sr-RS
    UkrainianUkraine                    = 93, // uk-UA
    HebrewIsrael                        = 94, // he-IL
    ArabicIsrael                        = 95, // ar-IL
    ArabicJordan                        = 96, // ar-JO
    ArabicUnitedArabEmirates            = 97, // ar-AE
    ArabicBahrain                       = 98, // ar-BH
    ArabicAlgeria                       = 99, // ar-DZ
    ArabicSaudiArabia                   = 100, // ar-SA
    ArabicIraq                          = 101, // ar-IQ
    ArabicKuwait                        = 102, // ar-KW
    ArabicMorocco                       = 103, // ar-MA
    ArabicTunisia                       = 104, // ar-TN
    ArabicOman                          = 105, // ar-OM
    ArabicStateofPalestine              = 106, // ar-PS
    ArabicQatar                         = 107, // 	ar-QA
    ArabicLebanon                       = 108, // ar-LB
    ArabicEgypt                         = 109, // ar-EG
    PersianIran                         = 110, // fa-IR
    HindiIndia                          = 111, // hi-IN
    ThaiThailand                        = 112, // th-TH
    KoreanSouthKorea                    = 113, // ko-KR
    ChineseMandarinTraditionalTaiwan    = 114, // zh-TW
    ChineseCantoneseTraditionalHongKong = 115, // yue-Hant-HK
    JapaneseJapan                       = 116, // ja-JP
    ChineseMandarinSimplifiedHongKong   = 117, // zh-HK
    ChineseMandarinSimplifiedChina      = 118 // zh
};


/**
 * Api recognition model
 */
UENUM(BlueprintType)
enum class EGoogleRecognitionModel : uint8
{
    Video = 0 UMETA(ToolTip="Use this model for transcribing audio in video clips or that includes multiple speakers. For best results, provide audio recorded at 16,000Hz or greater sampling rate. Note: This is a premium model that costs more than the standard rate.\n Note:\ten-US only"),
    PhoneCall = 1 UMETA(ToolTip = "Use this model for transcribing audio from a phone call. Typically, phone audio is recorded at 8,000Hz sampling rate.\n Note:\ten-US only"),
    CommandAndSearch = 2 UMETA(ToolTip="Use this model for transcribing shorter audio clips. Some examples include voice commands or voice search."),
    Default = 3 UMETA(ToolTip="Use this model if your audio does not fit one of the previously described models. For example, you can use this for long-form audio recordings that feature a single speaker only. Ideally, the audio is high-fidelity, recorded at 16,000Hz or greater sampling rate.")
};


/**
 * Api InteractionType
 * 
 * Use case categories that the audio recognition request can be described by.
 */
UENUM(BlueprintType)
enum class EGoogleInteractionType : uint8
{
    INTERACTION_TYPE_UNSPECIFIED = 0 UMETA(ToolTip = "Use case is either unknown or is something other than one of the other values below"),
    DISCUSSION = 1 UMETA(ToolTip = "Multiple people in a conversation or discussion. For example in a meeting with two or more people actively participating. Typically all the primary people speaking would be in the same room (if not, see PHONE_CALL)"),
    PRESENTATION = 2 UMETA(ToolTip = "One or more persons lecturing or presenting to others, mostly uninterrupted."),
    PHONE_CALL = 3 UMETA(ToolTip = "A phone-call or video-conference in which two or more people, who are not in the same room, are actively participating."),
    VOICEMAIL = 4 UMETA(ToolTip = "A recorded message intended for another person to listen to."),
    PROFESSIONALLY_PRODUCED = 5 UMETA(ToolTip = "Professionally produced audio (eg. TV Show, Podcast)."),
    VOICE_SEARCH = 6 UMETA(ToolTip = "Transcribe spoken questions and queries into text."),
    VOICE_COMMAND = 7 UMETA(ToolTip = "Transcribe voice commands, such as for controlling a device."),
    DICTATION = 8 UMETA(ToolTip = "Transcribe speech to text to create a written document, such as a text-message, email or report.")
};


/**
 * Api MicrophoneDistance
 * 
 * Enumerates the types of capture settings describing an audio file.
 */
UENUM(BlueprintType)
enum class EGoogleMicrophoneDistance : uint8
{
    MICROPHONE_DISTANCE_UNSPECIFIED = 0 UMETA(ToolTip="Audio type is not known."),
    NEARFIELD = 1 UMETA(ToolTip = "The audio was captured from a closely placed microphone. Eg. phone, dictaphone, or handheld microphone. Generally if there speaker is within 1 meter of the microphone."),
    MIDFIELD = 2 UMETA(ToolTip = "The speaker if within 3 meters of the microphone."),
    FARFIELD = 3 UMETA(ToolTip = "The speaker is more than 3 meters away from the microphone.")
};


/**
 * Api OriginalMediaType
 *
 * The original media the speech was recorded on.
 */
UENUM(BlueprintType)
enum class EGoogleOriginalMediaType : uint8
{
    ORIGINAL_MEDIA_TYPE_UNSPECIFIED = 0 UMETA(ToolTip=""),
    AUDIO = 1 UMETA(ToolTip = "The speech data is an audio recording."),
    VIDEO = 2 UMETA(ToolTip = "The speech data originally recorded on a video.")
};


/**
 * Api RecordingDeviceType
 * 
 * The type of device the speech was recorded with.
 */
UENUM(BlueprintType)
enum class EGoogleRecordingDeviceType : uint8
{
    RECORDING_DEVICE_TYPE_UNSPECIFIED = 0 UMETA(ToolTip = "The recording device is unknown."),
    SMARTPHONE = 1 UMETA(ToolTip = "Speech was recorded on a smartphone."),
    PC = 2 UMETA(ToolTip = "Speech was recorded using a personal computer or tablet."),
    PHONE_LINE = 3 UMETA(ToolTip = "Speech was recorded over a phone line."),
    VEHICLE = 4 UMETA(ToolTip = "Speech was recorded in a vehicle."),
    OTHER_OUTDOOR_DEVICE = 5 UMETA(ToolTip = "Speech was recorded outdoors."),
    OTHER_INDOOR_DEVICE = 6 UMETA(ToolTip = "Speech was recorded indoors.")
};


/**
 * Api RecognitionMetadata
 * 
 * Description of audio data to be recognized.
 */
USTRUCT(BlueprintType)
struct FGoogleRecognitionMetadata
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        EGoogleInteractionType InteractionType = EGoogleInteractionType::VOICE_SEARCH;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        EGoogleMicrophoneDistance MicrophoneDistance = EGoogleMicrophoneDistance::NEARFIELD;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        EGoogleOriginalMediaType OriginalMediaType = EGoogleOriginalMediaType::AUDIO;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        EGoogleRecordingDeviceType RecordingDeviceType = EGoogleRecordingDeviceType::PC;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        FString AudioTopic = "Short common answers";
};


/**
 * Recognition parameters
 */
USTRUCT(BlueprintType)
struct FGoogleSpeechRecognitionParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        TArray<uint8> RawSamples;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        EGoogleSTTLanguage Language = EGoogleSTTLanguage::EnglishUnitedStates;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        TArray<FString> speechContexts;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        EGoogleRecognitionModel model = EGoogleRecognitionModel::CommandAndSearch;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        FGoogleRecognitionMetadata RecognitionMetadata;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        bool bEnableAutomaticPunctuation = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        bool bProfanityFilter = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GoogleSpeechKit|STT")
        bool bUseEnhancedModels = false;
};


class FGoogleMetadataValues final
{
public:
    FGoogleMetadataValues();

    TMap<EGoogleSTTLanguage, FString> _languageValues;
    TMap<EGoogleRecognitionModel, FString> _recognitionModelValues;
    TMap<EGoogleInteractionType, FString> _interactionTypeValues;
    TMap<EGoogleMicrophoneDistance, FString> _microphoneDistanceValues;
    TMap<EGoogleOriginalMediaType, FString> _originalMediaTypeValues;
    TMap<EGoogleRecordingDeviceType, FString> _recordingDeviceTypeValues;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeechRecognitionFinishedPin, FString, Phrase, bool, Success, FString, ErrMessage);


UCLASS()
class UGoogleSTT final : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UGoogleSTT() {};

    FGoogleSpeechRecognitionParams params;

    UPROPERTY(BlueprintAssignable, Category = "GoogleSpeechKit|STT")
        FOnSpeechRecognitionFinishedPin Finished;
private:

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "GoogleSpeech")
        static UGoogleSTT* GoogleSTT(FGoogleSpeechRecognitionParams params);

    virtual void Activate() override;
    void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
private:
    FGoogleMetadataValues metadataValues;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeechRecognitionVariantsFinishedPin, const TArray<FRecognitionVariant>&, Variants, bool, Success, FString, ErrMessage);


UCLASS()
class UGoogleSTTVariants final : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UGoogleSTTVariants() {};

    FGoogleSpeechRecognitionParams params;
    void SetMaxAlternatives(int32 value);

    UPROPERTY(BlueprintAssignable, Category = "GoogleSpeechKit|STT")
        FOnSpeechRecognitionVariantsFinishedPin Finished;
private:

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "GoogleSpeech")
        static UGoogleSTTVariants* GoogleSTTVariants(FGoogleSpeechRecognitionParams paramsm, int32 maxAlternatives = 10);

    virtual void Activate() override;
    void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
private:
    FGoogleMetadataValues metadataValues;
    int32 _maxAlternatives;
};
