// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MassAITesting : ModuleRules
{
	public MassAITesting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay",
			"MassEntity", "MassCommon", "MassNavigation", "StructUtils", "MassMovement", "NavigationSystem",
			"AIModule", "MassAIBehavior", "StateTreeModule", "GameplayTags", "SmartObjectsModule", "MassSmartObjects", "MassSignals",
			"MassRepresentation", "MassLOD", "AnimToTexture"
		});
	}
}
