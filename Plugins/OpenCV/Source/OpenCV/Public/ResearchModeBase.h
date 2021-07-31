// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#if defined(__aarch64__) || defined(_M_ARM64)
	#include "ResearchModeApi.h"
#endif

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResearchModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ResearchModeBase, Log, All);

UCLASS()
class OPENCV_API AResearchModeBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AResearchModeBase();

	// Container for visible static mesh attached to actor
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ResearchMode)
		UStaticMeshComponent* StaticMeshComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called when game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if defined(__aarch64__) || defined(_M_ARM64)
	HRESULT hr = S_OK;

	// Instantiate device, snesor descriptors for main sensor loop usage
	IResearchModeSensorDevice* pSensorDevice;
	std::vector<ResearchModeSensorDescriptor> sensorDescriptors;

	// Selected Sensor and respective frame pointers
	IResearchModeSensor* pSensor;
	IResearchModeSensorFrame* pSensorFrame;

#endif
};
