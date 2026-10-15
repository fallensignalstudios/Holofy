#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HolofyTypes.h"
#include "HolofyLibrary.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class HOLOFY_API UHolofyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Full settings for one of the built-in looks. Custom returns the defaults (Classic). */
	UFUNCTION(BlueprintPure, Category = "Holofy")
	static FHolofyStyleSettings GetStylePreset(EHolofyStyle Style);

	/** The blend mode a style was designed for. */
	UFUNCTION(BlueprintPure, Category = "Holofy")
	static EHolofyBlendMode GetRecommendedBlendMode(EHolofyStyle Style);

	/** Morph between two looks. */
	UFUNCTION(BlueprintPure, Category = "Holofy")
	static FHolofyStyleSettings LerpStyleSettings(const FHolofyStyleSettings& A, const FHolofyStyleSettings& B, float Alpha);

	/** Push every setting onto a dynamic instance of a Holofy master material. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	static void ApplySettingsToMaterial(UMaterialInstanceDynamic* Material, const FHolofyStyleSettings& Settings);

	/** Loads the generated master material for a blend mode (Auto = Additive). Null if not generated yet. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	static UMaterialInterface* GetBaseMaterial(EHolofyBlendMode BlendMode);

	/** Loads the generated preset instance (MI_Holofy_<Style>). Null if not generated yet. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	static UMaterialInterface* GetPresetMaterial(EHolofyStyle Style);

	/** One-liner: dynamic material for a style, ready to slap on any mesh / Niagara renderer. */
	UFUNCTION(BlueprintCallable, Category = "Holofy", meta = (DefaultToSelf = "Outer"))
	static UMaterialInstanceDynamic* CreateHologramMaterial(UObject* Outer, EHolofyStyle Style, EHolofyBlendMode BlendMode = EHolofyBlendMode::Auto);

	// ---- Asset naming (shared with the editor generator) ----

	static constexpr const TCHAR* MaterialsPath = TEXT("/Holofy/Materials");
	static constexpr const TCHAR* PresetsPath = TEXT("/Holofy/Materials/Presets");
	static constexpr const TCHAR* ShaderInclude = TEXT("/Plugin/Holofy/Private/Holofy.ush");

	static EHolofyBlendMode ResolveBlendMode(EHolofyBlendMode BlendMode, EHolofyStyle Style);
	static FString GetBaseMaterialName(EHolofyBlendMode BlendMode);
	static FString GetPresetMaterialName(EHolofyStyle Style);
	static FSoftObjectPath GetBaseMaterialPath(EHolofyBlendMode BlendMode);
	static FSoftObjectPath GetPresetMaterialPath(EHolofyStyle Style);
};
