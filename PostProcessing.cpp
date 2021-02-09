#include "PostProcessing.h"

using namespace std;

CPostProcessing::CPostProcessing(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format)
{
	m_D3DDevice = Device;
	m_Width = Width;
	m_Height = Height;
	m_Format = Format;

	CreateResources();
}

void CPostProcessing::CreateDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle, UINT DescSize)
{
	m_PPResource.CpuSrvHandleA = CpuSrvHandle;
	m_PPResource.CpuUavHandleA = CpuSrvHandle.Offset(1, DescSize);
	m_PPResource.CpuSrvHandleB = CpuSrvHandle.Offset(1, DescSize);
	m_PPResource.CpuUavHandleB = CpuSrvHandle.Offset(1, DescSize);

	m_PPResource.GpuSrvHandleA = GpuSrvHandle;
	m_PPResource.GpuUavHandleA = GpuSrvHandle.Offset(1, DescSize);
	m_PPResource.GpuSrvHandleB = GpuSrvHandle.Offset(1, DescSize);
	m_PPResource.GpuUavHandleB = GpuSrvHandle.Offset(1, DescSize);

	CreateDescriptors();
}

void CPostProcessing::CreatePostProcessingExecution(string PPName, CPostProcessingExecution* PPExecution)
{
	unique_ptr<CPostProcessingExecution> ppExecution(PPExecution);
	m_PPExecutions[PPName] = move(ppExecution);
}

void CPostProcessing::OnResize(UINT NewWidth, UINT NewHeight)
{
	if ((m_Width != NewWidth) || (m_Height != NewHeight))
	{
		m_Width = NewWidth;
		m_Height = NewHeight;

		CreateResources();
		CreateDescriptors();
	}
}

void CPostProcessing::Execute(
	ID3D12GraphicsCommandList* CommandList, ID3D12RootSignature* RootSignature,
	ID3D12Resource* ResourceIn, std::string PPName, void* CB,
	ID3D12PipelineState* PSOA, ID3D12PipelineState* PSOB)
{
	m_PPExecutions[PPName]->Execute(CommandList, RootSignature, ResourceIn, CB, m_PPResource, m_Width, m_Height, PSOA, PSOB);
}

void CPostProcessing::CreateResources()
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
		IID_PPV_ARGS(&m_PPResource.FullA)));

	ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_PPResource.FullB)));
}

void CPostProcessing::CreateDescriptors()
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

	m_D3DDevice->CreateShaderResourceView(m_PPResource.FullA.Get(), &srvDesc, m_PPResource.CpuSrvHandleA);
	m_D3DDevice->CreateUnorderedAccessView(m_PPResource.FullA.Get(), nullptr, &uavDesc, m_PPResource.CpuUavHandleA);

	m_D3DDevice->CreateShaderResourceView(m_PPResource.FullB.Get(), &srvDesc, m_PPResource.CpuSrvHandleB);
	m_D3DDevice->CreateUnorderedAccessView(m_PPResource.FullB.Get(), nullptr, &uavDesc, m_PPResource.CpuUavHandleB);
}