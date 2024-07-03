// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#if PLATFORM_WINDOWS
#       include <windows/WindowsWindow.h>
#include "Windows/AllowWindowsPlatformTypes.h"
#       include <windows.h>
#       include <shellapi.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif 
#include "AIGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AIASSISTANT_API AAIGameMode : public AGameModeBase
{
	GENERATED_BODY()

// protected:
// 	virtual void BeginPlay()override;
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	static void HandleHotkeyEvent();
public:
	FString ReadRegValue(const FString& PathDir, const FString& KeyName, bool& bIsFind);
	bool WriteRegValue(const FString& PathDir, const FString& KeyName, const FString& Value);
	bool DeleteRegKey(const FString& PathDir, const FString& KeyName);
	void RegisterGlobalHotkey();
	void RegisterHotKeyForWindow();
public:
	static const int MY_HOTKEY_ID = 1;
	inline static bool bStayOnTop = true;

private:
	HWND WindowHandle;
	static const int32 MaxBuffsize;
};
