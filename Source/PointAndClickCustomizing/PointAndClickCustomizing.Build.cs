// Copyright 2025 Devhanghae All Rights Reserved.

using UnrealBuildTool;

public class PointAndClickCustomizing : ModuleRules
{
    public PointAndClickCustomizing(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
			}
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "CoreUObject", "Engine",
                "EnhancedInput", "OnlineSubsystem"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "InputCore",
                "OnlineSubsystem",
                "EnhancedInput",
                "InteractiveToolsFramework", 
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
			}
            );
    }
}
