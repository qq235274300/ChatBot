// Fill out your copyright notice in the Description page of Project Settings.


#include "AIGameMode.h"

#include "AIAssistantFunctionLibrary.h"


LRESULT CALLBACK GlobalHotkeyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
const int32 AAIGameMode::MaxBuffsize= 64;


void AAIGameMode::BeginPlay()
{
	Super::BeginPlay();


#if PLATFORM_WINDOWS

	if (!GEngine)
	{
		return;
	}

// 	TSharedPtr<FGenericWindow> NativeWindow = GEngine->GameViewport->GetWindow()->GetNativeWindow();
// 	auto Window = static_cast<FWindowsWindow*>(NativeWindow.Get());
// 	WindowHandle = Window->GetHWnd();
// #endif // #if PLATFORM_WINDOWS
//
// 	// Alt + Shift + R
// 	if (!RegisterHotKey((HWND)WindowHandle, MY_HOTKEY_ID, MOD_ALT | MOD_SHIFT, 'R'))
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("Failed to register hotkey!"));
// 	}
//
// 	LONG_PTR OldWndProc = GetWindowLongPtr((HWND)WindowHandle, GWLP_WNDPROC);
//
// 	SetWindowLongPtr((HWND)WindowHandle, GWLP_WNDPROC, (LONG_PTR)GlobalHotkeyWndProc);
//
// 	SetWindowLongPtr((HWND)WindowHandle, GWLP_USERDATA, OldWndProc);


	RegisterGlobalHotkey();
#endif
	
}

void AAIGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if PLATFORM_WINDOWS

	if (!GEngine)
	{
		return;
	}
	if(WindowHandle == nullptr)
	{
		return;
	}
	// TSharedPtr<FGenericWindow> NativeWindow = GEngine->GameViewport->GetWindow()->GetNativeWindow();
	// auto Window = static_cast<FWindowsWindow*>(NativeWindow.Get());
	// auto WindowHandle = Window->GetHWnd();
#endif // #if PLATFORM_WINDOWS
	UnregisterHotKey((HWND)WindowHandle, MY_HOTKEY_ID);
	
	LONG_PTR OldWndProc = GetWindowLongPtr((HWND)WindowHandle, GWLP_USERDATA);
	SetWindowLongPtr((HWND)WindowHandle, GWLP_WNDPROC, OldWndProc);
	
	Super::EndPlay(EndPlayReason);
}

void AAIGameMode::HandleHotkeyEvent()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Hot Key Pressed"));
	if(bStayOnTop)
	{
		UAIAssistantFunctionLibrary::MinimizeWindow();
		bStayOnTop = false;
	}
	else
	{
		UAIAssistantFunctionLibrary::SetWindowStayOnTop();
		bStayOnTop =true;
	}
}

FString AAIGameMode::ReadRegValue(const FString& PathDir, const FString& KeyName, bool& bIsFind)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, *PathDir, 0, KEY_WOW64_64KEY | KEY_READ, &hKey) == 0)
	{
		TCHAR Buffer[MaxBuffsize];
		DWORD BufferSize = sizeof(Buffer);
		HRESULT hResult = RegQueryValueEx(hKey, *KeyName, 0, nullptr, reinterpret_cast<LPBYTE>(Buffer), &BufferSize);
		RegCloseKey(hKey);
		if (hResult != 0)
		{
			bIsFind = false;
			return FString(TEXT("Find'n Key"));
		}
		bIsFind = true;
		return FString(Buffer);
	}
	return FString(TEXT("Find'n Reg"));
}

bool AAIGameMode::WriteRegValue(const FString& PathDir, const FString& KeyName, const FString& Value)
{
	bool bSuccess = false;
	HKEY hRootKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, *PathDir, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WOW64_64KEY | KEY_ALL_ACCESS, nullptr, &hRootKey, nullptr) == 0)
	{
		LRESULT Result = RegSetValueEx(hRootKey, *KeyName, 0, REG_SZ, (const BYTE*)*Value, (Value.Len() + 1) * sizeof(TCHAR));
		RegCloseKey(hRootKey);

		if (Result == ERROR_SUCCESS)
		{
			bSuccess = true;
		}
	}
	return bSuccess;
}

bool AAIGameMode::DeleteRegKey(const FString& PathDir, const FString& KeyName)
{
	bool bSuccess = false;
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, *PathDir, 0, KEY_WOW64_64KEY | KEY_READ, &hKey) == 0)
	{
		LRESULT Result = RegDeleteKey(hKey, *KeyName);
		RegCloseKey(hKey);
		if (Result == 0)
		{
			bSuccess = true;
		}
	}
	return bSuccess;
}

void AAIGameMode::RegisterGlobalHotkey()
{
	const FString PathDir = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\123");
	const FString KeyName = TEXT("HotKey");
	const DWORD HotkeyCode = 0x40001; // Alt + Shift + R

	// Write the HotKey value to the registry
	FString Value = FString::Printf(TEXT("%d"), HotkeyCode);
	if (WriteRegValue(PathDir, KeyName, Value))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully registered global hotkey."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to register global hotkey."));
	}

	// Register the hotkey with Windows
	RegisterHotKeyForWindow();
}

void AAIGameMode::RegisterHotKeyForWindow()
{
	// Obtain the window handle
	TSharedPtr<FGenericWindow> NativeWindow = GEngine->GameViewport->GetWindow()->GetNativeWindow();
	auto Window = static_cast<FWindowsWindow*>(NativeWindow.Get());
	WindowHandle = Window->GetHWnd();

	// Register the hotkey (Alt + Shift + R)
	if (!RegisterHotKey(WindowHandle, 1, MOD_ALT | MOD_SHIFT, 'R'))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to register hotkey!"));
	}

	// Subclass the window procedure to handle hotkey messages
	LONG_PTR OldWndProc = GetWindowLongPtr(WindowHandle, GWLP_WNDPROC);
	SetWindowLongPtr(WindowHandle, GWLP_WNDPROC, (LONG_PTR)GlobalHotkeyWndProc);
	SetWindowLongPtr(WindowHandle, GWLP_USERDATA, OldWndProc);
}


LRESULT CALLBACK  GlobalHotkeyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	const int MY_HOTKEY_ID = 1;
	if (msg == WM_HOTKEY && wParam == MY_HOTKEY_ID)
	{
		AAIGameMode::HandleHotkeyEvent();
		
	}

	return CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
}
