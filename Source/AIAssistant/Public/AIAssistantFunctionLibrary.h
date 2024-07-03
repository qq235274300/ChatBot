// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AIAssistantFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class AIASSISTANT_API UAIAssistantFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Window")
	static void SetWindowStayOnTop(bool stayOnTop = true);

	UFUNCTION(BlueprintCallable, Category = "Window")
	static void MinimizeWindow();

	
	UFUNCTION(BlueprintCallable, Category = "Time")
	static void Now();

	UFUNCTION(BlueprintCallable, Category = "Time")
	static void Timestamp();

public:
	static double currentTime;
};
