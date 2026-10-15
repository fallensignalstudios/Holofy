using UnrealBuildTool;

public class HolofyShaders : ModuleRules
{
	public HolofyShaders(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"Projects",
			"RenderCore",
		});
	}
}
