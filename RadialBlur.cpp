#include "RadialBlur.h"

CRadialBlur::CRadialBlur(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format)
{
	m_D3DDevice = Device;
	m_Width = Width;
	m_Height = Height;
	m_Format = Format;

	CreateResources();
}

void CRadialBlur::CreateDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle, UINT DescSize)
{
	m_CpuSrvHandleIn = CpuSrvHandle;
	m_CpuUavHandleIn = CpuSrvHandle.Offset(1, DescSize);
	m_CpuSrvHandleOut = CpuSrvHandle.Offset(1, DescSize);
	m_CpuUavHandleOut = CpuSrvHandle.Offset(1, DescSize);

	m_GpuSrvHandleIn = GpuSrvHandle;
	m_GpuUavHandleIn = GpuSrvHandle.Offset(1, DescSize);
	m_GpuSrvHandleOut = GpuSrvHandle.Offset(1, DescSize);
	m_GpuUavHandleOut = GpuSrvHandle.Offset(1, DescSize);

	CreateDescriptors();
}

void CRadialBlur::OnResize(UINT NewWidth, UINT NewHeight)
{
	if ((m_Width != NewWidth) || (m_Height != NewHeight))
	{
		m_Width = NewWidth;
		m_Height = NewHeight;

		CreateResources();

		// New resource, so we need new descriptors to that resource.
		CreateDescriptors();
	}
}

void CRadialBlur::Execute(ID3D12GraphicsCommandList* CommandList, ID3D12RootSignature* RootSignature, ID3D12PipelineState* RadialBlurPSO, ID3D12Resource* ResourceIn)
{
	CommandList->SetComputeRootSignature(RootSignature);

	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ResourceIn,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RadialBlurIn.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	CommandList->CopyResource(m_RadialBlurIn.Get(), ResourceIn);
	CommandList->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(m_RadialBlurIn.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RadialBlurOut.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	CommandList->SetPipelineState(RadialBlurPSO);
	CommandList->SetComputeRootDescriptorTable(1, m_GpuSrvHandleOut);
	CommandList->SetComputeRootDescriptorTable(2, m_GpuUavHandleOut);


}

void CRadialBlur::CreateResources()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = m_Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_RadialBlurIn)));

	ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_RadialBlurOut)));
}

void CRadialBlur::CreateDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	uavDesc.Format = m_Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	m_D3DDevice->CreateShaderResourceView(m_RadialBlurIn.Get(), &srvDesc, m_CpuSrvHandleIn);
	m_D3DDevice->CreateUnorderedAccessView(m_RadialBlurIn.Get(), nullptr, &uavDesc, m_CpuUavHandleIn);

	m_D3DDevice->CreateShaderResourceView(m_RadialBlurOut.Get(), &srvDesc, m_CpuSrvHandleOut);
	m_D3DDevice->CreateUnorderedAccessView(m_RadialBlurOut.Get(), nullptr, &uavDesc, m_CpuUavHandleOut);
}