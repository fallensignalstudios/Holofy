using UnrealBuildTool;

public class HolofyEditor : ModuleRules
{
	public HolofyEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"MaterialEditor",
			"AssetTools",
			"ToolMenus",
			"Slate",
			"SlateCore",
			"Holofy",
		});
	}
}
