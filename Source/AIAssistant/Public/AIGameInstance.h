// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AIGameInstance.generated.h"

/**
 * 
 */ 
UCLASS(BlueprintType)
class AIASSISTANT_API UAIGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init()override;

	UFUNCTION(BlueprintCallable)
	void SetWindowPosition(FVector2D WindowSize);
private:
	int32 ScreenWidth ;
	int32 ScreenHeight ;


};
