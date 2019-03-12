/**
* Copyright (c) 2018 Cronofear Softworks, Inc. All Rights Reserved.
*
* Developed by Kevin Yabar Garces
*/

using UnrealBuildTool;

public class CSWAutoSaveAndLoadSystem : ModuleRules
{
	public CSWAutoSaveAndLoadSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		//Override UnrealEngineBuildTool to speedup compilation times
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = true;

        //INCLUDE WHAT YOU NEED CONVENTION ACTIVATION
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;


//         PublicIncludePaths.AddRange(
// 			new string[] {
// 				"CSWAutoSaveAndLoadSystem/Public"
// 			}
// 			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"CSWAutoSaveAndLoadSystem/Private"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine"
				
				// ... add other public dependencies that you statically link with here ...
			}
			);
		
// 		PrivateDependencyModuleNames.AddRange(
// 			new string[]
// 			{
//             }
// 			);
	}
}
