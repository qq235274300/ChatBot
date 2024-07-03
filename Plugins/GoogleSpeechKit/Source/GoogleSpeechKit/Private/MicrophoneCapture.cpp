// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#include "MicrophoneCapture.h"
#include "Voice.h"
#include "GoogleSpeechUtils.h"
#include "Misc/FileHelper.h"

// Sets default values for this component's properties
UMicrophoneCapture::UMicrophoneCapture()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


bool UMicrophoneCapture::BeginCapture(int32 SampleRate, const FString deviceName)
{
    if (bIsCaptureActive) return false;

    if (!FVoiceModule::Get().DoesPlatformSupportVoiceCapture())
    {
        UE_LOG(LogTemp, Log, TEXT("VoiceCapture is not supported on this platform!"));
        return false;
    }

    if (!VoiceCapture.IsValid())
    {
        VoiceCapture = FVoiceModule::Get().CreateVoiceCapture(deviceName);
        if (VoiceCapture.IsValid())
        {
            FString DeviceName;
            if (!VoiceCapture->Init(DeviceName, SampleRate, 1))
                return false;
            sample_rate_ = SampleRate;
            UE_LOG(LogTemp, Log, TEXT("IVoiceCapture initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to obtain IVoiceCapture, no voice available!"));
            return false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Capture started"));
    RecordedSamples.Empty();
    VoiceCapture->Start();
    bIsCaptureActive = true;

    return true;
}

void UMicrophoneCapture::FinishCapture(TArray<uint8> &CaptureData, int32 &SamplesRecorded)
{
    if (!bIsCaptureActive) return;
    bIsCaptureActive = false;

    if (RecordedSamples.Num() == 0) return;

    SamplesRecorded = RecordedSamples.Num() / 2;

    // add wave header to data
    CaptureData = GoogleSpeechKitUtils::GenerateWaveHeader(RecordedSamples, sample_rate_, 1);

    if (VoiceCapture.IsValid())
        VoiceCapture->Stop();
}


// Called when the game starts
void UMicrophoneCapture::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMicrophoneCapture::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (VoiceCapture.IsValid() && bIsCaptureActive)
    {
        uint32 VoiceCaptureBytesAvailable = 0;
        EVoiceCaptureState::Type CaptureState = VoiceCapture->GetCaptureState(VoiceCaptureBytesAvailable);

        if (CaptureState == EVoiceCaptureState::Ok && VoiceCaptureBytesAvailable > 0)
        {
            uint32 VoiceCaptureReadBytes = 0;
            uint32 record_start = RecordedSamples.AddUninitialized(VoiceCaptureBytesAvailable);
            VoiceCapture->GetVoiceData(RecordedSamples.GetData() + record_start * sizeof(uint8), VoiceCaptureBytesAvailable, VoiceCaptureReadBytes);
        }
    }
}

