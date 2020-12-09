#include "CubeRenderTarget.h"
 
CCubeRenderTarget::CCubeRenderTarget(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format)
{
	m_D3DDevice = Device;

	m_Width = Width;
	m_Height = Height;
	m_Format = Format;

	m_Viewport = { 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f };
	m_ScissorRect = { 0, 0, (int)Width, (int)Height };

	CreateResource();
}

ID3D12Resource*  CCubeRenderTarget::GetResource()
{
	return m_CubeMapResource.Get();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE CCubeRenderTarget::GetSrv()
{
	return m_GpuSrvHandle;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE CCubeRenderTarget::GetRtv(int FaceIndex)
{
	return m_CpuRtvHandle[FaceIndex];
}

D3D12_VIEWPORT CCubeRenderTarget::GetViewport()const
{
	return m_Viewport;
}

D3D12_RECT CCubeRenderTarget::GetScissorRect()const
{
	return m_ScissorRect;
}

void CCubeRenderTarget::CreateDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
	                                CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
	                                CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle[6])
{
	// Save references to the descriptors. 
	m_CpuSrvHandle = CpuSrvHandle;
	m_GpuSrvHandle = GpuSrvHandle;

	for(int i = 0; i < 6; ++i)
		m_CpuRtvHandle[i] = CpuRtvHandle[i];

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
 
void CCubeRenderTarget::CreateDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	// Create SRV to the entire cubemap resource.
	m_D3DDevice->CreateShaderResourceView(m_CubeMapResource.Get(), &srvDesc, m_CpuSrvHandle);

	// Create RTV to each cube face.
	for(int i = 0; i < 6; ++i)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc; 
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Format = m_Format;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;

		// Render target to ith element.
		rtvDesc.Texture2DArray.FirstArraySlice = i;

		// Only view one element of the array.
		rtvDesc.Texture2DArray.ArraySize = 1;

		// Create RTV to ith cubemap face.
		m_D3DDevice->CreateRenderTargetView(m_CubeMapResource.Get(), &rtvDesc, m_CpuRtvHandle[i]);
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

	ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_CubeMapResource)));
}