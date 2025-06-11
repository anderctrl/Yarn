// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YarnEditor : ModuleRules
{
	public YarnEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"YarnEditor"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore", 
				"InputCore",
				"UnrealEd",
				"GraphEditor",
				"EditorStyle",
				"AssetTools",
				"ContentBrowser",
				"PropertyEditor",
				"DetailCustomizations",
				"Yarn",
				"GraphEditor",
				"Projects"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
