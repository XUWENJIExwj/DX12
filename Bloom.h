#pragma once

#include "GaussBlur.h"

struct BloomCB :public GaussBlurCB
{
	float BaseColor = 0.0f;
	float LuminanceThreshold = 0.8f;
	float LuminanceStrength = 1.2f;
};

class CBloom :public CGaussBlur
{
public:
	CBloom() = default;
	CBloom(const CBloom& rhs) = delete;
	CBloom& operator=(const CBloom& rhs) = delete;
	~CBloom() = default;

	void Execute(
		ID3D12GraphicsCommandList* CommandList,
		ID3D12RootSignature* RootSignature,
		ID3D12Resource* ResourceIn,
		void* CB,
		PostProcessingResource& PPResource,
		UINT Width, UINT Height,
		const std::vector<ID3D12PipelineState*>& PSOs)override;
};