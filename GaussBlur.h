#pragma once

#include "PostProcessingResource.h"

struct GaussBlurCB :public PostProcessingCB
{
	int  BlurCount = 1;
	int  CenterX = 0;
	int  CenterY = 0;
	int  SampleDistance = 100;
	int  SampleStrength = 100;
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
		ID3D12PipelineState* PSO,
		ID3D12Resource* ResourceIn,
		void* CB,
		PostProcessingResource& PPResource,
		UINT Width, UINT Height)override;
};