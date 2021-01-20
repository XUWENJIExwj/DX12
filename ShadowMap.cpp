#include "ShadowMap.h"
 
CShadowMap::CShadowMap(ID3D12Device* Device, UINT Width, UINT Height, UINT CascadNum)
{
	m_D3dDevice = Device;

	m_Width = Width;
	m_Height = Height;

	m_Viewports.resize(CascadNum);
	m_ScissorRects.resize(CascadNum);
	
	float start, end;
	float width = (float)Width / CascadNum;

	for (UINT i = 0; i < CascadNum; ++i)
	{
		start = i * width;
		end = start + width;
		m_Viewports[i] = { start, 0.0f, end, (float)Height, 0.0f, 1.0f };
		m_ScissorRects[i] = { (int)start, 0, (int)end, (int)Height };
	}

	CreateResource();
}

void CShadowMap::CreateDescriptors(
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuDsvHandle)
{
	// Save references to the descriptors. 
	m_CpuSrvHandle = CpuSrvHandle;
	m_GpuSrvHandle = GpuSrvHandle;
    m_CpuDsvHandle = CpuDsvHandle;

	//  Create the descriptors
	CreateDescriptors();
}

void CShadowMap::OnResize(UINT NewWidth, UINT NewHeight)
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
 
void CShadowMap::CreateDescriptors()
{
    // Create SRV to resource so we can sample the shadow map in a shader program.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; 
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    srvDesc.Texture2D.PlaneSlice = 0;
    m_D3dDevice->CreateShaderResourceView(m_ShadowMap.Get(), &srvDesc, m_CpuSrvHandle);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc; 
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.Texture2D.MipSlice = 0;
	m_D3dDevice->CreateDepthStencilView(m_ShadowMap.Get(), &dsvDesc, m_CpuDsvHandle);
}

void CShadowMap::CreateResource()
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
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_D3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_ShadowMap)));
}