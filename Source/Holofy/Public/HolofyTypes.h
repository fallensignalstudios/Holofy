#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "HolofyTypes.generated.h"

class UTexture;

/** Built-in looks. Each maps to a full FHolofyStyleSettings preset (see UHolofyLibrary::GetStylePreset). */
UENUM(BlueprintType)
enum class EHolofyStyle : uint8
{
	Classic			UMETA(DisplayName = "Classic Hologram", ToolTip = "Blue sci-fi hologram: scanlines, rim glow, rising sweep band."),
	Transmission	UMETA(DisplayName = "Transmission", ToolTip = "Incoming holo-call with a shaky signal: static, tearing, chromatic fringes."),
	Tactical		UMETA(DisplayName = "Tactical", ToolTip = "Amber war-table projection with a measurement grid and scan sweep."),
	Ghost			UMETA(DisplayName = "Ghost", ToolTip = "Faint, breathing, spectral. Mostly rim light."),
	Glitch			UMETA(DisplayName = "Glitch", ToolTip = "Heavily corrupted, neon, constantly tearing."),
	HardLight		UMETA(DisplayName = "Hard Light", ToolTip = "Near-solid light construct. Subtle scanlines, high opacity."),
	HoloScreen		UMETA(DisplayName = "Holo Screen", ToolTip = "Floating sci-fi UI panel. Shows Content Texture with a holo tint."),
	CRTScreen		UMETA(DisplayName = "CRT Screen", ToolTip = "Old monitor: curvature, RGB pixel grid, vignette, scanlines."),
	Terminal		UMETA(DisplayName = "Terminal", ToolTip = "Green phosphor monochrome terminal."),
	Interference	UMETA(DisplayName = "Interference", ToolTip = "Video feed that is barely holding on: heavy static and tearing."),
	Custom			UMETA(DisplayName = "Custom", ToolTip = "Use the Custom Settings on the component."),
};

UENUM(BlueprintType)
enum class EHolofyBlendMode : uint8
{
	/** Use the blend mode the style was designed for. */
	Auto,
	/** Classic glowing hologram. Never darkens what's behind it. */
	Additive,
	/** Can darken / occlude. Good for screens and hard light. */
	Translucent,
	/** Opaque screens. Cheapest, writes depth, works with everything. */
	Masked,
};

UENUM(BlueprintType)
enum class EHolofySpace : uint8
{
	/** Object space in meters. Best for 3D holograms (characters, props). Lines run along local Z. */
	Local,
	/** Mesh UVs. Best for screens and panels. Lines run along V. */
	UV,
	/** Screen space. Lines stay fixed on screen like a camera overlay. */
	Screen,
};

/** Metadata for a material parameter exposed by Holofy. */
struct FHolofyParamInfo
{
	/** Case-exact name. Matches the FHolofyParams field in Holofy.ush (HLSL is case-sensitive, FName isn't). */
	const TCHAR* Key = nullptr;
	FName Name;
	FName Group;
	float SliderMin = 0.f;
	float SliderMax = 1.f;
};

/**
 * Every knob the Holofy master materials expose. Blendable, so you can morph between looks.
 *
 * Adding a parameter: add the field here, register it in ForEachFloat (HolofyTypes.cpp),
 * and add a field with the same name to FHolofyParams in Shaders/Private/Holofy.ush.
 * Then re-run Tools > Holofy > Generate Hologram Materials.
 */
USTRUCT(BlueprintType)
struct HOLOFY_API FHolofyStyleSettings
{
	GENERATED_BODY()

	// ---- Color ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor HoloColor = FLinearColor(0.1f, 0.6f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor RimColor = FLinearColor(0.4f, 0.85f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0", UIMax = "20"))
	float Brightness = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0", ClampMax = "1"))
	float Opacity = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0.1", UIMax = "10"))
	float RimPower = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0", UIMax = "10"))
	float RimIntensity = 1.5f;

	/** How much mesh vertex color / particle color tints the result. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0", ClampMax = "1"))
	float VertexColorAmount = 0.0f;

	// ---- Scanlines ----

	/** Lines per meter (Local) or per full UV/screen height (UV/Screen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanlines", meta = (ClampMin = "0", UIMax = "500"))
	float ScanlineDensity = 60.0f;

	/** Scroll speed in meters/sec (Local) or screens/sec (UV/Screen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanlines", meta = (UIMin = "-2", UIMax = "2"))
	float ScanlineSpeed = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanlines", meta = (ClampMin = "0", ClampMax = "1"))
	float ScanlineIntensity = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanlines", meta = (ClampMin = "0.1", UIMax = "8"))
	float ScanlineSharpness = 2.0f;

	/** The big bright sweep that travels up the hologram. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanlines", meta = (ClampMin = "0", UIMax = "5"))
	float BandIntensity = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanlines", meta = (UIMin = "-2", UIMax = "2"))
	float BandSpeed = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanlines", meta = (ClampMin = "0.01", ClampMax = "1"))
	float BandWidth = 0.15f;

	// ---- Signal ----

	/** 1 = perfect signal. Lower values add static, dropouts, tearing and a rolling bar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal", meta = (ClampMin = "0", ClampMax = "1"))
	float SignalStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal", meta = (ClampMin = "0", ClampMax = "1"))
	float FlickerIntensity = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal", meta = (ClampMin = "0", UIMax = "60"))
	float FlickerSpeed = 12.0f;

	/** Static "snow". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal", meta = (ClampMin = "0", ClampMax = "1"))
	float NoiseIntensity = 0.03f;

	/** Static grain cells across the surface/screen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Signal", meta = (ClampMin = "1", UIMax = "1000"))
	float NoiseScale = 300.0f;

	// ---- Glitch ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch", meta = (ClampMin = "0", ClampMax = "1"))
	float GlitchIntensity = 0.1f;

	/** Glitch "rolls of the dice" per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch", meta = (ClampMin = "0", UIMax = "30"))
	float GlitchFrequency = 4.0f;

	/** Horizontal tear slices per meter (Local) or per screen (UV/Screen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch", meta = (ClampMin = "1", UIMax = "200"))
	float GlitchSlices = 24.0f;

	/** RGB split on the content texture. Gets worse during glitches and bad signal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch", meta = (ClampMin = "0", UIMax = "5"))
	float ChromaticAberration = 0.0f;

	/** How far (units) geometry slices get shoved sideways during a glitch. 0 for flat screens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glitch", meta = (ClampMin = "0", UIMax = "50"))
	float GlitchVertexOffset = 2.0f;

	// ---- Grid ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "0", UIMax = "2"))
	float GridIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "0.1", UIMax = "100"))
	float GridDensity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "0.001", ClampMax = "0.5"))
	float GridThickness = 0.04f;

	// ---- Screen ----

	/** CRT barrel distortion. Pixels outside the curved screen are cut. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen", meta = (ClampMin = "0", UIMax = "1"))
	float Curvature = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen", meta = (ClampMin = "0", ClampMax = "1"))
	float Vignette = 0.0f;

	/** RGB phosphor triads. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen", meta = (ClampMin = "0", ClampMax = "1"))
	float PixelGridIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen", meta = (ClampMin = "1", UIMax = "2048"))
	float PixelGridDensity = 320.0f;

	// ---- Content ----

	/** What the screen/hologram is showing: a texture, render target, media texture, scene capture... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TObjectPtr<UTexture> ContentTexture = nullptr;

	/** 0 = flat holo color, 1 = full content texture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (ClampMin = "0", ClampMax = "1"))
	float ContentIntensity = 0.0f;

	/** 0 = original content colors, 1 = monochrome in HoloColor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (ClampMin = "0", ClampMax = "1"))
	float ContentTintAmount = 0.6f;

	/** Pixels across the content. 0 = off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (ClampMin = "0", UIMax = "1024"))
	float ContentPixelate = 0.0f;

	// ---- Materialize ----

	/** 0 = invisible, 1 = fully built. Animate it with UHolofyComponent::Materialize. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materialize", meta = (ClampMin = "0", ClampMax = "1"))
	float Materialize = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materialize", meta = (ClampMin = "0.001", ClampMax = "0.5"))
	float MaterializeEdgeWidth = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materialize", meta = (ClampMin = "0", UIMax = "20"))
	float MaterializeEdgeBrightness = 6.0f;

	// ---- Space ----

	/** Which coordinates the effects run along. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
	EHolofySpace CoordinateSpace = EHolofySpace::Local;

	/** Rotate UVs in quarter turns (fixes sideways screen content). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space", meta = (ClampMin = "0", ClampMax = "3"))
	int32 UVRotation = 0;

	/** Mirror UVs horizontally (fixes backwards screen content). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
	bool bFlipU = false;

	/** Material parameter name for ContentTexture. */
	static const FName ContentTextureParam;

	/** Iterates every blendable float field by reference. */
	void ForEachFloat(TFunctionRef<void(const FHolofyParamInfo&, float&)> Fn);

	/** Every scalar material parameter (floats + Space settings) with its value. */
	void ForEachScalarParameter(TFunctionRef<void(const FHolofyParamInfo&, float)> Fn) const;

	/** Every vector material parameter with its value. */
	void ForEachVectorParameter(TFunctionRef<void(const FHolofyParamInfo&, const FLinearColor&)> Fn) const;

	/** Blends every numeric parameter. Discrete settings and the texture switch at the halfway point. */
	static FHolofyStyleSettings Lerp(const FHolofyStyleSettings& A, const FHolofyStyleSettings& B, float Alpha);
};
