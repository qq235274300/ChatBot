// Fill out your copyright notice in the Description page of Project Settings.


#include "AIGameInstance.h"

//#include "Windows/AllowWindowsPlatformTypes.h"
//#include "Windows/PreWindowsApi.h"
//#include <windows.h>
//#include "Windows/PostWindowsApi.h"
//#include "Windows/HideWindowsPlatformTypes.h"


#       include <windows/WindowsWindow.h>
#include "Windows/AllowWindowsPlatformTypes.h"




void UAIGameInstance::Init()
{
    
    FDisplayMetrics DisplayMetrics;
    FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);

     ScreenWidth = DisplayMetrics.PrimaryDisplayWidth;
     ScreenHeight = DisplayMetrics.PrimaryDisplayHeight;

    //SetWindowPosition(FVector2D(400,370));
    
    Super::Init();
}

void UAIGameInstance::SetWindowPosition(FVector2D WindowSize)
{
  
    int32 WindowPosX = ScreenWidth - WindowSize.X; // 窗口左上角 x 坐标
    int32 WindowPosY = ScreenHeight - WindowSize.Y;

    if (GEngine && GEngine->GameViewport && GEngine->GameViewport->GetWindow().IsValid())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "GEngineWork");
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("ScreenWidth: %d"), ScreenWidth));
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("ScreenHeight: %d"), ScreenHeight));
       
        TSharedPtr<SWindow> MainWindow = GEngine->GameViewport->GetWindow();
        MainWindow->MoveWindowTo(FVector2D(WindowPosX, WindowPosY));

        //// 获取窗口句柄
        //HWND WindowHandle = static_cast<HWND>(MainWindow->GetNativeWindow()->GetOSWindowHandle());

        //// 设置窗口为无边框，并总在最顶层
        //LONG_PTR Style = GetWindowLongPtr(WindowHandle, GWL_STYLE);
        //Style &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
        //Style |= WS_POPUP;
        //SetWindowLongPtr(WindowHandle, GWL_STYLE, Style);
        //SetWindowPos(WindowHandle, HWND_TOPMOST, WindowPosX, WindowPosY, WindowSize.X, WindowSize.Y, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);

   
    }
    
  
}



