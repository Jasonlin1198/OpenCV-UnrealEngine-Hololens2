//**********************************************************************************
//
//  This Unreal Engine Actor handles the C++ implementation of accessing and rendering
//  the Hololens 2 Photo/Video camera through the texture of a plane. This is done by
//  creating a dynamic material and fetching the AR Texture object captured by the
//  detected camera device. The texture is then assigned to the dynamic material of the
//  static mesh component of the actor, which is scaled to the appropriate camera intrinsic
//  resolutions. 
//
//  Camera Intrinsic resolutions values were tested and found to be:
//  Intrinsics.ImageResolution.X => 1504.0f
//  Intrinsics.ImageResolution.Y => 846.0f
// 
//  Approaches taken to retrieve pixel data from UARTexture object:
// 
//  1. UARTexture derives from UTexture which allows accessibility to the parent class
//     functions. The issue that arises is that when calling the below function, the
//     returned double pointer is in fact a nullptr and is unable to access the Mips data. 
//     
//     FTexturePlatformData** platData = ARTexture->GetRunningPlatformData();
// 
//  2. Obtain the FRHITexture2D from UARTexture resource property. Use the
//     GetNativeResource() function to obtain a texture back buffer data pointer.
//     Cast the pointer to a ID3D11Texture2D which is a DirectX11 (imported from "d3d11.h")
//     texture and attempt to convert to a readable pixel type. Development stopped when
//     trying Mapping resource from CPU to GPU was unsuccessful. 
//     
//
//**********************************************************************************

#include "ARCameraRenderer.h"

DEFINE_LOG_CATEGORY(ARCameraRenderer)

// Sets default values
AARCameraRenderer::AARCameraRenderer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    // Load a mesh from the engine to render the camera feed to.
    StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EngineMeshes/Cube.Cube"), nullptr, LOAD_None, nullptr);

    // Create a static mesh component to render the static mesh
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CameraPlane"));
    StaticMeshComponent->SetStaticMesh(StaticMesh);

    // Scale and add to the scene
    StaticMeshComponent->SetWorldScale3D(FVector(0.1f, 1, 1));
    this->SetRootComponent(StaticMeshComponent);
}

// Called when the game starts or when spawned
void AARCameraRenderer::BeginPlay()
{
    
	Super::BeginPlay();
    // Create a dynamic material instance from the game's camera material.
    // Right-click on a material in the project and select "Copy Reference" to get this string.
    FString CameraMatPath("Material'/Game/ARCameraRendererC++/M_ARCameraTextureC++.M_ARCameraTextureC++'");
    UMaterial* BaseMaterial = (UMaterial*)StaticLoadObject(UMaterial::StaticClass(), nullptr, *CameraMatPath, nullptr, LOAD_None, nullptr);
    DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);

    // Use the dynamic material instance when rendering the camera mesh.
    StaticMeshComponent->SetMaterial(0, DynamicMaterial);

    // Start the webcam.
    UARBlueprintLibrary::ToggleARCapture(true, EARCaptureType::Camera);
}

// Called every frame
void AARCameraRenderer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Dynamic material instance only needs to be set once.
    if (IsTextureParamSet)
    {
        return;
    }
    
    // Get the texture from the camera.
    UARTexture* ARTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::CameraImage);
    if (ARTexture != nullptr)
    {

        //UTexture* parent = ARTexture;

        //FTexturePlatformData** platData = ARTexture->GetRunningPlatformData();
        //if (platData == nullptr)
        //    UE_LOG(LogTemp, Warning, TEXT("FTexturePlatformData Null"));

        //FTexturePlatformData** platDataParent = parent->GetRunningPlatformData();
        //if (platDataParent == nullptr)
        //    UE_LOG(LogTemp, Warning, TEXT("FTexturePlatformData Casted Null"));

        //FString name = ARTexture->GetName();
        //UE_LOG(LogTemp, Warning, TEXT("Texture name is : %s"), *name); //OpenXRCameraImageTexture_2147482531

        // Set the shader's texture parameter (named "Param") to the camera image.
        DynamicMaterial->SetTextureParameterValue("Param", ARTexture);
        IsTextureParamSet = true;

        // Get the camera instrincs
        FARCameraIntrinsics Intrinsics;
        UARBlueprintLibrary::GetCameraIntrinsics(Intrinsics);

        // Scale the camera mesh by the aspect ratio.
        float R = (float)Intrinsics.ImageResolution.X / (float)Intrinsics.ImageResolution.Y;
        StaticMeshComponent->SetWorldScale3D(FVector(0.1f, R, 1));



        //UE_LOG(LogTemp, Warning, TEXT("Attempt to access UARTexture Resource"));
        //FTextureResource* Resource = ARTexture->Resource;
        //if (Resource == nullptr) {
            //UE_LOG(LogTemp, Warning, TEXT("Resource Null"));
        //}


        //FRHITexture2D* rhi = Resource->GetTexture2DRHI();
        //if (rhi == nullptr) {
        //    UE_LOG(LogTemp, Warning, TEXT("FRHITexture2D Null"));
        //}
        //else {

        //    UE_LOG(LogTemp, Warning, TEXT("AFTER RHI"));

        //    uint32 x = rhi->GetSizeX();
        //    uint32 y = rhi->GetSizeY();
        //    UE_LOG(LogTemp, Warning, TEXT("GetSizeX : %d"), x); //1504
        //    UE_LOG(LogTemp, Warning, TEXT("GetSizeY : %d"), y); //806

        //    FRHITexture2D* rhicasted = rhi->GetTexture2D();
        //    if (rhicasted == nullptr) {
        //        UE_LOG(LogTemp, Warning, TEXT("Failed GetTexture2D"));
        //    }
        //    else {
        //        UE_LOG(LogTemp, Warning, TEXT("Success GetTexture2D"));
        //    }

        //    void* data = rhi->GetNativeResource();
        //    if (data == nullptr) {
        //        UE_LOG(LogTemp, Warning, TEXT("Failed GetNativeResource()"));
        //    }
        //    else {
        //        UE_LOG(LogTemp, Warning, TEXT("Success GetNativeResource()"));
        //        ID3D11Texture2D* Texture = static_cast<ID3D11Texture2D*>(data);
        //        if (Texture == nullptr) {
        //            UE_LOG(LogTemp, Warning, TEXT("Failed D3D11Texture2D"));
        //        }
        //        else {
        //            UE_LOG(LogTemp, Warning, TEXT("Success D3D11Texture2D"));

        //            // https://github.com/Microsoft/graphics-driver-samples/blob/master/render-only-sample/rostest/util.cpp#L244
        //            // First verify that we can map the texture
        //            D3D11_TEXTURE2D_DESC desc;
        //            Texture->GetDesc(&desc);
        //            UE_LOG(LogTemp, Warning, TEXT("DESCRIPTION Width %d  value"), desc.Width); //1504
        //            UE_LOG(LogTemp, Warning, TEXT("DESCRIPTION Height %d value"), desc.Height); //846
        //            UE_LOG(LogTemp, Warning, TEXT("DESCRIPTION CPU %d value"), desc.CPUAccessFlags); //0

        //            // Get the device context
        //            ID3D11Device* d3dDevice;
        //            Texture->GetDevice(&d3dDevice);
        //            ID3D11DeviceContext* d3dContext;
        //            d3dDevice->GetImmediateContext(&d3dContext);

        //            // map the texture
        //            //ID3D11Texture2D* mappedTexture;
        //            D3D11_MAPPED_SUBRESOURCE mapInfo;
        //            mapInfo.RowPitch;
        //            HRESULT hr = d3dContext->Map(Texture,0, D3D11_MAP_READ_WRITE,0, &mapInfo);

        //            if (SUCCEEDED(hr)) {
        //                UE_LOG(LogTemp, Warning, TEXT("Success Mapping"));
        //            }
        //            else {
        //                UE_LOG(LogTemp, Warning, TEXT("Failed MAPPINGS"));
        //            }
        //            if (mapInfo.pData == nullptr) {
        //                UE_LOG(LogTemp, Warning, TEXT("Null data pointer"));
        //            }
        //            else {
        //                UE_LOG(LogTemp, Warning, TEXT("Non-null data pointer"));
        //            }



        //            ////Variable Declaration
        //            //ID3D11Texture2D* lDestImage = static_cast<ID3D11Texture2D*>(data);
        //            //ID3D11DeviceContext* lImmediateContext;
        //
        //            //ID3D11Device* d3dDevice;
        //            //Texture->GetDevice(&d3dDevice);
        //            //d3dDevice->GetImmediateContext(&lImmediateContext);
        //
        //            //// Copy image into GDI drawing texture
        //            //lImmediateContext->CopyResource(lDestImage, Texture);
        //            //Texture->Release();
        //
        //            //// Copy GPU Resource to CPU
        //            //D3D11_TEXTURE2D_DESC description;
        //            //lDestImage->GetDesc(&description);
        //            //D3D11_MAPPED_SUBRESOURCE resource;
        //            //UINT subresource = D3D11CalcSubresource(0, 0, 0);
        //            //HRESULT hr = lImmediateContext->Map(lDestImage, subresource, D3D11_MAP_READ_WRITE, 0, &resource);
        //
        //            //std::unique_ptr<BYTE> pBuf(new BYTE[resource.RowPitch * description.Height]);
        //        }
        //    }
        //}


        //FTexture2DMipMap mip = data->Mips[0];
        //FByteBulkData RawImageData = mip.BulkData;
        //const FColor* FormatedImageData = reinterpret_cast<const FColor*>(RawImageData.Lock(LOCK_READ_ONLY));

        //int X = 0;
        //int Y = 0;

        //if (FormatedImageData == nullptr) {
        //    UE_LOG(LogTemp, Warning, TEXT("Failed FCOLOR"));
        //}
        //else {
        //    UE_LOG(LogTemp, Warning, TEXT("Success FCOLOR"));
        //}
        //FColor c = FormatedImageData[Y * width + X]

    }
}

