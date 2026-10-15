#include "HolofyLibrary.h"
#include "HolofyLog.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture.h"

FHolofyStyleSettings UHolofyLibrary::GetStylePreset(EHolofyStyle Style)
{
	FHolofyStyleSettings S; // Defaults == Classic

	switch (Style)
	{
	case EHolofyStyle::Classic:
	case EHolofyStyle::Custom:
		break;

	case EHolofyStyle::Transmission:
		S.HoloColor = FLinearColor(0.2f, 0.75f, 1.0f);
		S.RimColor = FLinearColor(0.5f, 0.9f, 1.0f);
		S.Brightness = 2.5f;
		S.Opacity = 0.3f;
		S.ScanlineDensity = 90.f;
		S.ScanlineIntensity = 0.75f;
		S.BandIntensity = 0.5f;
		S.BandSpeed = 0.6f;
		S.SignalStrength = 0.75f;
		S.FlickerIntensity = 0.35f;
		S.FlickerSpeed = 18.f;
		S.NoiseIntensity = 0.15f;
		S.GlitchIntensity = 0.45f;
		S.GlitchFrequency = 6.f;
		S.GlitchSlices = 40.f;
		S.ChromaticAberration = 0.6f;
		S.GlitchVertexOffset = 4.f;
		break;

	case EHolofyStyle::Tactical:
		S.HoloColor = FLinearColor(1.0f, 0.45f, 0.05f);
		S.RimColor = FLinearColor(1.0f, 0.7f, 0.3f);
		S.Brightness = 3.f;
		S.Opacity = 0.25f;
		S.ScanlineDensity = 30.f;
		S.ScanlineIntensity = 0.3f;
		S.BandIntensity = 1.5f;
		S.BandSpeed = 0.25f;
		S.BandWidth = 0.08f;
		S.FlickerIntensity = 0.05f;
		S.GlitchIntensity = 0.f;
		S.GlitchVertexOffset = 0.f;
		S.GridIntensity = 0.8f;
		S.GridDensity = 10.f;
		S.GridThickness = 0.05f;
		break;

	case EHolofyStyle::Ghost:
		S.HoloColor = FLinearColor(0.6f, 1.0f, 0.85f);
		S.RimColor = FLinearColor(0.85f, 1.0f, 0.95f);
		S.Brightness = 1.5f;
		S.Opacity = 0.12f;
		S.RimPower = 1.5f;
		S.RimIntensity = 2.5f;
		S.ScanlineDensity = 20.f;
		S.ScanlineSpeed = 0.02f;
		S.ScanlineIntensity = 0.15f;
		S.BandIntensity = 0.2f;
		S.BandSpeed = 0.1f;
		S.BandWidth = 0.4f;
		S.FlickerIntensity = 0.25f;
		S.FlickerSpeed = 2.f; // slow "breathing"
		S.NoiseIntensity = 0.02f;
		S.GlitchIntensity = 0.f;
		S.GlitchVertexOffset = 0.f;
		break;

	case EHolofyStyle::Glitch:
		S.HoloColor = FLinearColor(1.0f, 0.15f, 0.7f);
		S.RimColor = FLinearColor(0.1f, 0.9f, 1.0f);
		S.Brightness = 3.5f;
		S.Opacity = 0.4f;
		S.ScanlineDensity = 120.f;
		S.ScanlineIntensity = 0.5f;
		S.SignalStrength = 0.6f;
		S.FlickerIntensity = 0.4f;
		S.FlickerSpeed = 25.f;
		S.NoiseIntensity = 0.1f;
		S.GlitchIntensity = 0.9f;
		S.GlitchFrequency = 10.f;
		S.GlitchSlices = 60.f;
		S.ChromaticAberration = 1.5f;
		S.GlitchVertexOffset = 8.f;
		break;

	case EHolofyStyle::HardLight:
		S.HoloColor = FLinearColor(0.3f, 0.8f, 1.0f);
		S.RimColor = FLinearColor(0.8f, 0.95f, 1.0f);
		S.Brightness = 2.f;
		S.Opacity = 0.85f;
		S.RimIntensity = 1.f;
		S.ScanlineIntensity = 0.1f;
		S.BandIntensity = 0.4f;
		S.FlickerIntensity = 0.03f;
		S.GlitchIntensity = 0.f;
		S.GlitchVertexOffset = 0.f;
		break;

	case EHolofyStyle::HoloScreen:
		S.CoordinateSpace = EHolofySpace::UV;
		S.HoloColor = FLinearColor(0.15f, 0.65f, 1.0f);
		S.RimColor = FLinearColor(0.15f, 0.65f, 1.0f);
		S.Brightness = 2.5f;
		S.Opacity = 0.75f;
		S.RimIntensity = 0.3f;
		S.ScanlineDensity = 300.f;
		S.ScanlineSpeed = 0.02f;
		S.ScanlineIntensity = 0.25f;
		S.BandIntensity = 0.35f;
		S.BandSpeed = 0.15f;
		S.BandWidth = 0.1f;
		S.FlickerIntensity = 0.05f;
		S.NoiseIntensity = 0.02f;
		S.GlitchIntensity = 0.05f;
		S.GlitchVertexOffset = 0.f;
		S.GridIntensity = 0.1f;
		S.GridDensity = 12.f;
		S.GridThickness = 0.02f;
		S.ContentIntensity = 1.f;
		S.ContentTintAmount = 0.35f;
		break;

	case EHolofyStyle::CRTScreen:
		S.CoordinateSpace = EHolofySpace::UV;
		S.HoloColor = FLinearColor(0.8f, 0.9f, 1.0f);
		S.RimColor = FLinearColor::Black;
		S.Brightness = 1.4f;
		S.Opacity = 1.f;
		S.RimIntensity = 0.f;
		S.ScanlineDensity = 240.f;
		S.ScanlineSpeed = 0.f;
		S.ScanlineIntensity = 0.45f;
		S.ScanlineSharpness = 1.5f;
		S.BandIntensity = 0.15f;
		S.BandSpeed = 0.08f;
		S.BandWidth = 0.3f;
		S.FlickerIntensity = 0.04f;
		S.FlickerSpeed = 30.f;
		S.NoiseIntensity = 0.04f;
		S.GlitchIntensity = 0.f;
		S.ChromaticAberration = 0.25f;
		S.GlitchVertexOffset = 0.f;
		S.Curvature = 0.25f;
		S.Vignette = 0.6f;
		S.PixelGridIntensity = 0.5f;
		S.PixelGridDensity = 320.f;
		S.ContentIntensity = 1.f;
		S.ContentTintAmount = 0.f;
		break;

	case EHolofyStyle::Terminal:
		S.CoordinateSpace = EHolofySpace::UV;
		S.HoloColor = FLinearColor(0.2f, 1.0f, 0.35f);
		S.RimColor = FLinearColor::Black;
		S.Brightness = 2.f;
		S.Opacity = 1.f;
		S.RimIntensity = 0.f;
		S.ScanlineDensity = 200.f;
		S.ScanlineSpeed = 0.f;
		S.ScanlineIntensity = 0.5f;
		S.BandIntensity = 0.2f;
		S.BandSpeed = 0.1f;
		S.BandWidth = 0.25f;
		S.FlickerIntensity = 0.05f;
		S.FlickerSpeed = 30.f;
		S.NoiseIntensity = 0.03f;
		S.GlitchIntensity = 0.f;
		S.GlitchVertexOffset = 0.f;
		S.Curvature = 0.15f;
		S.Vignette = 0.5f;
		S.PixelGridIntensity = 0.2f;
		S.ContentIntensity = 1.f;
		S.ContentTintAmount = 1.f;
		break;

	case EHolofyStyle::Interference:
		S.CoordinateSpace = EHolofySpace::UV;
		S.HoloColor = FLinearColor(0.6f, 0.8f, 1.0f);
		S.RimColor = FLinearColor(0.6f, 0.8f, 1.0f);
		S.Brightness = 1.8f;
		S.Opacity = 0.9f;
		S.RimIntensity = 0.f;
		S.ScanlineDensity = 200.f;
		S.ScanlineIntensity = 0.5f;
		S.BandIntensity = 0.3f;
		S.SignalStrength = 0.35f;
		S.FlickerIntensity = 0.3f;
		S.FlickerSpeed = 20.f;
		S.NoiseIntensity = 0.35f;
		S.GlitchIntensity = 0.7f;
		S.GlitchFrequency = 8.f;
		S.GlitchSlices = 30.f;
		S.ChromaticAberration = 1.2f;
		S.GlitchVertexOffset = 0.f;
		S.ContentIntensity = 1.f;
		S.ContentTintAmount = 0.2f;
		break;
	}

	return S;
}

EHolofyBlendMode UHolofyLibrary::GetRecommendedBlendMode(EHolofyStyle Style)
{
	switch (Style)
	{
	case EHolofyStyle::HardLight:
	case EHolofyStyle::HoloScreen:
	case EHolofyStyle::Interference:
		return EHolofyBlendMode::Translucent;

	case EHolofyStyle::CRTScreen:
	case EHolofyStyle::Terminal:
		return EHolofyBlendMode::Masked;

	default:
		return EHolofyBlendMode::Additive;
	}
}

FHolofyStyleSettings UHolofyLibrary::LerpStyleSettings(const FHolofyStyleSettings& A, const FHolofyStyleSettings& B, float Alpha)
{
	return FHolofyStyleSettings::Lerp(A, B, Alpha);
}

void UHolofyLibrary::ApplySettingsToMaterial(UMaterialInstanceDynamic* Material, const FHolofyStyleSettings& Settings)
{
	if (!Material)
	{
		return;
	}

	Settings.ForEachScalarParameter([Material](const FHolofyParamInfo& Info, float Value)
	{
		Material->SetScalarParameterValue(Info.Name, Value);
	});

	Settings.ForEachVectorParameter([Material](const FHolofyParamInfo& Info, const FLinearColor& Value)
	{
		Material->SetVectorParameterValue(Info.Name, Value);
	});

	if (Settings.ContentTexture)
	{
		Material->SetTextureParameterValue(FHolofyStyleSettings::ContentTextureParam, Settings.ContentTexture);
	}
}

UMaterialInterface* UHolofyLibrary::GetBaseMaterial(EHolofyBlendMode BlendMode)
{
	return Cast<UMaterialInterface>(GetBaseMaterialPath(BlendMode).TryLoad());
}

UMaterialInterface* UHolofyLibrary::GetPresetMaterial(EHolofyStyle Style)
{
	return Cast<UMaterialInterface>(GetPresetMaterialPath(Style).TryLoad());
}

UMaterialInstanceDynamic* UHolofyLibrary::CreateHologramMaterial(UObject* Outer, EHolofyStyle Style, EHolofyBlendMode BlendMode)
{
	UMaterialInterface* Base = GetBaseMaterial(ResolveBlendMode(BlendMode, Style));
	if (!Base)
	{
		UE_LOG(LogHolofy, Warning, TEXT("Holofy master materials not found. Run Tools > Holofy > Generate Hologram Materials in the editor."));
		return nullptr;
	}

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Outer);
	ApplySettingsToMaterial(MID, GetStylePreset(Style));
	return MID;
}

EHolofyBlendMode UHolofyLibrary::ResolveBlendMode(EHolofyBlendMode BlendMode, EHolofyStyle Style)
{
	return BlendMode == EHolofyBlendMode::Auto ? GetRecommendedBlendMode(Style) : BlendMode;
}

FString UHolofyLibrary::GetBaseMaterialName(EHolofyBlendMode BlendMode)
{
	switch (BlendMode)
	{
	case EHolofyBlendMode::Translucent:	return TEXT("M_Holofy_Translucent");
	case EHolofyBlendMode::Masked:		return TEXT("M_Holofy_Masked");
	default:							return TEXT("M_Holofy_Additive");
	}
}

FString UHolofyLibrary::GetPresetMaterialName(EHolofyStyle Style)
{
	const UEnum* Enum = StaticEnum<EHolofyStyle>();
	return FString::Printf(TEXT("MI_Holofy_%s"), *Enum->GetNameStringByValue(static_cast<int64>(Style)));
}

FSoftObjectPath UHolofyLibrary::GetBaseMaterialPath(EHolofyBlendMode BlendMode)
{
	const FString Name = GetBaseMaterialName(BlendMode);
	return FSoftObjectPath(FString::Printf(TEXT("%s/%s.%s"), MaterialsPath, *Name, *Name));
}

FSoftObjectPath UHolofyLibrary::GetPresetMaterialPath(EHolofyStyle Style)
{
	const FString Name = GetPresetMaterialName(Style == EHolofyStyle::Custom ? EHolofyStyle::Classic : Style);
	return FSoftObjectPath(FString::Printf(TEXT("%s/%s.%s"), PresetsPath, *Name, *Name));
}
