#pragma once
#include "Common\\d3dUtil.h"

struct PostProcessingResource
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandleA;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuUavHandleA;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandleB;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuUavHandleB;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandleA;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuUavHandleA;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandleB;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuUavHandleB;

	Microsoft::WRL::ComPtr<ID3D12Resource> RA;
	Microsoft::WRL::ComPtr<ID3D12Resource> RB;
};

struct PostProcessingCB
{
	bool EffectOn = false;
};

class CPostProcessingExecution
{
public:
	CPostProcessingExecution() = default;
	CPostProcessingExecution(const CPostProcessingExecution& rhs) = delete;
	CPostProcessingExecution& operator=(const CPostProcessingExecution& rhs) = delete;
	~CPostProcessingExecution() = default;

	virtual void Execute(
		ID3D12GraphicsCommandList* CommandList,
		ID3D12RootSignature* RootSignature,
		ID3D12PipelineState* PSO,
		ID3D12Resource* ResourceIn,
		void* CB,
		PostProcessingResource& PPResource,
		UINT Width, UINT Height) = 0;
};