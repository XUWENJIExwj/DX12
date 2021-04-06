#pragma once

#include "..\\Common\\d3dUtil.h"
#include "..\\Common\\MathHelper.h"
#include "ShadowMap.h"

struct CSMPassCB
{
	float MaxBorderPadding = 0.0f;
	float MinBorderPadding = 0.0f;
	float CascadeBlendArea = 0.005f;
	DirectX::XMFLOAT4 ShadowBias = DirectX::XMFLOAT4(-0.0002f, 0.0002f, 0.0008f, 0.0000f);
	DirectX::XMFLOAT4 ShadowTexScale[3]; // óvëfêî = CascadeNum
	DirectX::XMFLOAT4 ShadowTexOffset[3]; // óvëfêî = CascadeNum
	int PCFBlurSize = 3;
	int PCFBlurForLoopStart = -1;
	int PCFBlurForLoopEnd = 2;
	bool VisualCascade = false;
	bool BlendCascade = true;
	bool CancelJitter = true;
	bool NearFarCorrection = true;
	DirectX::XMFLOAT4 CascadeFrustumsEyeSpaceDepthsFloat; // óvëfêî = CascadeNum
	DirectX::XMFLOAT4 CascadeFrustumsEyeSpaceDepthsFloat4[3]; // óvëfêî = CascadeNum
};

class CCamera;
class CLight;

class CCascadeShadowMap
{
private:
	ID3D12Device*                              m_D3DDevice = nullptr;
	CSMPassCB                                  m_CSMPassCB;
	UINT                                       m_ShadowMapSize = 1024;
	UINT                                       m_CascadeNum;
	std::vector<std::unique_ptr<CShadowMap>>   m_ShadowMap;
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> m_ShadowMapDescHandle;

public:
	CCascadeShadowMap() = delete;
	CCascadeShadowMap(const CCascadeShadowMap& rhs) = delete;
	CCascadeShadowMap(ID3D12Device* Device, UINT CascadeNum);
	~CCascadeShadowMap() = default;

	void UpdateCSMImGui();

	void CreateDescriptors(
		D3D12_CPU_DESCRIPTOR_HANDLE CpuSrvStartHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE GpuSrvStartHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE CpuDsvStartHandle,
		UINT Offset, UINT CbvSrvUavDescSize, UINT DsvDescSize);

	void CreateSceneAABBPoints(DirectX::XMVECTOR* SceneAABBPoints, const DirectX::BoundingBox* SceneBoundingBox);

	void XM_CALLCONV ComputeFitCascadeCSMPassCB(
		const DirectX::FXMMATRIX& CameraInvView, const DirectX::BoundingBox* SceneBoundingBox,
		CCamera* Camera, CLight* Light);

	void XM_CALLCONV ComputeNearAndFarInCSM(float& Near, float& Far,
		DirectX::FXMVECTOR LightOrthographicMin, DirectX::FXMVECTOR LightOrthographicMax, DirectX::XMVECTOR* SceneAABBPointsLiS);

	CSMPassCB* GetCSMPassCB() { return &m_CSMPassCB; }
	UINT GetShadowMapSize() { return m_ShadowMapSize; }
	UINT GetCascadeNum() { return m_CascadeNum; }
	CShadowMap* GetShadowMap(int CascadeIndex) { return m_ShadowMap[CascadeIndex].get(); }
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapDescHandle(int CascadeIndex) { return m_ShadowMapDescHandle[CascadeIndex]; }
};