// Copyright 2019 Ilgar Lunin. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class GoogleSpeechKit : ModuleRules
{

    public GoogleSpeechKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "Public"),
            }
		);

        PrivateIncludePaths.AddRange(
			new string[] {

            }
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "Voice",
                "Json",
                "XmlParser",
                "HTTP"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "Voice"
			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
    }
}
