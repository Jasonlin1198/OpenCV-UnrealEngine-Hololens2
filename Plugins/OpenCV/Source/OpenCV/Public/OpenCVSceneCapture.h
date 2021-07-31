// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Unreal Engine Headers
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SphereComponent.h"
#include "Engine/TextureRenderTarget2D.h"

// Third Party Library Headers
#if PLATFORM_WINDOWS
	#include <zbar.h>
#endif

#include <opencv2/core/mat.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include "zxing/common/Counted.h"
#include "zxing/Binarizer.h"
#include "zxing/MultiFormatReader.h"
#include "zxing/Result.h"
#include "zxing/ReaderException.h"
#include "zxing/common/GlobalHistogramBinarizer.h"
#include "zxing/Exception.h"
#include "zxing/common/IllegalArgumentException.h"
#include "zxing/BinaryBitmap.h"
#include "zxing/DecodeHints.h"
#include "zxing/qrcode/QRCodeReader.h"
#include "zxing/MultiFormatReader.h"
#include "zxing/MatSource.h"

// Standard Lib Headers
#include <vector>
#include <iostream>

#include "OpenCVSceneCapture.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(OpenCVSceneCapture, Log, All);


USTRUCT(BlueprintType)
struct FDecodedObject
{
	GENERATED_USTRUCT_BODY()

public:

	// Type of qr code detected by ZBar
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = OpenCVSceneCapture)
		FString type;

	// Data decoded by qr code 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = OpenCVSceneCapture)
		FString data;

	// Pixel locations of detected image
	std::vector <cv::Point> location;

	// Equals overloader for object comparison
	bool operator==(const FDecodedObject& obj) const
	{
		return type.Equals(obj.type) && data.Equals(obj.data);
	}

};

UCLASS()
class OPENCV_API AOpenCVSceneCapture : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AOpenCVSceneCapture();

	// Container for USceneCapture2D component
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = OpenCVSceneCapture)
		class USceneCaptureComponent2D* sceneCaptureComponent;

	// Container for visible static mesh attached to actor
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = OpenCVSceneCapture)
		UStaticMeshComponent* staticMeshComponent;

	// Image texture resolution to be captured by USceneCapture2D component
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = OpenCVSceneCapture)
		int32 resolutionWidth = 1024;

	// Image texture resolution to be captured by USceneCapture2D component
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = OpenCVSceneCapture)
		int32 resolutionHeight = 1024;

	// Array of decoded objects
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = OpenCVSceneCapture)
		TArray<FDecodedObject> decoded;

	// Enable USceneCapture2D component to render texture
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = OpenCVSceneCapture)
		bool captureEnabled;

	// The current scene frame's corresponding texture
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = OpenCVSceneCapture)
		UTexture2D* SceneTexture;

	// The current scene frame's corresponding texture
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = OpenCVSceneCapture)
		UTexture2D* SceneTextureRaw;

	// Blueprint Event called every time the scene frame is updated
	UFUNCTION(BlueprintImplementableEvent, Category = OpenCVSceneCapture)
		void OnNextSceneFrame();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called when game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Scene Capture functions
	cv::Mat captureSceneToMat();
	UTexture2D* convertMatToTexture(cv::Mat& inputImage, int32 imageResolutionWidth, int32 imageResolutionHeight);
	UTexture2D* convertMatToTextureRaw(cv::Mat& inputImage, int32 imageResolutionWidth, int32 imageResolutionHeight);
	bool convertMatToTextureBoth(cv::Mat& inputImage, int32 imageResolutionWidth, int32 imageResolutionHeight);

	// ZXing functions
	cv::Point toOpenCvPoint(zxing::Ref<zxing::ResultPoint> resultPoint);
	void decodeZXing(cv::Mat& inputImage);

	// Unreal Print Helper
	void printToScreen(FString str, FColor color, float duration = 1.0f);

// ZBar support only on Win64
#if PLATFORM_WINDOWS
	void decode(cv::Mat& inputImage, TArray<FDecodedObject>& decodedObjects);
	void displayBox(cv::Mat& inputImage, TArray<FDecodedObject>& decodedObjects);
#endif

};
