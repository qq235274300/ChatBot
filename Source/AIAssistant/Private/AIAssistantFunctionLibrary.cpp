// Fill out your copyright notice in the Description page of Project Settings.

#include "AIAssistantFunctionLibrary.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include <windows.h>
#include "Windows/PostWindowsApi.h"
#include "Misc/DateTime.h"
#include "Windows/HideWindowsPlatformTypes.h"

double UAIAssistantFunctionLibrary::currentTime = 0;

void UAIAssistantFunctionLibrary::SetWindowStayOnTop(bool stayOnTop )
{
    if (GEngine && GEngine->GameViewport && GEngine->GameViewport->GetWindow())
    {
        if (auto NativeWindowPtr = GEngine->GameViewport->GetWindow()->GetNativeWindow())
        {
            Windows::HWND handle = (Windows::HWND)NativeWindowPtr->GetOSWindowHandle();
            ::ShowWindow(handle, SW_RESTORE);
            ::SetWindowPos(handle, stayOnTop? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        }
    }
}

void UAIAssistantFunctionLibrary::MinimizeWindow()
{
    if (GEngine && GEngine->GameViewport && GEngine->GameViewport->GetWindow())
    {
        if (auto NativeWindowPtr = GEngine->GameViewport->GetWindow()->GetNativeWindow())
        {
            Windows::HWND handle = (Windows::HWND)NativeWindowPtr->GetOSWindowHandle();
            ::ShowWindow(handle, SW_MINIMIZE);
            ::SetWindowPos(handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        }
    }
}

void UAIAssistantFunctionLibrary::Now()
{
   currentTime =  FDateTime::Now().GetTimeOfDay().GetTotalMilliseconds();
}

void UAIAssistantFunctionLibrary::Timestamp()
{
    auto deltaTime = FDateTime::Now().GetTimeOfDay().GetTotalMilliseconds() - currentTime;
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT(" spent %f ms"), deltaTime)); 
}
