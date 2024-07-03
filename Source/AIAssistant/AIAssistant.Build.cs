// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AIAssistant : ModuleRules
{
	public AIAssistant(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "Voice" , "Slate", "SlateCore", "ApplicationCore", });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}
