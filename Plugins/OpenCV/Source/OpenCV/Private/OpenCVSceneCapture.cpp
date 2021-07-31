//**********************************************************************************
//
//  This Unreal Engine Actor handles different implementations for Windows and the Hololens.
//  This Actor captures in-game scene frames, converts the frames into OpenCV Mats to be
//  processed by image detection libraries. ZBar is not currently integrated with the 
//  Hololens so frames are decoded with ZXing. The output decoded frames are then converted
//  into UTexture2D objects for use in blueprints. Capture must be enabled either through actor
//  properties in the editor or through the blueprint exposed captureEnabled flag. 
// 
//  Important: When capturing scene to mat on hololens, crash and error output occurrs:
// 
//[2021.07.31-19.11.11:437][394]LogStats: FPlatformStackWalk::StackWalkAndDump -  0.000 s
//[2021.07.31 - 19.11.11:437][394]LogOutputDevice: Error: == = Handled ensure : == =
//[2021.07.31 - 19.11.11:437][394]LogOutputDevice : Error :
//[2021.07.31 - 19.11.11:437] [394] LogOutputDevice : Error : Ensure condition failed : ((Result) >= 0)[File:D: / Build / ++UE4 / Sync / Engine / Plugins / Runtime / OpenXR / Source / OpenXRHMD / Private / OpenXRHMD.cpp][Line:2398]
//[2021.07.31 - 19.11.11:437][394]LogOutputDevice : Error : OpenXR call failed with result XR_ERROR_LAYER_INVALID
//[2021.07.31 - 19.11.11:437][394]LogOutputDevice : Error : Stack :
//[2021.07.31 - 19.11.11:437] [394] LogOutputDevice : Error :
//[2021.07.31 - 19.11.11:437] [394] LogStats : SubmitErrorReport - 0.000 s
//[2021.07.31 - 19.11.11:437][394]LogStats : SendNewReport - 0.000 s
//[2021.07.31 - 19.11.11:437][394]LogStats : FDebug::EnsureFailed - 0.000 s
// 
// Currently, functionality is set to be disabled on Hololens
//     
//
//**********************************************************************************

#include "OpenCVSceneCapture.h"

#if defined(__aarch64__) || defined(_M_ARM64)
	//#include "zxing/MatSource.cpp"
#endif

DEFINE_LOG_CATEGORY(OpenCVSceneCapture)

using namespace std;

// Sets default values
AOpenCVSceneCapture::AOpenCVSceneCapture()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize features exposed to blueprint
	captureEnabled = false;

	// Attach scenecapture camera to actor and set as root
	sceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Scene Capture"));
	sceneCaptureComponent->bCaptureEveryFrame = false;
	// Create Static Mesh Component and attach Cube to camera
	staticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	UStaticMesh* cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object;
	staticMeshComponent->SetStaticMesh(cubeMesh);
	staticMeshComponent->AttachToComponent(sceneCaptureComponent, FAttachmentTransformRules::KeepRelativeTransform);
	staticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set camera to as root
	SetRootComponent(sceneCaptureComponent);
}


// Called when the game starts or when spawned
void AOpenCVSceneCapture::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AOpenCVSceneCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Main Driver code for handling OpenCV functionality
	if (captureEnabled) {

		// Ensure that resolution is power of 2, display final resolution in actor properties
		resolutionWidth = FGenericPlatformMath::Pow(2, FGenericPlatformMath::FloorLog2(FGenericPlatformMath::Max(resolutionWidth, 1) * 2 - 1));
		resolutionHeight = FGenericPlatformMath::Pow(2, FGenericPlatformMath::FloorLog2(FGenericPlatformMath::Max(resolutionHeight, 1) * 2 - 1));

		//const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		//resolutionWidth = ViewportSize.X;
		//resolutionHeight = ViewportSize.Y;

		UE_LOG(OpenCVSceneCapture, Warning, TEXT("Viewport X: %d"), resolutionWidth);
		UE_LOG(OpenCVSceneCapture, Warning, TEXT("Viewport Y: %d"), resolutionHeight);

#if PLATFORM_WINDOWS

		// Capture scene view, process image, and create texture
		cv::Mat inputImage = captureSceneToMat();

		// Check that image has elements
		if (!inputImage.empty()) {
			UE_LOG(OpenCVSceneCapture, Warning, TEXT("Begin Decoding"));

			// Process and decode scene frames for target platforms
			#if defined(__aarch64__) || defined(_M_ARM64)

				decodeZXing(inputImage);

			#endif
				
				TArray<FDecodedObject> decodedObjects;

				// Find and decode barcodes and QR codes
				decode(inputImage, decodedObjects);

				// Display location with box
				displayBox(inputImage, decodedObjects);

				// Reset array
				decodedObjects.Empty();

				UE_LOG(OpenCVSceneCapture, Warning, TEXT("Converting to Texture"));

				// Display image with detected box only as blueprint property
				SceneTexture = convertMatToTexture(inputImage, resolutionWidth, resolutionHeight);

				// Display raw image with detection as blueprint property
				SceneTextureRaw = convertMatToTextureRaw(inputImage, resolutionWidth, resolutionHeight);

		}

#endif

	}
	// Clear array of decoded images when capture is ended
	else {
		decoded.Empty();
	}

	// Call custom event handler function
	OnNextSceneFrame();
}


void AOpenCVSceneCapture::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

// Capture scene view from scene capture component and pass image into OpenCV processing
cv::Mat AOpenCVSceneCapture::captureSceneToMat() {

	// Create new texture target
	UTextureRenderTarget2D* currentTarget = NewObject<UTextureRenderTarget2D>();
	currentTarget->InitCustomFormat(resolutionWidth, resolutionHeight, PF_B8G8R8A8, false);

	// Assign to scene capture component
	sceneCaptureComponent->TextureTarget = currentTarget;

	// Capture scene texture
	sceneCaptureComponent->CaptureScene();

	// Create container for pixels
	TArray<FColor> imagePixels;
	imagePixels.Empty();
	imagePixels.Reserve(resolutionWidth * resolutionHeight);

	// Get texture render target
	FTextureRenderTargetResource* textureResource = currentTarget->GameThread_GetRenderTargetResource();

	// Populate RGBA8 texture pixels into buffer
	bool readPixelResult = textureResource->ReadPixels(imagePixels);

	// Pixel read unsuccessful
	if (!readPixelResult) {
		UE_LOG(OpenCVSceneCapture, Warning, TEXT("Failed read pixels from render target resource"));
	}

	// Shink array to number of pixels read
	imagePixels.Shrink();

	// Print pixel data to output log
	//for (int32 i = 0; i < imagePixels.Num(); i++) {
	//	UE_LOG(OpenCVSceneCapture, Warning, TEXT("%s"), *imagePixels[i].ToString());
	//}

	// Create array for 8-bit int conversion
	TArray<uint8> pixelData;
	int32 bufferSize = imagePixels.Num() * 4;
	pixelData.Reserve(bufferSize);

	// Copy texture render pixels into array with BGR format OpenCV expects
	for (int i = 0; i < imagePixels.Num(); i++) {
		pixelData.Append({ imagePixels[i].B, imagePixels[i].G, imagePixels[i].R, 0xff });
	}

	// Get pointer to first element of array
	void* imageData = pixelData.GetData();

	// Construct OpenCv image defined by image width, height, 8-bit 4 color channel pixels, and pointer to data
	cv::Mat inputImage = cv::Mat(resolutionWidth, resolutionHeight, CV_8UC4, imageData);

	// Return clone to get valid Mat reference since inputImage is invalid after function return
	return inputImage.clone();
}


// Convert OpenCV detection image into UE texture2D image, currently uses zbar decoding and locates blue pixels drawn from displayBox()
UTexture2D* AOpenCVSceneCapture::convertMatToTexture(cv::Mat& inputImage, int32 imageResolutionWidth, int32 imageResolutionHeight) {

	TArray<FColor> pixels;

	// Convert OpenCV Mat image into UE formatted pixel arrays
	for (int x = 0; x < imageResolutionWidth; x++) {
		for (int y = 0; y < imageResolutionHeight; y++) {
			// Current Color point
			cv::Vec4b point = inputImage.at<cv::Vec4b>(x, y);

			// Append pure blue pixels, otherwise transparent
			if (point[0] == 255 && point[1] == 0 && point[2] == 0) {
				FColor pix(point[0], point[1], point[2], 0xff);
				pixels.Add(pix);
			}
			else {
				FColor pix(0, 0, 0, 0x00);
				pixels.Add(pix);
			}
		}
	}

	// Create modified texture with RGB encoding format
	UTexture2D* texture = UTexture2D::CreateTransient(imageResolutionWidth, imageResolutionHeight, EPixelFormat::PF_R8G8B8A8, "OpenCV Texture Output");

	if (!texture) {
		UE_LOG(OpenCVSceneCapture, Error, TEXT("Failed to create output texture"));
		return nullptr;
	}

	void* data = texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	// Copy data of pixels with 4 channels into pointer
	FMemory::Memcpy(data, pixels.GetData(), imageResolutionWidth * imageResolutionHeight * 4);
	texture->PlatformData->Mips[0].BulkData.Unlock();
	texture->UpdateResource();
	texture->Filter = TextureFilter::TF_Nearest;

	return texture;
}


// Convert OpenCV output image into UE texture2D image
UTexture2D* AOpenCVSceneCapture::convertMatToTextureRaw(cv::Mat& inputImage, int32 imageResolutionWidth, int32 imageResolutionHeight) {

	TArray<FColor> pixels;

	// Convert OpenCV Mat image into UE formatted pixel arrays
	for (int x = 0; x < imageResolutionWidth; x++) {
		for (int y = 0; y < imageResolutionHeight; y++) {
			// Current Color point
			cv::Vec4b point = inputImage.at<cv::Vec4b>(x, y);
			// Append all pixels into raw array
			FColor pix(point[0], point[1], point[2], 0xff);
			pixels.Add(pix);
		}
	}

	// Create modified texture with RGB encoding format
	UTexture2D* texture = UTexture2D::CreateTransient(imageResolutionWidth, imageResolutionHeight, EPixelFormat::PF_R8G8B8A8, "OpenCV Raw Texture Output");

	if (!texture) {
		UE_LOG(OpenCVSceneCapture, Error, TEXT("Failed to create raw output texture"));
		return nullptr;
	}

	void* data = texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	// Copy data of pixels with 4 channels into pointer
	FMemory::Memcpy(data, pixels.GetData(), imageResolutionWidth * imageResolutionHeight * 4);
	texture->PlatformData->Mips[0].BulkData.Unlock();
	texture->UpdateResource();
	texture->Filter = TextureFilter::TF_Nearest;

	return texture;
}


// Converts decoded border texture and OpenCV output image into UE texture2D image
bool AOpenCVSceneCapture::convertMatToTextureBoth(cv::Mat& inputImage, int32 imageResolutionWidth, int32 imageResolutionHeight) {

	TArray<FColor> pixels;
	TArray<FColor> pixelsRaw;

	// Convert OpenCV Mat image into UE formatted pixel arrays
	for (int x = 0; x < imageResolutionWidth; x++) {
		for (int y = 0; y < imageResolutionHeight; y++) {
			// Current Color point
			cv::Vec4b point = inputImage.at<cv::Vec4b>(x, y);

			// Append pure blue pixels, otherwise transparent
			if (point[0] == 255 && point[1] == 0 && point[2] == 0) {
				FColor pix(point[0], point[1], point[2], 0xff);
				pixels.Add(pix);
			}
			else {
				FColor pix(0, 0, 0, 0x00);
				pixels.Add(pix);
			}
			// Append all pixels into raw array
			FColor pixRaw(point[0], point[1], point[2], 0xff);
			pixelsRaw.Add(pixRaw);
		}
	}

	// Create modified texture with RGB encoding format
	UTexture2D* texture = UTexture2D::CreateTransient(imageResolutionWidth, imageResolutionHeight, EPixelFormat::PF_R8G8B8A8, "OpenCV Texture Output");

	if (!texture) {
		UE_LOG(OpenCVSceneCapture, Error, TEXT("Failed to create output texture"));
		return false;
	}

	void* data = texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(data, pixels.GetData(), imageResolutionWidth * imageResolutionHeight * 4);
	texture->PlatformData->Mips[0].BulkData.Unlock();
	texture->UpdateResource();
	texture->Filter = TextureFilter::TF_Nearest;

	SceneTexture = texture;

	// Create raw output texture
	UTexture2D* textureRaw = UTexture2D::CreateTransient(imageResolutionWidth, imageResolutionHeight, EPixelFormat::PF_R8G8B8A8, "OpenCV Texture Output Raw");

	if (!textureRaw) {
		UE_LOG(OpenCVSceneCapture, Error, TEXT("Failed to create raw output texture"));
		return false;
	}

	void* dataRaw = textureRaw->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	// Copy data of pixels with 4 channels into pointer
	FMemory::Memcpy(dataRaw, pixelsRaw.GetData(), imageResolutionWidth * imageResolutionHeight * 4);
	textureRaw->PlatformData->Mips[0].BulkData.Unlock();
	textureRaw->UpdateResource();
	textureRaw->Filter = TextureFilter::TF_Nearest;

	SceneTextureRaw = textureRaw;

	return true;
}


cv::Point AOpenCVSceneCapture::toOpenCvPoint(zxing::Ref<zxing::ResultPoint> resultPoint) {
	return cv::Point(resultPoint->getX(), resultPoint->getY());
}

void AOpenCVSceneCapture::decodeZXing(cv::Mat& inputImage) {
	cv::Mat grey;

	//cv::cvtColor(inputImage, grey, cv::COLOR_BGR2GRAY);

	try {

		// Create luminance source
		zxing::Ref<zxing::LuminanceSource> source = MatSource::create(inputImage);

		// Search for QR code
		zxing::Ref<zxing::Reader> reader;

		// Create QR code reader object
		reader.reset(new zxing::qrcode::QRCodeReader);

		zxing::Ref<zxing::Binarizer> binarizer(new zxing::GlobalHistogramBinarizer(source));
		zxing::Ref<zxing::BinaryBitmap> bitmap(new zxing::BinaryBitmap(binarizer));
		zxing::Ref<zxing::Result> result(reader->decode(bitmap, zxing::DecodeHints(zxing::DecodeHints::QR_CODE_HINT)));

		// Get result point count
		int resultPointCount = result->getResultPoints()->size();

		for (int j = 0; j < resultPointCount; j++) {

			// Draw circle
			cv::circle(inputImage, toOpenCvPoint(result->getResultPoints()[j]), 10, cv::Scalar(255, 0, 0), 2);

		}

		// Draw boundary on image
		if (resultPointCount > 1) {

			for (int j = 0; j < resultPointCount; j++) {

				// Get start result point
				zxing::Ref<zxing::ResultPoint> previousResultPoint = (j > 0) ? result->getResultPoints()[j - 1] : result->getResultPoints()[resultPointCount - 1];

				// Draw line
				cv::line(inputImage, toOpenCvPoint(previousResultPoint), toOpenCvPoint(result->getResultPoints()[j]), cv::Scalar(255, 0, 0), 2, 8);

				// Update previous point
				previousResultPoint = result->getResultPoints()[j];

			}

		}
		if (resultPointCount > 0) {

			FString text(result->getText()->getText().c_str());

			UE_LOG(OpenCVSceneCapture, Warning, TEXT("QR code detected: %s"), *text);

			printToScreen(FString::Printf(TEXT("------------------------------------- QR code detected: %s"), *text), FColor::Green, 2.0f);

			// Draw text
			cv::putText(inputImage, result->getText()->getText(), toOpenCvPoint(result->getResultPoints()[0]), cv::LINE_4, 1, cv::Scalar(255, 0, 0));

			FDecodedObject obj;

			// Print type and data
			FString type("Qrcode");

			obj.type = type;
			obj.data = text;

			// Add unique detections to visible property
			decoded.AddUnique(obj);

		}
	}
	catch (const zxing::ReaderException& e) {
		cout << e.what() << endl;
	}
	catch (const zxing::IllegalArgumentException& e) {
		cout << e.what() << endl;
	}
	catch (const zxing::Exception& e) {
		cout << e.what() << endl;
	}
	catch (const std::exception& e) {
		cout << e.what() << endl;
	}
}

void AOpenCVSceneCapture::printToScreen(FString str, FColor color, float duration) {
	GEngine->AddOnScreenDebugMessage(-1, duration, color, *str);
}

#if PLATFORM_WINDOWS
// Find and decode barcodes and QR codes
void AOpenCVSceneCapture::decode(cv::Mat& inputImage, TArray<FDecodedObject>& decodedObjects)
{
	// Create zbar scanner
	zbar::ImageScanner scanner;

	// Configure scanner to detect image types
	scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

	// Convert image to grayscale
	cv::Mat imGray;
	cv::cvtColor(inputImage, imGray, cv::COLOR_BGR2GRAY);

	// Wrap opencv Mat image data in a zbar image
	zbar::Image image(inputImage.cols, inputImage.rows, "Y800", (uchar*)imGray.data, inputImage.cols * inputImage.rows);

	// Scan the image for barcodes and QRCodes
	int n = scanner.scan(image);

	if (n <= 0) {
		UE_LOG(OpenCVSceneCapture, Warning, TEXT("No QR Detected"));
		return;
	}

	// Print results
	for (zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
	{
		FDecodedObject obj;

		// Print type and data
		FString type(symbol->get_type_name().c_str());
		FString data(symbol->get_data().c_str());

		obj.type = type;
		obj.data = data;

		// Print data to Log
		UE_LOG(OpenCVSceneCapture, Warning, TEXT("%s"), *type);
		UE_LOG(OpenCVSceneCapture, Warning, TEXT("%s"), *data);

		// Print data to screen
		static double deltaTime = FApp::GetDeltaTime();
		printToScreen(obj.type, FColor::Green, deltaTime);
		printToScreen(obj.data, FColor::Blue, deltaTime);

		// Obtain location
		for (int i = 0; i < symbol->get_location_size(); i++)
		{
			obj.location.push_back(cv::Point(symbol->get_location_x(i), symbol->get_location_y(i)));
		}

		// Add unique detections to visible property
		decoded.AddUnique(obj);

		// Add all detections for displaying image outlines
		decodedObjects.Add(obj);
	}
}

// Display barcode outline at QR code location
void AOpenCVSceneCapture::displayBox(cv::Mat& inputImage, TArray<FDecodedObject>& decodedObjects)
{
	// Loop over all decoded objects
	for (int i = 0; i < decodedObjects.Num(); i++)
	{
		vector<cv::Point> points = decodedObjects[i].location;
		vector<cv::Point> hull;

		// If the points do not form a quad, find convex hull
		if (points.size() > 4) {
			convexHull(points, hull);
		}
		else {
			hull = points;
		}

		// Number of points in the convex hull
		int n = hull.size();

		for (int j = 0; j < n; j++)
		{
			line(inputImage, hull[j], hull[(j + 1) % n], cv::Scalar(255, 0, 0), 2);
		}

	}
}
#endif