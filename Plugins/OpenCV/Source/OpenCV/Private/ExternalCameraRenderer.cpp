//**********************************************************************************
//
//  This Unreal Engine Actor handles different implementations for Windows and the Hololens.
//  When run with Windows, the actor sets up video capturing through external cameras 
//  using OpenCV then decodes and detects QR codes using the ZBar library. The output of the 
//  camera and decoding is then converted to a UTexture2D and displayed on a mesh in game for
//  feed preview. Additional blueprint implementation to display the texture is required, examples
//  of which is located in the project content browser.
// 
//  For the Hololens, this actor uses research mode api to access the front left sensor frames
//  and decodes qr codes with ZXing library. The output is also displayed on a mesh to view
//  through the same blueprint implementation. 
// 
//  Important: ToggleARCapture with EARCaptureType::Camera mode must be set to true prior to opening 
//  research mode sensors. A delay is needed after toggle before opening. This can be done by setting
//  an event by timer after the toggle in a blueprint, then spawning this actor into the level.  
//     
//
//**********************************************************************************

#include "ExternalCameraRenderer.h"

#if defined(__aarch64__) || defined(_M_ARM64)
extern "C"
HMODULE LoadLibraryA(
	LPCSTR lpLibFileName
);

#include "zxing/MatSource.cpp"
#endif

using namespace std;

DEFINE_LOG_CATEGORY(ExternalCameraRenderer)


// Sets default values
AExternalCameraRenderer::AExternalCameraRenderer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

	PrimaryActorTick.bCanEverTick = true;
	rootComp = CreateDefaultSubobject<USceneComponent>("Root");
	Screen_Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Screen_Mesh");
	Screen_Mesh->AttachToComponent(rootComp, FAttachmentTransformRules::KeepRelativeTransform);
	SetRootComponent(rootComp);

	// Initialize OpenCV and webcam properties
	isStreamOpen = false;
	VideoSize = FVector2D(640, 480);
	RefreshRate = 30.0f;

#if PLATFORM_WINDOWS
	CameraID = 0;
	VideoTrackID = 0;
	cvVideoCapture = cv::VideoCapture();
#endif
}


// Called when the game starts or when spawned
void AExternalCameraRenderer::BeginPlay() {
	Super::BeginPlay();
#if defined(__aarch64__) || defined(_M_ARM64)

	// create dynamic texture
	Camera_Texture2D = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y, PF_B8G8R8A8);

	// Initialize number of accessible sensors
	size_t sensorCount = 0;

	// Load Research Mode functions and create CreateResearchModeSensorDevice() for use
	HMODULE hrResearchMode = LoadLibraryA("ResearchModeAPI");
	if (hrResearchMode)
	{
		typedef HRESULT(__cdecl* PFN_CREATEPROVIDER) (IResearchModeSensorDevice** ppSensorDevice);
		PFN_CREATEPROVIDER pfnCreate = reinterpret_cast<PFN_CREATEPROVIDER>(GetProcAddress(hrResearchMode, "CreateResearchModeSensorDevice"));
		if (pfnCreate)
		{
			hr = pfnCreate(&pSensorDevice);
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}

	// Makes cameras run at full frame rate rather than optimized for headtracker use
	pSensorDevice->DisableEyeSelection();

	// Get number of sensors on device
	hr = pSensorDevice->GetSensorCount(&sensorCount);

	// resize vector to number of sensor descriptor objects
	sensorDescriptors.resize(sensorCount);

	// Populate sensor descriptor objects with device data
	hr = pSensorDevice->GetSensorDescriptors(sensorDescriptors.data(), sensorDescriptors.size(), &sensorCount);

	for (auto& sensorDescriptor : sensorDescriptors)
	{
		// Sensor frame read thread
		size_t sampleBufferSize;

		// Reset for each sensor type
		pSensor = nullptr;
		pSensorFrame = nullptr;

		hr = pSensorDevice->GetSensor(
			sensorDescriptor.sensorType,
			&pSensor);

		// Error Check Getting Sensor Types
		if (SUCCEEDED(hr)) {
			UE_LOG(ExternalCameraRenderer, Warning, TEXT("Get Sensor Device Type Successful"));
		}

		// Open and process only specified sensor
		if (pSensor->GetSensorType() == LEFT_FRONT) {
			UE_LOG(ExternalCameraRenderer, Warning, TEXT("Get Left Front Sensor"));

			hr = pSensor->GetSampleBufferSize(&sampleBufferSize);
			hr = pSensor->OpenStream();

			// Error Check Stream Opening
			if (SUCCEEDED(hr)) {
				UE_LOG(ExternalCameraRenderer, Warning, TEXT("Opening Stream Successful"));
				isStreamOpen = true;
			}
			else {
				UE_LOG(ExternalCameraRenderer, Warning, TEXT("Opening Stream Failed"));

			}

			// Keep pSensor to point at Left_front sensor
			break;

		}
	}


#elif PLATFORM_WINDOWS

	if (!cvVideoCapture.isOpened()){
		cvVideoCapture.open(CameraID);
	}

	isStreamOpen = true;

	Camera_Texture2D = UTexture2D::CreateTransient(VideoSize.X, VideoSize.Y, PF_B8G8R8A8);
#if WITH_EDITORONLY_DATA
	Camera_Texture2D->MipGenSettings = TMGS_NoMipmaps;
#endif
	Camera_Texture2D->SRGB = Camera_RenderTarget->SRGB;

#endif
}

void AExternalCameraRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if defined(__aarch64__) || defined(_M_ARM64)

	// Clean up Sensor object and stop frame capture
	hr = pSensor->CloseStream();
	if (pSensor) {
		pSensor->Release();
	}

	// Clean up Sensor Device object
	pSensorDevice->EnableEyeSelection();
	pSensorDevice->Release();

#endif
}


void AExternalCameraRenderer::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	RefreshTimer += DeltaTime;
	if (isStreamOpen && RefreshTimer >= 1.0f / RefreshRate) {
		RefreshTimer -= 1.0f / RefreshRate;
		ReadFrame();
		OnNextVideoFrame();
	}

}

void AExternalCameraRenderer::ReadFrame() {

#if defined(__aarch64__) || defined(_M_ARM64)

	// Retrieve next available buffer frame from sensor
	hr = pSensor->GetNextBuffer(&pSensorFrame);

	// Error Check Recieving Buffer frame
	if (SUCCEEDED(hr)) {
		UE_LOG(ExternalCameraRenderer, Warning, TEXT("Get Next Buffer From Frame Successful"));
	}
	else {
		UE_LOG(ExternalCameraRenderer, Warning, TEXT("Failed to get buffer"));
	}

	// Process Visual-light camera frame
	ResearchModeSensorResolution resolution;
	size_t outBufferCount = 0;
	const BYTE* pImage = nullptr;
	IResearchModeSensorVLCFrame* pVLCFrame = nullptr;

	pSensorFrame->GetResolution(&resolution);
	hr = pSensorFrame->QueryInterface(IID_PPV_ARGS(&pVLCFrame));

	if (SUCCEEDED(hr))
	{
		pVLCFrame->GetBuffer(&pImage, &outBufferCount);

		if (!pImage) {
			UE_LOG(ExternalCameraRenderer, Warning, TEXT("Empty Sensor Frame"));
			return;
		}

		// Construct OpenCV image defined by image width, height, 8-bit pixels, and pointer to data
		cv::Mat inputImage = cv::Mat(resolution.Height, resolution.Width, CV_8U, (void*)pImage);
		
		if (inputImage.empty()) {
			UE_LOG(ExternalCameraRenderer, Warning, TEXT("Empty Mat"));
			return;
		}

		////Print VLC Buffer values, works on hl
		//for (size_t i = 0; i < outBufferCount; ++i)
		//{
		//	uint8_t pixel= (uint8_t)pImage[i];
		// 	UE_LOG(ExternalCameraRenderer, Warning, TEXT( Pixel Buffer: %d"), pixel);
		//}


		// Qr code detector init
		//cv::QRCodeDetector qrDecoder = cv::QRCodeDetector::QRCodeDetector();
		//std::vector<cv::Point> qrPoints;
		//std::string data = qrDecoder.detectAndDecode(processed, qrPoints);

		//	if (data.length() > 0)
		//	{
		//		UE_LOG(ExternalCameraRenderer, Warning, TEXT("QR code detected"));
		// 
		//		// Create rectangle from upper left and bottom right output array points
		//		rectangle(processed, qrPoints[0], qrPoints[2], Scalar(255, 0, 255), 1);
		//	}


		// Decode Mat with ZXing library
		decodeZXing(inputImage);

		TArray<FColor> pixels;

		int32 imageResolutionWidth = inputImage.cols;
		int32 imageResolutionHeight = inputImage.rows;

		UE_LOG(ExternalCameraRenderer, Warning, TEXT("Mat Image Width: %d"), imageResolutionWidth);
		UE_LOG(ExternalCameraRenderer, Warning, TEXT("Mat Image Height %d"), imageResolutionHeight);

		pixels.Init(FColor(0, 0, 0, 255), imageResolutionWidth * imageResolutionHeight);

		// Copy Mat data to Data array
		for (int y = 0; y < imageResolutionHeight; y++)
		{
			for (int x = 0; x < imageResolutionWidth; x++)
			{
				int i = x + (y * imageResolutionWidth);
				pixels[i].B = inputImage.data[i];
				pixels[i].G = inputImage.data[i];
				pixels[i].R = inputImage.data[i];
			}
		}

		// Lock the texture so we can read / write to it
		void* TextureData = Camera_Texture2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		const int32 TextureDataSize = pixels.Num() * 4;
		// set the texture data
		FMemory::Memcpy(TextureData, pixels.GetData(), TextureDataSize);
		// Unlock the texture
		Camera_Texture2D->PlatformData->Mips[0].BulkData.Unlock();
		// Apply Texture changes to GPU memory
		Camera_Texture2D->UpdateResource();

		// Release current Visual Light Camera frame
		if (pVLCFrame){
			pVLCFrame->Release();
		}

		// Release IResearchModeSensorFrame interface memory.
		if (pSensorFrame) {
			pSensorFrame->Release();
		}
	}


#elif PLATFORM_WINDOWS
	// Check texture and render target are initialized before accessing
	if (!Camera_Texture2D || !Camera_RenderTarget) {
		return;
	}

	cv::Mat inputImage;
	cvVideoCapture.read(inputImage);

	TArray<FDecodedObject2> decodedObjects;

	// Find and decode barcodes and QR codes
	decode(inputImage, decodedObjects);

	// Display location
	displayBox(inputImage, decodedObjects);

	TArray<FColor> pixels;

	int32 imageResolutionWidth = inputImage.cols;
	int32 imageResolutionHeight = inputImage.rows;

	pixels.Init(FColor(0, 0, 0, 255), imageResolutionWidth * imageResolutionHeight);

	// Copy Mat data to Data array
	for (int y = 0; y < imageResolutionHeight; y++)
	{
		for (int x = 0; x < imageResolutionWidth; x++)
		{
			int i = x + (y * imageResolutionWidth);
			pixels[i].B = inputImage.data[i * 3 + 0];
			pixels[i].G = inputImage.data[i * 3 + 1];
			pixels[i].R = inputImage.data[i * 3 + 2];
		}
	}

	// Lock the texture so we can read / write to it
	void* TextureData = Camera_Texture2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	const int32 TextureDataSize = pixels.Num() * 4;
	// set the texture data
	FMemory::Memcpy(TextureData, pixels.GetData(), TextureDataSize);
	// Unlock the texture
	Camera_Texture2D->PlatformData->Mips[0].BulkData.Unlock();
	// Apply Texture changes to GPU memory
	Camera_Texture2D->UpdateResource();

#endif

}


#if PLATFORM_WINDOWS

// Find and decode barcodes and QR codes
void AExternalCameraRenderer::decode(cv::Mat& inputImage, TArray<FDecodedObject2>& decodedObjects)
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
		return;
	}

	// Print results
	for (zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
	{
		FDecodedObject2 obj;

		// Print type and data
		FString type(symbol->get_type_name().c_str());
		FString data(symbol->get_data().c_str());

		obj.type = type;
		obj.data = data;

		// Print data to screen
		static double deltaTime = FApp::GetDeltaTime();
		printToScreen(obj.type, FColor::Green, deltaTime);
		printToScreen(obj.data, FColor::Blue, deltaTime);

		// Obtain location
		for (int i = 0; i < symbol->get_location_size(); i++)
		{
			obj.location.push_back(cv::Point(symbol->get_location_x(i), symbol->get_location_y(i)));
		}

		// Add all detections for displaying image outlines
		decodedObjects.Add(obj);
	}
}


// Display barcode outline at QR code location
void AExternalCameraRenderer::displayBox(cv::Mat& inputImage, TArray<FDecodedObject2>& decodedObjects)
{
	// Loop over all decoded objects
	for (int i = 0; i < decodedObjects.Num(); i++)
	{
		vector<cv::Point> points = decodedObjects[i].location;
		vector<cv::Point> hull;

		// If the points do not form a quad, find convex hull
		if (points.size() > 4) {
			cv::convexHull(points, hull);
		}
		else {
			hull = points;
		}

		// Number of points in the convex hull
		int n = hull.size();

		for (int j = 0; j < n; j++)
		{
			cv::line(inputImage, hull[j], hull[(j + 1) % n], cv::Scalar(255, 0, 0), 2);
		}

	}
}
#endif


cv::Point AExternalCameraRenderer::toOpenCvPoint(zxing::Ref<zxing::ResultPoint> resultPoint) {
	return cv::Point(resultPoint->getX(), resultPoint->getY());
}

void AExternalCameraRenderer::decodeZXing(cv::Mat& inputImage) {

	cv::Mat grey;
	//cvtColor(inputImage, grey, cv::COLOR_BGR2GRAY);

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

			UE_LOG(ExternalCameraRenderer, Warning, TEXT("QR code detected: %s"), *text);

			printToScreen(FString::Printf(TEXT("------------------------------------- QR code detected: %s"), *text), FColor::Green, 2.0f);

			// Draw text
			cv::putText(inputImage, result->getText()->getText(), toOpenCvPoint(result->getResultPoints()[0]), cv::LINE_4, 1, cv::Scalar(255, 0, 0));
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


void AExternalCameraRenderer::printToScreen(FString str, FColor color, float duration) {
	GEngine->AddOnScreenDebugMessage(-1, duration, color, *str);
}

