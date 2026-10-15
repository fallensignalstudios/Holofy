#include "HolofyMaterialBuilder.h"

#include "HolofyLibrary.h"
#include "HolofyTypes.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Engine/Texture.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "FileHelpers.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionCameraVectorWS.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionLocalPosition.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionScreenPosition.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionTransform.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogHolofyEditor, Log, All);

namespace HolofyMaterialBuilder
{
namespace
{
	constexpr int32 VertexColorAlphaOutput = 4; // VertexColor pins: RGB, R, G, B, A

	template <typename T>
	T* AddExpression(UMaterial* Material, int32 X, int32 Y)
	{
		return Cast<T>(UMaterialEditingLibrary::CreateMaterialExpression(Material, T::StaticClass(), X, Y));
	}

	void AddCustomInput(UMaterialExpressionCustom* Custom, const FString& Name, UMaterialExpression* Source, int32 OutputIndex = 0)
	{
		FCustomInput& Input = Custom->Inputs.AddDefaulted_GetRef();
		Input.InputName = FName(*Name);
		Input.Input.Connect(OutputIndex, Source);
	}

	/** "FHolofyParams P = ...; P.X = In_X; ..." for every parameter. */
	FString BuildParamCode(const FHolofyStyleSettings& Settings)
	{
		FString Code = TEXT("FHolofyParams P = (FHolofyParams)0;\n");
		Settings.ForEachVectorParameter([&Code](const FHolofyParamInfo& Info, const FLinearColor&)
		{
			Code += FString::Printf(TEXT("P.%s = In_%s.rgb;\n"), Info.Key, Info.Key);
		});
		Settings.ForEachScalarParameter([&Code](const FHolofyParamInfo& Info, float)
		{
			Code += FString::Printf(TEXT("P.%s = In_%s;\n"), Info.Key, Info.Key);
		});
		return Code;
	}

	struct FParamExpression
	{
		const TCHAR* Key;
		UMaterialExpression* Expression;
	};

	FString ObjectPathFor(const FString& PackagePath, const FString& Name)
	{
		return FString::Printf(TEXT("%s/%s.%s"), *PackagePath, *Name, *Name);
	}

	bool PackageExists(const FString& PackagePath, const FString& Name)
	{
		return FPackageName::DoesPackageExist(PackagePath / Name);
	}

	UMaterial* FindOrCreateMaterial(const FString& Name)
	{
		const FString PackagePath = UHolofyLibrary::MaterialsPath;
		if (PackageExists(PackagePath, Name))
		{
			if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *ObjectPathFor(PackagePath, Name)))
			{
				return Existing;
			}
		}

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
		return Cast<UMaterial>(AssetTools.CreateAsset(Name, PackagePath, UMaterial::StaticClass(), Factory));
	}

	UMaterialInstanceConstant* FindOrCreateInstance(const FString& Name, UMaterialInterface* Parent)
	{
		const FString PackagePath = UHolofyLibrary::PresetsPath;
		if (PackageExists(PackagePath, Name))
		{
			if (UMaterialInstanceConstant* Existing = LoadObject<UMaterialInstanceConstant>(nullptr, *ObjectPathFor(PackagePath, Name)))
			{
				Existing->SetParentEditorOnly(Parent);
				return Existing;
			}
		}

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
		Factory->InitialParent = Parent;
		return Cast<UMaterialInstanceConstant>(AssetTools.CreateAsset(Name, PackagePath, UMaterialInstanceConstant::StaticClass(), Factory));
	}

	void SetTwoSided(UMaterial* Material)
	{
		// Via reflection so we don't care whether the field is public in this engine version.
		if (FBoolProperty* Prop = FindFProperty<FBoolProperty>(UMaterial::StaticClass(), TEXT("TwoSided")))
		{
			Prop->SetPropertyValue_InContainer(Material, true);
		}
	}

	void BuildMaster(UMaterial* Material, EHolofyBlendMode Blend)
	{
		UMaterialEditingLibrary::DeleteAllMaterialExpressions(Material);

		// ---- Material settings ----
		Material->MaterialDomain = MD_Surface;
		Material->SetShadingModel(MSM_Unlit);
		SetTwoSided(Material);

		float BlendHint = 0.f;
		EHolofyStyle DefaultsStyle = EHolofyStyle::Classic;
		switch (Blend)
		{
		case EHolofyBlendMode::Translucent:
			Material->BlendMode = BLEND_Translucent;
			BlendHint = 1.f;
			DefaultsStyle = EHolofyStyle::HoloScreen;
			break;
		case EHolofyBlendMode::Masked:
			Material->BlendMode = BLEND_Masked;
			Material->OpacityMaskClipValue = 0.5f;
			BlendHint = 2.f;
			DefaultsStyle = EHolofyStyle::CRTScreen;
			break;
		default:
			Material->BlendMode = BLEND_Additive;
			break;
		}

		bool bNeedsRecompile = false;
		Material->SetMaterialUsage(bNeedsRecompile, MATUSAGE_SkeletalMesh);
		Material->SetMaterialUsage(bNeedsRecompile, MATUSAGE_InstancedStaticMeshes);
		Material->SetMaterialUsage(bNeedsRecompile, MATUSAGE_NiagaraSprites);
		Material->SetMaterialUsage(bNeedsRecompile, MATUSAGE_NiagaraRibbons);
		Material->SetMaterialUsage(bNeedsRecompile, MATUSAGE_NiagaraMeshParticles);

		const FHolofyStyleSettings Defaults = UHolofyLibrary::GetStylePreset(DefaultsStyle);

		// ---- Parameters (left column) ----
		TArray<FParamExpression> Params;
		int32 Row = 0;
		const int32 ParamX = -1700;
		const int32 RowHeight = 70;

		Defaults.ForEachVectorParameter([&](const FHolofyParamInfo& Info, const FLinearColor& Value)
		{
			UMaterialExpressionVectorParameter* Expr = AddExpression<UMaterialExpressionVectorParameter>(Material, ParamX, Row * RowHeight);
			Expr->ParameterName = Info.Name;
			Expr->DefaultValue = Value;
			Expr->Group = Info.Group;
			Expr->SortPriority = Row;
			Params.Add({ Info.Key, Expr });
			Row += 3; // vector nodes are tall
		});

		Defaults.ForEachScalarParameter([&](const FHolofyParamInfo& Info, float Value)
		{
			UMaterialExpressionScalarParameter* Expr = AddExpression<UMaterialExpressionScalarParameter>(Material, ParamX, Row * RowHeight);
			Expr->ParameterName = Info.Name;
			Expr->DefaultValue = Value;
			Expr->Group = Info.Group;
			Expr->SortPriority = Row;
			Expr->SliderMin = Info.SliderMin;
			Expr->SliderMax = Info.SliderMax;
			Params.Add({ Info.Key, Expr });
			++Row;
		});

		UMaterialExpressionTextureObjectParameter* ContentTex = AddExpression<UMaterialExpressionTextureObjectParameter>(Material, ParamX, (Row + 1) * RowHeight);
		ContentTex->ParameterName = FHolofyStyleSettings::ContentTextureParam;
		ContentTex->Group = FName(TEXT("Content"));
		ContentTex->SortPriority = Row;
		ContentTex->Texture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));

		// ---- Geometry / time inputs ----
		const int32 CtxX = -1250;
		UMaterialExpressionTextureCoordinate* TexCoord = AddExpression<UMaterialExpressionTextureCoordinate>(Material, CtxX, -600);
		UMaterialExpressionScreenPosition* ScreenPos = AddExpression<UMaterialExpressionScreenPosition>(Material, CtxX, -500);
		UMaterialExpressionLocalPosition* LocalPos = AddExpression<UMaterialExpressionLocalPosition>(Material, CtxX, -400);
		UMaterialExpressionVertexNormalWS* NormalWS = AddExpression<UMaterialExpressionVertexNormalWS>(Material, CtxX, -300);
		UMaterialExpressionCameraVectorWS* CameraVec = AddExpression<UMaterialExpressionCameraVectorWS>(Material, CtxX, -200);
		UMaterialExpressionTime* Time = AddExpression<UMaterialExpressionTime>(Material, CtxX, -100);
		UMaterialExpressionVertexColor* VertexColor = AddExpression<UMaterialExpressionVertexColor>(Material, CtxX, 0);

		const FString ParamCode = BuildParamCode(Defaults);

		// ---- Pixel: Holofy Surface ----
		UMaterialExpressionCustom* Surface = AddExpression<UMaterialExpressionCustom>(Material, -600, -200);
		Surface->Description = TEXT("Holofy Surface");
		Surface->OutputType = CMOT_Float4;
		Surface->IncludeFilePaths.Reset();
		Surface->IncludeFilePaths.Add(UHolofyLibrary::ShaderInclude);
		Surface->Inputs.Reset();
		for (const FParamExpression& Param : Params)
		{
			AddCustomInput(Surface, FString(TEXT("In_")) + Param.Key, Param.Expression);
		}
		AddCustomInput(Surface, TEXT("In_ContentTexture"), ContentTex);
		AddCustomInput(Surface, TEXT("In_UV"), TexCoord);
		AddCustomInput(Surface, TEXT("In_ScreenUV"), ScreenPos);
		AddCustomInput(Surface, TEXT("In_LocalPos"), LocalPos);
		AddCustomInput(Surface, TEXT("In_Normal"), NormalWS);
		AddCustomInput(Surface, TEXT("In_CameraVector"), CameraVec);
		AddCustomInput(Surface, TEXT("In_Time"), Time);
		AddCustomInput(Surface, TEXT("In_VertexColor"), VertexColor);
		AddCustomInput(Surface, TEXT("In_VertexAlpha"), VertexColor, VertexColorAlphaOutput);
		Surface->Code = ParamCode
			+ TEXT("FHolofyContext C = (FHolofyContext)0;\n")
			+ TEXT("C.UV = In_UV;\n")
			+ TEXT("C.ScreenUV = In_ScreenUV.xy;\n")
			+ TEXT("C.LocalPos = In_LocalPos;\n")
			+ TEXT("C.Normal = In_Normal;\n")
			+ TEXT("C.CameraVector = In_CameraVector;\n")
			+ TEXT("C.Time = In_Time;\n")
			+ TEXT("C.VertexColor = float4(In_VertexColor.rgb, In_VertexAlpha);\n")
			+ FString::Printf(TEXT("return HolofySurface(P, C, In_ContentTexture, In_ContentTextureSampler, %.1f);\n"), BlendHint);
		Surface->PostEditChange();

		UMaterialExpressionComponentMask* EmissiveMask = AddExpression<UMaterialExpressionComponentMask>(Material, -250, -250);
		EmissiveMask->Input.Connect(0, Surface);
		EmissiveMask->R = 1;
		EmissiveMask->G = 1;
		EmissiveMask->B = 1;
		EmissiveMask->A = 0;
		UMaterialEditingLibrary::ConnectMaterialProperty(EmissiveMask, FString(), MP_EmissiveColor);

		if (Blend != EHolofyBlendMode::Additive)
		{
			UMaterialExpressionComponentMask* AlphaMask = AddExpression<UMaterialExpressionComponentMask>(Material, -250, -100);
			AlphaMask->Input.Connect(0, Surface);
			AlphaMask->R = 0;
			AlphaMask->G = 0;
			AlphaMask->B = 0;
			AlphaMask->A = 1;
			UMaterialEditingLibrary::ConnectMaterialProperty(AlphaMask, FString(), Blend == EHolofyBlendMode::Masked ? MP_OpacityMask : MP_Opacity);
		}

		// ---- Vertex: glitch slices (WPO) ----
		UMaterialExpressionCustom* Vertex = AddExpression<UMaterialExpressionCustom>(Material, -600, 400);
		Vertex->Description = TEXT("Holofy Vertex Glitch");
		Vertex->OutputType = CMOT_Float3;
		Vertex->IncludeFilePaths.Reset();
		Vertex->IncludeFilePaths.Add(UHolofyLibrary::ShaderInclude);
		Vertex->Inputs.Reset();
		for (const FParamExpression& Param : Params)
		{
			AddCustomInput(Vertex, FString(TEXT("In_")) + Param.Key, Param.Expression);
		}
		AddCustomInput(Vertex, TEXT("In_UV"), TexCoord);
		AddCustomInput(Vertex, TEXT("In_LocalPos"), LocalPos);
		AddCustomInput(Vertex, TEXT("In_Time"), Time);
		Vertex->Code = ParamCode
			+ TEXT("FHolofyContext C = (FHolofyContext)0;\n")
			+ TEXT("C.UV = In_UV;\n")
			+ TEXT("C.LocalPos = In_LocalPos;\n")
			+ TEXT("C.Time = In_Time;\n")
			+ TEXT("return HolofyVertexOffset(P, C);\n");
		Vertex->PostEditChange();

		UMaterialExpressionTransform* ToWorld = AddExpression<UMaterialExpressionTransform>(Material, -250, 400);
		ToWorld->Input.Connect(0, Vertex);
		ToWorld->TransformSourceType = TRANSFORMSOURCE_Local;
		ToWorld->TransformType = TRANSFORM_World;
		UMaterialEditingLibrary::ConnectMaterialProperty(ToWorld, FString(), MP_WorldPositionOffset);

		UMaterialEditingLibrary::RecompileMaterial(Material);
		Material->MarkPackageDirty();
	}

	void ApplySettingsToInstance(UMaterialInstanceConstant* Instance, const FHolofyStyleSettings& Settings)
	{
		Instance->ClearParameterValuesEditorOnly();

		Settings.ForEachScalarParameter([Instance](const FHolofyParamInfo& Info, float Value)
		{
			Instance->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(Info.Name), Value);
		});
		Settings.ForEachVectorParameter([Instance](const FHolofyParamInfo& Info, const FLinearColor& Value)
		{
			Instance->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(Info.Name), Value);
		});
		if (Settings.ContentTexture)
		{
			Instance->SetTextureParameterValueEditorOnly(FMaterialParameterInfo(FHolofyStyleSettings::ContentTextureParam), Settings.ContentTexture);
		}

		UMaterialEditingLibrary::UpdateMaterialInstance(Instance);
		Instance->MarkPackageDirty();
	}
}

bool AreMaterialsGenerated()
{
	return PackageExists(UHolofyLibrary::MaterialsPath, UHolofyLibrary::GetBaseMaterialName(EHolofyBlendMode::Additive))
		&& PackageExists(UHolofyLibrary::MaterialsPath, UHolofyLibrary::GetBaseMaterialName(EHolofyBlendMode::Translucent))
		&& PackageExists(UHolofyLibrary::MaterialsPath, UHolofyLibrary::GetBaseMaterialName(EHolofyBlendMode::Masked));
}

bool GenerateAll()
{
	TArray<UPackage*> Packages;
	UMaterial* Masters[4] = {}; // indexed by EHolofyBlendMode

	for (const EHolofyBlendMode Blend : { EHolofyBlendMode::Additive, EHolofyBlendMode::Translucent, EHolofyBlendMode::Masked })
	{
		UMaterial* Material = FindOrCreateMaterial(UHolofyLibrary::GetBaseMaterialName(Blend));
		if (!Material)
		{
			UE_LOG(LogHolofyEditor, Error, TEXT("Failed to create %s"), *UHolofyLibrary::GetBaseMaterialName(Blend));
			return false;
		}
		BuildMaster(Material, Blend);
		Masters[static_cast<int32>(Blend)] = Material;
		Packages.Add(Material->GetPackage());
	}

	for (int32 StyleIndex = 0; StyleIndex < static_cast<int32>(EHolofyStyle::Custom); ++StyleIndex)
	{
		const EHolofyStyle Style = static_cast<EHolofyStyle>(StyleIndex);
		UMaterial* Parent = Masters[static_cast<int32>(UHolofyLibrary::GetRecommendedBlendMode(Style))];
		UMaterialInstanceConstant* Instance = FindOrCreateInstance(UHolofyLibrary::GetPresetMaterialName(Style), Parent);
		if (!Instance)
		{
			UE_LOG(LogHolofyEditor, Error, TEXT("Failed to create %s"), *UHolofyLibrary::GetPresetMaterialName(Style));
			continue;
		}
		ApplySettingsToInstance(Instance, UHolofyLibrary::GetStylePreset(Style));
		Packages.Add(Instance->GetPackage());
	}

	const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(Packages, /*bOnlyDirty=*/false);
	UE_LOG(LogHolofyEditor, Log, TEXT("Holofy generated %d assets (saved: %s)"), Packages.Num(), bSaved ? TEXT("yes") : TEXT("no"));
	return bSaved;
}
}
