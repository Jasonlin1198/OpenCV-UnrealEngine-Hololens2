#pragma once

#include "ARBlueprintLibrary.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ARCameraRenderer.generated.h"

//#include <d3d11.h>

DECLARE_LOG_CATEGORY_EXTERN(ARCameraRenderer, Log, All);

UCLASS()
class OPENCV_API AARCameraRenderer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AARCameraRenderer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UStaticMesh* StaticMesh;
	UStaticMeshComponent* StaticMeshComponent;
	UMaterialInstanceDynamic* DynamicMaterial;
	bool IsTextureParamSet = false;
};
