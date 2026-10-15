#include "HolofyDisplay.h"
#include "HolofyComponent.h"
#include "HolofyLibrary.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AHolofyDisplay::AHolofyDisplay()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ScreenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenMesh"));
	ScreenMesh->SetupAttachment(RootComponent);
	ScreenMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScreenMesh->SetCastShadow(false);
	// Engine plane lies flat in XY facing +Z. Stand it up facing the actor's +X.
	ScreenMesh->SetRelativeRotation(FRotator(0.0, 90.0, 90.0));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		ScreenMesh->SetStaticMesh(PlaneMesh.Object);
	}

	Hologram = CreateDefaultSubobject<UHolofyComponent>(TEXT("Hologram"));
	Hologram->Style = EHolofyStyle::HoloScreen;
}

void AHolofyDisplay::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Plane is 100x100; local X ends up horizontal and local Y vertical.
	ScreenMesh->SetRelativeScale3D(FVector(FMath::Max(ScreenSize.X, 1.0) / 100.0, FMath::Max(ScreenSize.Y, 1.0) / 100.0, 1.0));

	// Editor preview: show the matching preset instance (the live dynamic material takes over at BeginPlay).
	if (UMaterialInterface* Preview = UHolofyLibrary::GetPresetMaterial(Hologram->Style == EHolofyStyle::Custom ? EHolofyStyle::HoloScreen : Hologram->Style))
	{
		ScreenMesh->SetMaterial(0, Preview);
	}
}
