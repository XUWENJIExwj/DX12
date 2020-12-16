#include "CubeRenderTarget.h"
 
CCubeRenderTarget::CCubeRenderTarget(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format)
{
	m_D3DDevice = Device;

	m_Width = Width;
	m_Height = Height;
	m_Format = Format;

	m_Viewport = { 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f };
	m_ScissorRect = { 0, 0, (int)Width, (int)Height };
}

void CCubeRenderTarget::CreateDescriptors(
	D3D12_CPU_DESCRIPTOR_HANDLE CpuSrvStartHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE GpuSrvStartHandle,
	std::vector<std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE>>& RtvCpuHandles,
	UINT Offset, UINT CbvSrvUavDescriptorSize)
{
	// Save references to the descriptors.
	for (int i = 0; i < (int)RtvCpuHandles.size(); ++i)
	{
		m_CpuSrvHandles.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(CpuSrvStartHandle, Offset + i, CbvSrvUavDescriptorSize));
		m_GpuSrvHandles.push_back(CD3DX12_GPU_DESCRIPTOR_HANDLE(GpuSrvStartHandle, Offset + i, CbvSrvUavDescriptorSize));
		m_CpuRtvHandles.push_back(RtvCpuHandles[i]);
	}

	CreateResource();

	//  Create the descriptors
	CreateDescriptors();
}

void CCubeRenderTarget::OnResize(UINT NewWidth, UINT NewHeight)
{
	if((m_Width != NewWidth) || (m_Height != NewHeight))
	{
		m_Width = NewWidth;
		m_Height = NewHeight;

		CreateResource();

		// New resource, so we need new descriptors to that resource.
		CreateDescriptors();
	}
}

void CCubeRenderTarget::CreateResource()
{
	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.DepthOrArraySize = 6;
	texDesc.MipLevels = 1;
	texDesc.Format = m_Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_CubeMapResources.resize(m_CpuRtvHandles.size());

	for (int i = 0; i < (int)m_CubeMapResources.size(); ++i)
	{
		ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_CubeMapResources[i])));
	}
}
 
void CCubeRenderTarget::CreateDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Format = m_Format;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;

	// Only view one element of the array.
	rtvDesc.Texture2DArray.ArraySize = 1;

	for (int i = 0; i < (int)m_CubeMapResources.size(); ++i)
	{
		// Create SRV to the entire cubemap resource.
		m_D3DDevice->CreateShaderResourceView(m_CubeMapResources[i].Get(), &srvDesc, m_CpuSrvHandles[i]);

		// Create RTV to each cube face.
		for (int j = 0; j < 6; ++j)
		{
			// Render target to FaceIndexth element.
			rtvDesc.Texture2DArray.FirstArraySlice = j;
			m_D3DDevice->CreateRenderTargetView(m_CubeMapResources[i].Get(), &rtvDesc, m_CpuRtvHandles[i][j]);
		}
	}
}