#include "HolofyComponent.h"
#include "HolofyLibrary.h"
#include "HolofyLog.h"
#include "Components/MeshComponent.h"
#include "Engine/Texture.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

UHolofyComponent::UHolofyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UHolofyComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentSettings = ResolveStyleSettings();
	ScheduleNextAutoGlitch();

	if (bMaterializeOnBeginPlay)
	{
		RuntimeMaterialize = 0.f;
		Materialize(MaterializeDuration);
	}
	else if (bApplyOnBeginPlay)
	{
		ApplyHologram();
	}
}

FHolofyStyleSettings UHolofyComponent::ResolveStyleSettings() const
{
	return Style == EHolofyStyle::Custom ? CustomSettings : UHolofyLibrary::GetStylePreset(Style);
}

UMaterialInterface* UHolofyComponent::ResolveBaseMaterial() const
{
	if (BaseMaterialOverride)
	{
		return BaseMaterialOverride;
	}
	return UHolofyLibrary::GetBaseMaterial(UHolofyLibrary::ResolveBlendMode(BlendMode, Style));
}

void UHolofyComponent::GatherTargetMeshes(TArray<UMeshComponent*>& OutMeshes) const
{
	OutMeshes.Reset();
	if (const AActor* Owner = GetOwner())
	{
		Owner->GetComponents<UMeshComponent>(OutMeshes);
		if (!TargetComponentTag.IsNone())
		{
			OutMeshes.RemoveAll([this](const UMeshComponent* Mesh)
			{
				return !Mesh->ComponentHasTag(TargetComponentTag);
			});
		}
	}
}

void UHolofyComponent::ApplyHologram()
{
	UMaterialInterface* Base = ResolveBaseMaterial();
	if (!Base)
	{
		UE_LOG(LogHolofy, Warning, TEXT("%s: Holofy master materials not found. Run Tools > Holofy > Generate Hologram Materials in the editor."), *GetPathName());
		return;
	}

	if (!DynamicMaterial || DynamicMaterial->Parent != Base)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(Base, this);
	}

	AssignMaterialToMeshes();
	bApplied = true;
	bDirty = true;
	PushParameters();
}

void UHolofyComponent::AssignMaterialToMeshes()
{
	TArray<UMeshComponent*> Meshes;
	GatherTargetMeshes(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		const bool bAlreadyBackedUp = Backups.ContainsByPredicate([Mesh](const FHolofyMaterialBackup& B)
		{
			return B.Mesh.Get() == Mesh;
		});

		if (!bAlreadyBackedUp)
		{
			FHolofyMaterialBackup& Backup = Backups.AddDefaulted_GetRef();
			Backup.Mesh = Mesh;
			for (UMaterialInterface* Mat : Mesh->GetMaterials())
			{
				Backup.Materials.Add(Mat);
			}
		}

		for (int32 Slot = 0; Slot < Mesh->GetNumMaterials(); ++Slot)
		{
			Mesh->SetMaterial(Slot, DynamicMaterial);
		}
	}
}

void UHolofyComponent::RemoveHologram()
{
	for (const FHolofyMaterialBackup& Backup : Backups)
	{
		if (UMeshComponent* Mesh = Backup.Mesh.Get())
		{
			for (int32 Slot = 0; Slot < Backup.Materials.Num(); ++Slot)
			{
				Mesh->SetMaterial(Slot, Backup.Materials[Slot]);
			}
		}
	}

	Backups.Reset();
	bApplied = false;
	RuntimeMaterialize = 1.f;
	MaterializeRate = 0.f;
}

void UHolofyComponent::SetStyle(EHolofyStyle NewStyle, float BlendTime)
{
	Style = NewStyle;

	// A style may want a different master (e.g. Additive -> Masked). That can't blend, so swap now.
	if (bApplied && DynamicMaterial && DynamicMaterial->Parent != ResolveBaseMaterial())
	{
		ApplyHologram();
	}

	StartTransition(ResolveStyleSettings(), BlendTime);
}

void UHolofyComponent::SetCustomSettings(const FHolofyStyleSettings& NewSettings, float BlendTime)
{
	CustomSettings = NewSettings;
	SetStyle(EHolofyStyle::Custom, BlendTime);
}

void UHolofyComponent::StartTransition(const FHolofyStyleSettings& Target, float BlendTime)
{
	if (BlendTime <= 0.f)
	{
		CurrentSettings = Target;
		TransitionDuration = 0.f;
	}
	else
	{
		TransitionFrom = CurrentSettings;
		TransitionTo = Target;
		TransitionElapsed = 0.f;
		TransitionDuration = BlendTime;
	}
	bDirty = true;
}

void UHolofyComponent::SetContentTexture(UTexture* NewContent)
{
	ContentTexture = NewContent;
	bDirty = true;
}

void UHolofyComponent::SetSignalStrength(float NewSignalStrength)
{
	RuntimeSignalStrength = FMath::Clamp(NewSignalStrength, 0.f, 1.f);
	bDirty = true;
}

void UHolofyComponent::Materialize(float Duration)
{
	if (!bApplied)
	{
		RuntimeMaterialize = 0.f;
		ApplyHologram();
	}
	else if (RuntimeMaterialize >= 1.f)
	{
		RuntimeMaterialize = 0.f; // replay from scratch
	}

	const float D = Duration > 0.f ? Duration : MaterializeDuration;
	if (D <= 0.f)
	{
		RuntimeMaterialize = 1.f;
		MaterializeRate = 0.f;
		OnMaterialized.Broadcast();
	}
	else
	{
		MaterializeRate = 1.f / D;
	}
	bDirty = true;
}

void UHolofyComponent::Dematerialize(float Duration)
{
	if (!bApplied)
	{
		ApplyHologram();
	}

	const float D = Duration > 0.f ? Duration : MaterializeDuration;
	if (D <= 0.f)
	{
		RuntimeMaterialize = 0.f;
		MaterializeRate = 0.f;
		OnDematerialized.Broadcast();
	}
	else
	{
		MaterializeRate = -1.f / D;
	}
	bDirty = true;
}

void UHolofyComponent::TriggerGlitch(float Duration, float Intensity)
{
	GlitchBoost = FMath::Clamp(Intensity, 0.f, 1.f);
	GlitchTimeRemaining = FMath::Max(Duration, 0.f);
	bDirty = true;
}

void UHolofyComponent::ScheduleNextAutoGlitch()
{
	const float Min = FMath::Max(0.05f, static_cast<float>(FMath::Min(AutoGlitchInterval.X, AutoGlitchInterval.Y)));
	const float Max = FMath::Max(Min, static_cast<float>(FMath::Max(AutoGlitchInterval.X, AutoGlitchInterval.Y)));
	NextAutoGlitchTime = FMath::FRandRange(Min, Max);
}

void UHolofyComponent::CustomizeCurrentStyle()
{
	if (Style == EHolofyStyle::Custom)
	{
		return;
	}

	Modify();
	CustomSettings = UHolofyLibrary::GetStylePreset(Style);
	Style = EHolofyStyle::Custom;
}

void UHolofyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bApplied)
	{
		return;
	}

	// Style morph
	if (TransitionDuration > 0.f)
	{
		TransitionElapsed += DeltaTime;
		const float Alpha = FMath::Clamp(TransitionElapsed / TransitionDuration, 0.f, 1.f);
		CurrentSettings = FHolofyStyleSettings::Lerp(TransitionFrom, TransitionTo, FMath::SmoothStep(0.f, 1.f, Alpha));
		if (Alpha >= 1.f)
		{
			TransitionDuration = 0.f;
		}
		bDirty = true;
	}

	// Materialize / dematerialize
	if (MaterializeRate != 0.f)
	{
		RuntimeMaterialize = FMath::Clamp(RuntimeMaterialize + MaterializeRate * DeltaTime, 0.f, 1.f);
		bDirty = true;

		if (MaterializeRate > 0.f && RuntimeMaterialize >= 1.f)
		{
			MaterializeRate = 0.f;
			OnMaterialized.Broadcast();
		}
		else if (MaterializeRate < 0.f && RuntimeMaterialize <= 0.f)
		{
			MaterializeRate = 0.f;
			OnDematerialized.Broadcast();
		}
	}

	// Glitch bursts
	if (GlitchTimeRemaining > 0.f)
	{
		GlitchTimeRemaining -= DeltaTime;
		if (GlitchTimeRemaining <= 0.f)
		{
			GlitchBoost = 0.f;
			bDirty = true;
		}
	}

	if (bAutoGlitch)
	{
		NextAutoGlitchTime -= DeltaTime;
		if (NextAutoGlitchTime <= 0.f)
		{
			TriggerGlitch(AutoGlitchDuration, AutoGlitchIntensity);
			ScheduleNextAutoGlitch();
		}
	}

	PushParameters();
}

void UHolofyComponent::PushParameters()
{
	if (!bDirty || !DynamicMaterial)
	{
		return;
	}
	bDirty = false;

	FHolofyStyleSettings Final = CurrentSettings;
	Final.Materialize *= RuntimeMaterialize;
	Final.SignalStrength *= RuntimeSignalStrength;
	Final.GlitchIntensity = FMath::Max(Final.GlitchIntensity, GlitchBoost);
	if (ContentTexture)
	{
		Final.ContentTexture = ContentTexture;
	}

	UHolofyLibrary::ApplySettingsToMaterial(DynamicMaterial, Final);
}

#if WITH_EDITOR
void UHolofyComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Live-tweak while playing in editor.
	if (bApplied)
	{
		if (DynamicMaterial && DynamicMaterial->Parent != ResolveBaseMaterial())
		{
			ApplyHologram();
		}
		StartTransition(ResolveStyleSettings(), 0.f);
		PushParameters();
	}
}
#endif
