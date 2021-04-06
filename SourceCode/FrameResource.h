#pragma once

#include "..\\Common\\d3dUtil.h"
#include "..\\Common\\MathHelper.h"
#include "..\\Common\\UploadBuffer.h"

#define CASCADE_NUM 3

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT     MaterialIndex;
	UINT     ObjPad0;
	UINT     ObjPad1;
	UINT     ObjPad2;
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;
    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light Lights[MaxLights];

	// CSM
	DirectX::XMFLOAT4X4 ShadowView = MathHelper::Identity4x4();
	float MaxBorderPadding = 0.0f;
	float MinBorderPadding = 0.0f;
	float CascadeBlendArea = 0.005f;
	float cbPerObjectPad0 = 0.0f;
	DirectX::XMFLOAT4 ShadowBias;
	DirectX::XMFLOAT4 ShadowTexScale[CASCADE_NUM]; // 要素数 = CascadeNum
	DirectX::XMFLOAT4 ShadowTexOffset[CASCADE_NUM]; // 要素数 = CascadeNum
	int PCFBlurForLoopStart = -1;
	int PCFBlurForLoopEnd = 2;
	int cbPerObjectPad2 = 0;
	int cbPerObjectPad3 = 0;
	BOOL VisualCascade = false;
	BOOL BlendCascade = true;
	BOOL cbPerObjectPad5 = false;
	BOOL cbPerObjectPad6 = false;
	DirectX::XMFLOAT4 CascadeFrustumsEyeSpaceDepthsFloat;
	DirectX::XMFLOAT4 CascadeFrustumsEyeSpaceDepthsFloat4[CASCADE_NUM];
};

struct MaterialData
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT4 CubeMapDiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.5f;

	// Used in texture mapping.
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();

	UINT DiffuseMapIndex = 0;
	UINT NormalMapIndex = 0;
	UINT HeightMapIndex = 0;
	int  BitangentSign = 1; // Bitangentの符号（反転用）
	UINT UseACForPOM = 0;
	UINT MaxSampleCount = 256; // HeightMap最大サンプル数
	UINT MinSampleCount = 8;   // HeightMap最小サンプル数
	UINT CascadeDebugIndex = 0;
	float HeightScale = 0.01f;
	float ShadowSoftening = 0.9f;
	float floatPad0 = 0.0f;
	float floatPad1 = 0.0f;
	BOOL ShowSelfShadow = true;
	BOOL boolPad0;
	BOOL boolPad1;
	BOOL boolPad2;
};

struct Vertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT2 TexC;
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct FrameResource
{
public:
	// We cannot reset the allocator until the GPU is done processing the commands.
	// So each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	// We cannot update a cbuffer until the GPU is done processing the commands
	// that reference it.  So each frame needs their own cbuffers.
	std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

	std::unique_ptr<UploadBuffer<MaterialData>> MaterialBuffer = nullptr;

	// Fence value to mark commands up to this fence point.  This lets us
	// check if these frame resources are still in use by the GPU.
	UINT64 Fence = 0;
    
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource() = default;
};

class CFrameResourceManager
{
private:
	static std::vector<std::unique_ptr<FrameResource>> m_FrameResources;

	static FrameResource* m_CurrentFrameResource;
	static int            m_CurrentFrameResourceIndex;

	static UINT m_ObjectCBCount;
	static UINT m_ObjCBByteSize;
	static UINT m_PassCBByteSize;

public:
	static void ComputeConstantBufferSize();
	static bool CreateFrameResources(); // true:再生成した false:再生成していない
	static void CycleFrameResources();

	static FrameResource* GetCurrentFrameResource() { return m_CurrentFrameResource; }
	static UINT GetObjCBByteSize() { return m_ObjCBByteSize; }
	static UINT GetPassCBByteSize() { return m_PassCBByteSize; }
};