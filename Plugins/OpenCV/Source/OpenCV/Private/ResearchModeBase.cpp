//**********************************************************************************
//
//	Base class that implements the Hololens Research Mode API to access VLC 
//  (Visual Light Camera) sensor frames in Unreal Engine. The library must be loaded
//  explicity with LoadLibraryA in order to access the definition of CreateResearchModeSensorDevice.
//  Currently only opens the stream of the LEFT_FRONT sensor. 
//
//**********************************************************************************

#include "ResearchModeBase.h"

#if defined(__aarch64__) || defined(_M_ARM64)
extern "C"
HMODULE LoadLibraryA(
	LPCSTR lpLibFileName
);
#endif

DEFINE_LOG_CATEGORY(ResearchModeBase)


// Sets default values
AResearchModeBase::AResearchModeBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Static Mesh Component and attach Cube to camera
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	UStaticMesh* cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object;
	StaticMeshComponent->SetStaticMesh(cubeMesh);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set camera to as root
	this->SetRootComponent(StaticMeshComponent);

}

// Called when the game starts or when spawned
void AResearchModeBase::BeginPlay()
{
	Super::BeginPlay();

#if defined(__aarch64__) || defined(_M_ARM64)

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

		// Sensor frame read thread, IReserachModeSensor object abstracts all sensor types to allow usage of OpenStream(), CloseStream(), GetFriendlyName(), GetSensorType(), and GetNextBuffer()
		// Reset for each sensor type
		pSensor = nullptr;
		pSensorFrame = nullptr;

		hr = pSensorDevice->GetSensor(
			sensorDescriptor.sensorType,
			&pSensor);

		// Error Check Getting Sensor Types
		if (SUCCEEDED(hr)) {
			UE_LOG(ResearchModeBase, Warning, TEXT("Get Sensor Device Type Successful"));
		}

		// Open and process only specified sensor
		if (pSensor->GetSensorType() == LEFT_FRONT) {
			UE_LOG(ResearchModeBase, Warning, TEXT("Get Left Front Sensor"));

			hr = pSensor->GetSampleBufferSize(&sampleBufferSize);
			hr = pSensor->OpenStream();

			// Error Check Stream Opening
			if (SUCCEEDED(hr)) {
				UE_LOG(ResearchModeBase, Warning, TEXT("Opening Stream Successful"));
			}

			// Keep pSensor to point at Left_front sensor
			break;

		}
	}

#endif
}

// Called every frame
void AResearchModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if defined(__aarch64__) || defined(_M_ARM64)

	// Retrieve next available buffer frame from sensor
	hr = pSensor->GetNextBuffer(&pSensorFrame);

	// Error Check Recieving Buffer frame
	if (SUCCEEDED(hr)) {
		UE_LOG(ResearchModeBase, Warning, TEXT("Get Next Buffer From Frame Successful"));
	}

	// Process Visual-light camera frame
	ResearchModeSensorResolution resolution;
	size_t outBufferCount = 0;
	const BYTE* pImage = nullptr;
	IResearchModeSensorVLCFrame* pVLCFrame = nullptr;

	pSensorFrame->GetResolution(&resolution);
	hr = pSensorFrame->QueryInterface(IID_PPV_ARGS(&pVLCFrame));

	// Implement additional functionality to process sensor frame image if successful here
	if (SUCCEEDED(hr))
	{
		pVLCFrame->GetBuffer(&pImage, &outBufferCount);

		if (!pImage) {
			UE_LOG(ResearchModeBase, Warning, TEXT("Empty Sensor Frame"));
			return;
		}
	}

	// Release current Visual Light Camera frame
	if (pVLCFrame)
	{
		pVLCFrame->Release();
	}

	// Release IResearchModeSensorFrame interface memory.
	if (pSensorFrame) {
		pSensorFrame->Release();
	}

#endif
}

void AResearchModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
