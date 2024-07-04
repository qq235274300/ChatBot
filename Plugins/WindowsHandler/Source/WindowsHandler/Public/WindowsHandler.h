// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#include "Framework/Application/SlateApplication.h"
#include "Misc/OutputDevice.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Windows/WindowsApplication.h"

#define LOCTEXT_NAMESPACE "FWindowsHandlerModule"
#if PLATFORM_WINDOWS
#include <activation.h>
#include <shellapi.h>
#include "Windows/MinWindows.h"
#endif

#include "WindowsMessageHelper.h"

#include "Modules/ModuleManager.h"


NOTIFYICONDATA nid;

#define NOTIFY_SHOW WM_USER+2500
#define IDR_MAINFRAME WM_USER +2501


class FExampleHandler
	: public IWindowsMessageHandler
{
public:
	bool CreateIco()
	{
		HWND hWnd = GetActiveWindow();
		nid.hWnd = hWnd;

		// 图标路径是package之后,主目录
		const FString pathLaunch = FPaths::LaunchDir() + "icon.ico";
		UE_LOG(LogTemp, Warning, TEXT(">>> FPATHS : %s"), *pathLaunch);
		HINSTANCE hInst = NULL;
		HICON hIcon = (HICON)LoadImage(hInst, *(pathLaunch), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION | LR_LOADFROMFILE);

		const uint32 Error = GetLastError();
		GLog->Logf(TEXT("%d"), Error);


		nid.cbSize = sizeof(NOTIFYICONDATA); 
		nid.uID = IDR_MAINFRAME;
		nid.hIcon = hIcon;
		nid.hWnd = hWnd;
		nid.uCallbackMessage = NOTIFY_SHOW;
		nid.uVersion = NOTIFYICON_VERSION;
		nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO;
		lstrcpy(nid.szTip, L"LOOP TIMER"); //szAppClassName

		UE_LOG(LogTemp, Warning, TEXT(">>> Shell_NotifyIcon : "));
	
		return Shell_NotifyIcon(NIM_ADD, &nid);
	}
	
	bool DeleteIco()
	{
		return Shell_NotifyIcon(NIM_DELETE, &nid);
	}
	
	// 添加菜单
	void AddTrayMenu(HWND Hwnd)
	{

		POINT pt;

		GetCursorPos(&pt);
		HMENU menu = CreatePopupMenu();

		// quit
		AppendMenu(menu, MF_STRING, WM_DESTROY, L"quit");

		::SetForegroundWindow(Hwnd);
		int32 MenuID = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, NULL, Hwnd, NULL);

		GLog->Logf(TEXT("%d"), MenuID);

		if (MenuID == WM_DESTROY)
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> WM_DESTROY : "));
			// 退出
			RequestEngineExit("exit");
		}

		DestroyMenu(menu);
	}


	//~ IWindowsMessageHandler interface

	virtual bool ProcessMessage(HWND Hwnd, uint32 Message, WPARAM WParam, LPARAM LParam, int32& OutResult) override
	{
		// log out some details for the received message
		GLog->Logf(TEXT("WindowsMessageHandlerExampleModule: hwnd = %i, msg = %s, wParam = %i, lParam = %i"), Hwnd, *GetMessageName(Message), WParam, LParam);

		switch (Message)
		{
		case NOTIFY_SHOW:
			{
				switch (LParam)
				{
				case WM_RBUTTONUP:
					AddTrayMenu(Hwnd);
					break;
								
				}
			}

			//...
		}
		
		// we did not handle this message, so make sure it gets passed on to other handlers
		return false;
	}
};



class FWindowsHandlerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	FWindowsApplication* GetApplication() const;
private:
	FExampleHandler Handler;
};
