#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HolofyTypes.h"
#include "HolofyComponent.generated.h"

class UMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTexture;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHolofySimpleEvent);

USTRUCT()
struct FHolofyMaterialBackup
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TWeakObjectPtr<UMeshComponent> Mesh;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> Materials;
};

/**
 * Turns the owning actor's meshes into a hologram / screen / transmission.
 *
 * Drop it on any actor, pick a Style, hit Play. At runtime you can morph between styles,
 * materialize / dematerialize, trigger glitches, and drive signal strength (great for
 * "incoming transmission" moments).
 */
UCLASS(ClassGroup = (Holofy), meta = (BlueprintSpawnableComponent, DisplayName = "Holofy Hologram"))
class HOLOFY_API UHolofyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHolofyComponent();

	// ---- Look ----

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Holofy")
	EHolofyStyle Style = EHolofyStyle::Classic;

	/** Used when Style is Custom. Tip: pick a preset, then click "Customize Current Style". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Holofy", meta = (EditCondition = "Style == EHolofyStyle::Custom", EditConditionHides))
	FHolofyStyleSettings CustomSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Holofy")
	EHolofyBlendMode BlendMode = EHolofyBlendMode::Auto;

	/** What to show on screens/displays. Overrides the style's Content Texture. Render targets & media textures work great. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Holofy")
	TObjectPtr<UTexture> ContentTexture;

	// ---- Targets ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Targets")
	bool bApplyOnBeginPlay = true;

	/** Only affect mesh components with this tag. None = every mesh component on the owner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Targets")
	FName TargetComponentTag;

	/** Use your own material instead of the generated masters (must expose the same parameters). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Holofy|Advanced")
	TObjectPtr<UMaterialInterface> BaseMaterialOverride;

	// ---- Materialize ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Materialize")
	bool bMaterializeOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Materialize", meta = (ClampMin = "0"))
	float MaterializeDuration = 1.5f;

	// ---- Auto glitch ----

	/** Randomly fire glitch bursts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Glitch")
	bool bAutoGlitch = false;

	/** Seconds between bursts (min, max). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Glitch", meta = (EditCondition = "bAutoGlitch"))
	FVector2D AutoGlitchInterval = FVector2D(2.0, 6.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Glitch", meta = (EditCondition = "bAutoGlitch", ClampMin = "0"))
	float AutoGlitchDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Glitch", meta = (EditCondition = "bAutoGlitch", ClampMin = "0", ClampMax = "1"))
	float AutoGlitchIntensity = 0.8f;

	// ---- Events ----

	UPROPERTY(BlueprintAssignable, Category = "Holofy")
	FHolofySimpleEvent OnMaterialized;

	UPROPERTY(BlueprintAssignable, Category = "Holofy")
	FHolofySimpleEvent OnDematerialized;

	// ---- API ----

	/** Swap the target meshes' materials for the hologram. Safe to call repeatedly. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void ApplyHologram();

	/** Restore the meshes' original materials. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void RemoveHologram();

	UFUNCTION(BlueprintPure, Category = "Holofy")
	bool IsHologramApplied() const { return bApplied; }

	/** Switch look. BlendTime > 0 morphs smoothly (blend mode switches instantly). */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void SetStyle(EHolofyStyle NewStyle, float BlendTime = 0.f);

	/** Switch to fully custom settings, optionally morphing. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void SetCustomSettings(const FHolofyStyleSettings& NewSettings, float BlendTime = 0.f);

	/** The settings currently on screen (mid-blend values included, runtime effects excluded). */
	UFUNCTION(BlueprintPure, Category = "Holofy")
	FHolofyStyleSettings GetCurrentSettings() const { return CurrentSettings; }

	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void SetContentTexture(UTexture* NewContent);

	/** 0..1 multiplier on signal quality. Lower = static, dropouts, tearing. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void SetSignalStrength(float NewSignalStrength);

	UFUNCTION(BlueprintPure, Category = "Holofy")
	float GetSignalStrength() const { return RuntimeSignalStrength; }

	/** Build the hologram in, line by line. Applies the hologram first if needed. Duration <= 0 uses MaterializeDuration. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void Materialize(float Duration = -1.f);

	/** Break the hologram apart until invisible. Duration <= 0 uses MaterializeDuration. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void Dematerialize(float Duration = -1.f);

	/** Force a glitch burst. */
	UFUNCTION(BlueprintCallable, Category = "Holofy")
	void TriggerGlitch(float Duration = 0.25f, float Intensity = 0.8f);

	/** The live material on the meshes (null until applied). Set extra params on it if you like. */
	UFUNCTION(BlueprintPure, Category = "Holofy")
	UMaterialInstanceDynamic* GetDynamicMaterial() const { return DynamicMaterial; }

	/** Copies the current preset into Custom Settings and switches Style to Custom so you can tweak it. */
	UFUNCTION(CallInEditor, Category = "Holofy")
	void CustomizeCurrentStyle();

	/** Settings for the current Style (preset or CustomSettings). */
	FHolofyStyleSettings ResolveStyleSettings() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UMaterialInterface* ResolveBaseMaterial() const;
	void GatherTargetMeshes(TArray<UMeshComponent*>& OutMeshes) const;
	void AssignMaterialToMeshes();
	void StartTransition(const FHolofyStyleSettings& Target, float BlendTime);
	void PushParameters();
	void ScheduleNextAutoGlitch();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	UPROPERTY(Transient)
	TArray<FHolofyMaterialBackup> Backups;

	UPROPERTY(Transient)
	FHolofyStyleSettings CurrentSettings;

	UPROPERTY(Transient)
	FHolofyStyleSettings TransitionFrom;

	UPROPERTY(Transient)
	FHolofyStyleSettings TransitionTo;

	float TransitionElapsed = 0.f;
	float TransitionDuration = 0.f;

	float RuntimeMaterialize = 1.f;
	float MaterializeRate = 0.f; // +/- per second, 0 = idle

	float RuntimeSignalStrength = 1.f;

	float GlitchBoost = 0.f;
	float GlitchTimeRemaining = 0.f;
	float NextAutoGlitchTime = 0.f;

	bool bApplied = false;
	bool bDirty = false;
};
