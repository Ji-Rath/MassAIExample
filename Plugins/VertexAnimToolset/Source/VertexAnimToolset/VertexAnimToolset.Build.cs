// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VertexAnimToolset : ModuleRules
{
	public VertexAnimToolset(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


        PrivateIncludePaths.AddRange(
                new string[] {
					// ... add other private include paths required here ...
				}
                );



        PublicDependencyModuleNames.AddRange(
			new string[]
			{

              "Core",
                    "CoreUObject",
                    "Engine",
                    "RHI",
                    "Engine",
                    "RenderCore",
					// ... add other public dependencies that you statically link with here ...
			}
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Projects",
					// ... add private dependencies that you statically link with here ...
			}
            );
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		PrecompileForTargets = PrecompileTargetsType.Any;
	}
}
