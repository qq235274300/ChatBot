// Copyright Epic Games, Inc. All Rights Reserved.

#include "WindowsHandler.h"




void FWindowsHandlerModule::StartupModule()
{
	FWindowsApplication* Application = GetApplication();
	// -----------------------托盘-------------------------
	Handler.CreateIco();
	if (Application != nullptr)
	{
		Application->AddMessageHandler(Handler);
	}
	
}

void FWindowsHandlerModule::ShutdownModule()
{
	FWindowsApplication* Application = GetApplication();
	// -----------------------移除托盘-------------------------
	Handler.DeleteIco();
	if (Application != nullptr)
	{
		Application->RemoveMessageHandler(Handler);
	}
}

FWindowsApplication* FWindowsHandlerModule::GetApplication() const
{
	if (!FSlateApplication::IsInitialized())
	{
		return nullptr;
	}

	return (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWindowsHandlerModule, WindowsHandler) 