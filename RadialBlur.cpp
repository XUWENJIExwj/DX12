#include "RadialBlur.h"

using namespace std;

void CRadialBlur::Execute(ID3D12GraphicsCommandList* CommandList, ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO, ID3D12Resource* ResourceIn, void* CB, PostProcessingResource& PPResource, UINT Width, UINT Height)
{
	RadialBlurCB* rbCB = (RadialBlurCB*)CB;
	if (rbCB->EffectOn)
	{
		CommandList->SetComputeRootSignature(RootSignature);

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.RA.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->CopyResource(PPResource.RA.Get(), ResourceIn);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.RA.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.RB.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		UINT numGroupsX = (UINT)ceilf(Width / 16.0f);
		UINT numGroupsY = (UINT)ceilf(Height / 16.0f);

		CommandList->SetPipelineState(PSO);

		vector<int> cb =
		{
			(int)Width, (int)Height,
			rbCB->CenterX, rbCB->CenterY,
			rbCB->SampleDistance, rbCB->SampleStrength
		};
		CommandList->SetComputeRoot32BitConstants(0, (UINT)cb.size(), cb.data(), 0);

		ID3D12Resource* workResources[2] = { PPResource.RA.Get(), PPResource.RB.Get() };
		D3D12_GPU_DESCRIPTOR_HANDLE workSrvHandls[2] = { PPResource.GpuSrvHandleA, PPResource.GpuSrvHandleB };
		D3D12_GPU_DESCRIPTOR_HANDLE workUavHandls[2] = { PPResource.GpuUavHandleA, PPResource.GpuUavHandleB };
		int currentInput, currentOutput;
		for (int i = 0; i < rbCB->BlurCount; ++i)
		{
			currentInput = i % 2;
			currentOutput = (i + 1) % 2;
			CommandList->SetComputeRootDescriptorTable(1, workSrvHandls[currentInput]);
			CommandList->SetComputeRootDescriptorTable(2, workUavHandls[currentOutput]);

			CommandList->Dispatch(numGroupsX, numGroupsY, 1);

			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(workResources[currentInput],
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(workResources[currentOutput],
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
		}

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(workResources[currentInput],
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(workResources[currentOutput],
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->CopyResource(ResourceIn, workResources[currentOutput]);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(workResources[currentOutput],
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
}