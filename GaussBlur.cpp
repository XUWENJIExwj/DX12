#include "GaussBlur.h"

enum class GaussBlurPSO :int
{
	HorizontalBlur,
	VerticalBlur,
};

using namespace std;

void CGaussBlur::Execute(
	ID3D12GraphicsCommandList* CommandList, ID3D12RootSignature* RootSignature,
	ID3D12Resource* ResourceIn, void* CB, PostProcessingResource& PPResource,
	UINT Width, UINT Height, const vector<ID3D12PipelineState*>& PSOs)
{
	GaussBlurCB* gbCB = (GaussBlurCB*)CB;
	if (gbCB->EffectOn)
	{
		auto weights = ComputeGaussWeights(gbCB->MaxBlurRadius, gbCB->Sigma);
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

		UINT numGroupsX = (UINT)ceilf(Width / 256.0f);
		UINT numGroupsY = (UINT)ceilf(Height / 256.0f);
		for (int i = 0; i < gbCB->BlurCount; ++i)
		{
			// Horizontal Blur Pass
			CommandList->SetPipelineState(PSOs[(int)GaussBlurPSO::HorizontalBlur]);
			CommandList->SetComputeRootDescriptorTable(1, PPResource.GpuSrvHandleA);
			CommandList->SetComputeRootDescriptorTable(3, PPResource.GpuUavHandleB);

			CommandList->Dispatch(numGroupsX, Height, 1);

			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

			// Vertical Blur Pass
			CommandList->SetPipelineState(PSOs[(int)GaussBlurPSO::VerticalBlur]);
			CommandList->SetComputeRootDescriptorTable(1, PPResource.GpuSrvHandleB);
			CommandList->SetComputeRootDescriptorTable(3, PPResource.GpuUavHandleA);

			CommandList->Dispatch(Width, numGroupsY, 1);
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
			CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		}

		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
		CommandList->CopyResource(ResourceIn, PPResource.FullA.Get());
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullA.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(PPResource.FullB.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON));
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
}

vector<float> CGaussBlur::ComputeGaussWeights(int MaxBlurRadius, float Sigma)
{
	// G(x) = exp(-x^2 / 2 * É–^2) (2 * É–^2)Å®twoSigma2
	float twoSigma2 = 2.0f * Sigma * Sigma;
	int blurRadius = (int)ceilf(2.0f * Sigma);
	assert(blurRadius <= MaxBlurRadius);

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