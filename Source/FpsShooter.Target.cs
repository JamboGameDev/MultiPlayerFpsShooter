// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FpsShooterTarget : TargetRules
{
	public FpsShooterTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("FpsShooter");
	}
}
