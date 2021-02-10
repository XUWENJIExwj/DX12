#include "Bloom.h"

enum class BloomPSO :int
{
	HorizontalBlur,
	VerticalBlur,
	LuminanceMap,
	BlendOpAdd,
};

using namespace std;

void CBloom::Execute(
	ID3D12GraphicsCommandList* CommandList, ID3D12RootSignature* RootSignature,
	ID3D12Resource* ResourceIn, void* CB, PostProcessingResource& PPResource,
	UINT Width, UINT Height, const vector<ID3D12PipelineState*>& PSOs)
{
	BloomCB* bCB = (BloomCB*)CB;
	if (bCB->EffectOn)
	{
		CommandList->SetComputeRootSignature(RootSignature);

		// CreateLuminanceMap
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->CopyResource(PPResource.FullA.Get(), ResourceIn);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		CommandList->SetPipelineState(PSOs[(int)BloomPSO::LuminanceMap]);
		CommandList->SetComputeRoot32BitConstants(0, 1, &bCB->LuminanceThreshold, 0);
		CommandList->SetComputeRoot32BitConstants(0, 1, &bCB->LuminanceStrength, 1);
		CommandList->SetComputeRootDescriptorTable(1, PPResource.GpuSrvHandleA);
		CommandList->SetComputeRootDescriptorTable(3, PPResource.GpuUavHandleB);

		UINT numGroupsXA = (UINT)ceilf(Width / 16.0f);
		UINT numGroupsYA = (UINT)ceilf(Height / 16.0f);
		CommandList->Dispatch(numGroupsXA, numGroupsYA, 1);

		// GaussBlur
		auto weights = ComputeGaussWeights(bCB->MaxBlurRadius, bCB->Sigma);
		int blurRadius = (int)weights.size() / 2;
		CommandList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
		CommandList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

		UINT numGroupsXB = (UINT)ceilf(Width / 256.0f);
		UINT numGroupsYB = (UINT)ceilf(Height / 256.0f);
		for (int i = 0; i < bCB->BlurCount; ++i)
		{
			// Horizontal Blur Pass
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

			CommandList->SetPipelineState(PSOs[(int)BloomPSO::HorizontalBlur]);
			CommandList->SetComputeRootDescriptorTable(1, PPResource.GpuSrvHandleB);
			CommandList->SetComputeRootDescriptorTable(3, PPResource.GpuUavHandleA);
			CommandList->Dispatch(numGroupsXB, Height, 1);

			// Vertical Blur Pass
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

			CommandList->SetPipelineState(PSOs[(int)BloomPSO::VerticalBlur]);
			CommandList->SetComputeRootDescriptorTable(1, PPResource.GpuSrvHandleA);
			CommandList->SetComputeRootDescriptorTable(3, PPResource.GpuUavHandleB);
			CommandList->Dispatch(Width, numGroupsYB, 1);
		}

		// Blend ResourceIn(FullC) And LuminanceMap
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullC.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->CopyResource(PPResource.FullC.Get(), ResourceIn);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullC.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		CommandList->SetPipelineState(PSOs[(int)BloomPSO::BlendOpAdd]);
		CommandList->SetComputeRootDescriptorTable(1, PPResource.GpuSrvHandleB);
		CommandList->SetComputeRootDescriptorTable(2, PPResource.GpuSrvHandleC);
		CommandList->SetComputeRootDescriptorTable(3, PPResource.GpuUavHandleA);
		CommandList->Dispatch(numGroupsXA, numGroupsYA, 1);

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->CopyResource(ResourceIn, PPResource.FullA.Get());
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullC.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
}