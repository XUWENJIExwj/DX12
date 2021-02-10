#pragma once

#include "PostProcessingResource.h"

struct GaussBlurCB :public PostProcessingCB
{
	int   BlurCount = 4;
	int   MaxBlurRadius = 5;
	float Sigma = 2.5f;
};

class CGaussBlur :public CPostProcessingExecution
{
public:
	CGaussBlur() = default;
	CGaussBlur(const CGaussBlur& rhs) = delete;
	CGaussBlur& operator=(const CGaussBlur& rhs) = delete;
	~CGaussBlur() = default;

	void Execute(
		ID3D12GraphicsCommandList* CommandList,
		ID3D12RootSignature* RootSignature,
		ID3D12Resource* ResourceIn,
		void* CB,
		PostProcessingResource& PPResource,
		UINT Width, UINT Height,
		const std::vector<ID3D12PipelineState*>& PSOs)override;

protected:
	std::vector<float> ComputeGaussWeights(int MaxBlurRadius, float Sigma);
};