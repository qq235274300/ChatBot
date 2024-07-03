// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIAssistantGameMode.h"
#include "AIAssistantCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAIAssistantGameMode::AAIAssistantGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
