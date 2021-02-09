#include "GaussBlur.h"

using namespace std;

const int gMaxBlurRadius = 5;

void CGaussBlur::Execute(
	ID3D12GraphicsCommandList* CommandList, ID3D12RootSignature* RootSignature,
	ID3D12Resource* ResourceIn, void* CB, PostProcessingResource& PPResource,
	UINT Width, UINT Height, ID3D12PipelineState* PSOA, ID3D12PipelineState* PSOB)
{
	GaussBlurCB* gbCB = (GaussBlurCB*)CB;
	if (gbCB->EffectOn)
	{
		auto weights = ComputeGaussWeights(gbCB->Sigma);
		int blurRadius = (int)weights.size() / 2;
		CommandList->SetComputeRootSignature(RootSignature);
		CommandList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
		CommandList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);
		
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->CopyResource(PPResource.FullA.Get(), ResourceIn);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		for (int i = 0; i < gbCB->BlurCount; ++i)
		{
			// Horizontal Blur Pass
			CommandList->SetPipelineState(PSOA);
			CommandList->SetComputeRootDescriptorTable(1, PPResource.GpuSrvHandleA);
			CommandList->SetComputeRootDescriptorTable(2, PPResource.GpuUavHandleB);

			UINT numGroupsX = (UINT)ceilf(Width / 256.0f);
			CommandList->Dispatch(numGroupsX, Height, 1);

			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

			// Vertical Blur Pass
			CommandList->SetPipelineState(PSOB);
			CommandList->SetComputeRootDescriptorTable(1, PPResource.GpuSrvHandleB);
			CommandList->SetComputeRootDescriptorTable(2, PPResource.GpuUavHandleA);

			UINT numGroupsY = (UINT)ceilf(Height / 256.0f);
			CommandList->Dispatch(Width, numGroupsY, 1);
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		}

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->CopyResource(ResourceIn, PPResource.FullA.Get());
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
}

vector<float> CGaussBlur::ComputeGaussWeights(float Sigma)
{
	// G(x) = exp(-x^2 / 2 * É–^2) (2 * É–^2)Å®twoSigma2
	float twoSigma2 = 2.0f * Sigma * Sigma;
	int blurRadius = (int)ceilf(2.0f * Sigma);
	assert(blurRadius <= gMaxBlurRadius);

	vector<float> weights;
	weights.resize(2 * blurRadius + 1);
	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;
		weights[i + blurRadius] = expf(-x * x / twoSigma2);
		weightSum += weights[i + blurRadius];
	}

	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}
	return weights;
}