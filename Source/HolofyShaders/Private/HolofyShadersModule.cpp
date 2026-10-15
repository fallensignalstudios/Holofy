#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

/**
 * Maps /Plugin/Holofy -> <Plugin>/Shaders so material Custom nodes can include Holofy.ush.
 * Loads at PostConfigInit so the mapping exists before any material compiles.
 */
class FHolofyShadersModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Holofy"));
		if (!Plugin.IsValid())
		{
			return;
		}

		const FString VirtualDir = TEXT("/Plugin/Holofy");
		const FString RealDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));

		// The engine may already auto-map plugin shader folders; adding twice asserts.
		if (!AllShaderSourceDirectoryMappings().Contains(VirtualDir))
		{
			AddShaderSourceDirectoryMapping(VirtualDir, RealDir);
		}
	}
};

IMPLEMENT_MODULE(FHolofyShadersModule, HolofyShaders)
