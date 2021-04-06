#pragma once
#include "..\\Common\\d3dUtil.h"

struct PostProcessingResource
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandleA;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandleB;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandleC;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuUavHandleA;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuUavHandleB;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuUavHandleC;

	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandleA;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandleB;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandleC;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuUavHandleA;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuUavHandleB;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuUavHandleC;

	Microsoft::WRL::ComPtr<ID3D12Resource> FullA;
	Microsoft::WRL::ComPtr<ID3D12Resource> FullB;
	Microsoft::WRL::ComPtr<ID3D12Resource> FullC;
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
		ID3D12Resource* ResourceIn,
		void* CB,
		PostProcessingResource& PPResource,
		UINT Width, UINT Height,
		const std::vector<ID3D12PipelineState*>& PSOs) = 0;
};