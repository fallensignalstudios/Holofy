#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HolofyDisplay.generated.h"

class UStaticMeshComponent;
class UHolofyComponent;

/**
 * Drag-and-drop screen: an upright plane + a Holofy component, defaulting to the Holo Screen style.
 * Set Hologram > Content Texture to a texture / render target / media texture and you're broadcasting.
 */
UCLASS(Blueprintable, ClassGroup = (Holofy), meta = (DisplayName = "Holofy Display"))
class HOLOFY_API AHolofyDisplay : public AActor
{
	GENERATED_BODY()

public:
	AHolofyDisplay();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Holofy")
	TObjectPtr<UStaticMeshComponent> ScreenMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Holofy")
	TObjectPtr<UHolofyComponent> Hologram;

	/** Screen width / height in units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holofy|Display")
	FVector2D ScreenSize = FVector2D(160.0, 90.0);

	virtual void OnConstruction(const FTransform& Transform) override;
};
