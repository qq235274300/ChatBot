// Copyright 2019 Ilgar Lunin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MicrophoneCapture.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOOGLESPEECHKIT_API UMicrophoneCapture : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMicrophoneCapture();

    UPROPERTY(BlueprintReadOnly, Category = "MicrophoneCapture")
    bool bIsCaptureActive = false;

    UFUNCTION(BlueprintCallable, Category = "MicrophoneCapture")
    /*
    If `Device Name` is empty, default microphone will be selected
    */
    bool BeginCapture(int32 SampleRate = 16000, const FString deviceName="");

    UFUNCTION(BlueprintCallable, Category = "MicrophoneCapture")
    void FinishCapture(TArray<uint8> &CaptureData, int32 &SamplesRecorded);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    TSharedPtr<class IVoiceCapture> VoiceCapture;
    TArray<uint8> RecordedSamples;
    int32 sample_rate_;
};
