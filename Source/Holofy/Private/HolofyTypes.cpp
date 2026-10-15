#include "HolofyTypes.h"
#include "Engine/Texture.h"

const FName FHolofyStyleSettings::ContentTextureParam(TEXT("ContentTexture"));

#define HOLOFY_FLOAT(Field, Group, Min, Max) \
	{ \
		static const FHolofyParamInfo Info{ TEXT(#Field), FName(TEXT(#Field)), FName(TEXT(Group)), Min, Max }; \
		Fn(Info, Field); \
	}

void FHolofyStyleSettings::ForEachFloat(TFunctionRef<void(const FHolofyParamInfo&, float&)> Fn)
{
	HOLOFY_FLOAT(Brightness,				"Color",		0.f,	20.f);
	HOLOFY_FLOAT(Opacity,					"Color",		0.f,	1.f);
	HOLOFY_FLOAT(RimPower,					"Color",		0.1f,	10.f);
	HOLOFY_FLOAT(RimIntensity,				"Color",		0.f,	10.f);
	HOLOFY_FLOAT(VertexColorAmount,			"Color",		0.f,	1.f);

	HOLOFY_FLOAT(ScanlineDensity,			"Scanlines",	0.f,	500.f);
	HOLOFY_FLOAT(ScanlineSpeed,				"Scanlines",	-2.f,	2.f);
	HOLOFY_FLOAT(ScanlineIntensity,			"Scanlines",	0.f,	1.f);
	HOLOFY_FLOAT(ScanlineSharpness,			"Scanlines",	0.1f,	8.f);
	HOLOFY_FLOAT(BandIntensity,				"Scanlines",	0.f,	5.f);
	HOLOFY_FLOAT(BandSpeed,					"Scanlines",	-2.f,	2.f);
	HOLOFY_FLOAT(BandWidth,					"Scanlines",	0.01f,	1.f);

	HOLOFY_FLOAT(SignalStrength,			"Signal",		0.f,	1.f);
	HOLOFY_FLOAT(FlickerIntensity,			"Signal",		0.f,	1.f);
	HOLOFY_FLOAT(FlickerSpeed,				"Signal",		0.f,	60.f);
	HOLOFY_FLOAT(NoiseIntensity,			"Signal",		0.f,	1.f);
	HOLOFY_FLOAT(NoiseScale,				"Signal",		1.f,	1000.f);

	HOLOFY_FLOAT(GlitchIntensity,			"Glitch",		0.f,	1.f);
	HOLOFY_FLOAT(GlitchFrequency,			"Glitch",		0.f,	30.f);
	HOLOFY_FLOAT(GlitchSlices,				"Glitch",		1.f,	200.f);
	HOLOFY_FLOAT(ChromaticAberration,		"Glitch",		0.f,	5.f);
	HOLOFY_FLOAT(GlitchVertexOffset,		"Glitch",		0.f,	50.f);

	HOLOFY_FLOAT(GridIntensity,				"Grid",			0.f,	2.f);
	HOLOFY_FLOAT(GridDensity,				"Grid",			0.1f,	100.f);
	HOLOFY_FLOAT(GridThickness,				"Grid",			0.001f,	0.5f);

	HOLOFY_FLOAT(Curvature,					"Screen",		0.f,	1.f);
	HOLOFY_FLOAT(Vignette,					"Screen",		0.f,	1.f);
	HOLOFY_FLOAT(PixelGridIntensity,		"Screen",		0.f,	1.f);
	HOLOFY_FLOAT(PixelGridDensity,			"Screen",		1.f,	2048.f);

	HOLOFY_FLOAT(ContentIntensity,			"Content",		0.f,	1.f);
	HOLOFY_FLOAT(ContentTintAmount,			"Content",		0.f,	1.f);
	HOLOFY_FLOAT(ContentPixelate,			"Content",		0.f,	1024.f);

	HOLOFY_FLOAT(Materialize,				"Materialize",	0.f,	1.f);
	HOLOFY_FLOAT(MaterializeEdgeWidth,		"Materialize",	0.001f,	0.5f);
	HOLOFY_FLOAT(MaterializeEdgeBrightness,	"Materialize",	0.f,	20.f);
}

#undef HOLOFY_FLOAT

void FHolofyStyleSettings::ForEachScalarParameter(TFunctionRef<void(const FHolofyParamInfo&, float)> Fn) const
{
	const_cast<FHolofyStyleSettings*>(this)->ForEachFloat([&Fn](const FHolofyParamInfo& Info, float& Value)
	{
		Fn(Info, Value);
	});

	static const FHolofyParamInfo SpaceInfo{ TEXT("CoordinateSpace"), FName(TEXT("CoordinateSpace")), FName(TEXT("Space")), 0.f, 2.f };
	static const FHolofyParamInfo RotationInfo{ TEXT("UVRotation"), FName(TEXT("UVRotation")), FName(TEXT("Space")), 0.f, 3.f };
	static const FHolofyParamInfo FlipInfo{ TEXT("FlipU"), FName(TEXT("FlipU")), FName(TEXT("Space")), 0.f, 1.f };

	Fn(SpaceInfo, static_cast<float>(static_cast<uint8>(CoordinateSpace)));
	Fn(RotationInfo, static_cast<float>(UVRotation & 3));
	Fn(FlipInfo, bFlipU ? 1.f : 0.f);
}

void FHolofyStyleSettings::ForEachVectorParameter(TFunctionRef<void(const FHolofyParamInfo&, const FLinearColor&)> Fn) const
{
	static const FHolofyParamInfo HoloColorInfo{ TEXT("HoloColor"), FName(TEXT("HoloColor")), FName(TEXT("Color")), 0.f, 1.f };
	static const FHolofyParamInfo RimColorInfo{ TEXT("RimColor"), FName(TEXT("RimColor")), FName(TEXT("Color")), 0.f, 1.f };

	Fn(HoloColorInfo, HoloColor);
	Fn(RimColorInfo, RimColor);
}

FHolofyStyleSettings FHolofyStyleSettings::Lerp(const FHolofyStyleSettings& A, const FHolofyStyleSettings& B, float Alpha)
{
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

	TArray<float, TInlineAllocator<64>> BValues;
	const_cast<FHolofyStyleSettings&>(B).ForEachFloat([&BValues](const FHolofyParamInfo&, float& Value)
	{
		BValues.Add(Value);
	});

	FHolofyStyleSettings Result = A;
	int32 Index = 0;
	Result.ForEachFloat([&](const FHolofyParamInfo&, float& Value)
	{
		Value = FMath::Lerp(Value, BValues[Index++], Alpha);
	});

	Result.HoloColor = FMath::Lerp(A.HoloColor, B.HoloColor, Alpha);
	Result.RimColor = FMath::Lerp(A.RimColor, B.RimColor, Alpha);

	const bool bUseB = Alpha >= 0.5f;
	Result.CoordinateSpace = bUseB ? B.CoordinateSpace : A.CoordinateSpace;
	Result.UVRotation = bUseB ? B.UVRotation : A.UVRotation;
	Result.bFlipU = bUseB ? B.bFlipU : A.bFlipU;
	if (bUseB && B.ContentTexture)
	{
		Result.ContentTexture = B.ContentTexture;
	}

	return Result;
}
