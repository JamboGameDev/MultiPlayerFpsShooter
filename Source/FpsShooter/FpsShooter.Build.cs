// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FpsShooter : ModuleRules
{
	public FpsShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PublicDependencyModuleNames.AddRange(new string[] { "FpsShooter" });


	}
}
