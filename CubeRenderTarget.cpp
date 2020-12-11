#include "CubeRenderTarget.h"
#include "TextureManager.h"
 
CCubeRenderTarget::CCubeRenderTarget(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format)
{
	m_D3DDevice = Device;

	m_Width = Width;
	m_Height = Height;
	m_Format = Format;

	m_Viewport = { 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f };
	m_ScissorRect = { 0, 0, (int)Width, (int)Height };

	//CreateResource();
}

// DCM
//void CCubeRenderTarget::CreateDescriptors(
//	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
//	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
//	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle[6])
//{
//	// Save references to the descriptors.
//	m_CpuSrvHandle = CpuSrvHandle;
//	m_GpuSrvHandle = GpuSrvHandle;
//
//	for (int i = 0; i < 6; ++i)
//	{
//		m_CpuRtvHandle[i] = CpuRtvHandle[i];
//	}
//
//	CreateResource();
//
//	//  Create the descriptors
//	CreateDescriptors();
//}

void CCubeRenderTarget::CreateDescriptors(
	D3D12_CPU_DESCRIPTOR_HANDLE SrvCPUStartHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE SrvGPUStartHandle,
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle[6],
	UINT Offset, UINT CbvSrvUavDescriptorSize)
{
	// Save references to the descriptors.
	int SrvHandlesNum = (int)CTextureManager::GetDynamicTextureNum();
	m_CpuSrvHandles.resize(SrvHandlesNum);
	m_GpuSrvHandles.resize(SrvHandlesNum);

	for (int i = 0; i < SrvHandlesNum; ++i)
	{
		m_CpuSrvHandles[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvCPUStartHandle, Offset + i, CbvSrvUavDescriptorSize);
		m_GpuSrvHandles[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvGPUStartHandle, Offset + i, CbvSrvUavDescriptorSize);
	}

	for (int i = 0; i < 6; ++i)
	{
		m_CpuRtvHandle[i] = CpuRtvHandle[i];
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

void CCubeRenderTarget::CreateRtvToEachCubeFace(int DCMResourcesIndex, int FaceIndex)
{
	//for (int i = 0; i < 6; ++i)
	{
		UpdateRtvDesc(FaceIndex);
		m_D3DDevice->CreateRenderTargetView(m_CubeMapResources[DCMResourcesIndex].Get(), &m_RtvDesc, m_CpuRtvHandle[FaceIndex]);
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

	// DCM
	//ThrowIfFailed(m_D3DDevice->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//	D3D12_HEAP_FLAG_NONE,
	//	&texDesc,
	//	D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr,
	//	IID_PPV_ARGS(&m_CubeMapResource)));

	m_CubeMapResources.resize(CTextureManager::GetDynamicTextureNum());

	for (int i = 0; i < m_CubeMapResources.size(); ++i)
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

	// Create SRV to the entire cubemap resource.
	// DCM
	//m_D3DDevice->CreateShaderResourceView(m_CubeMapResource.Get(), &srvDesc, m_CpuSrvHandle);

	for (int i = 0; i < m_CubeMapResources.size(); ++i)
	{
		m_D3DDevice->CreateShaderResourceView(m_CubeMapResources[i].Get(), &srvDesc, m_CpuSrvHandles[i]);
	}

	CreateRtvDesc();

	// Create RTV to each cube face.
	//for(int i = 0; i < 6; ++i)
	//{
	//	UpdateRtvDesc(i);

	//	// Create RTV to ith cubemap face.
	//	// DCM
	//	//m_D3DDevice->CreateRenderTargetView(m_CubeMapResource.Get(), &rtvDesc, m_CpuRtvHandle[i]);

	//	// Bug‚ÌŒ´ˆö:Rtv‚ªã‘‚«‚³‚ê‚é
	//	//for (int j = (int)m_CubeMapResources.size() - 1; j >= 0; --j)
	//	for (int j = 0; j < (int)m_CubeMapResources.size(); ++j)
	//	{
	//		m_D3DDevice->CreateRenderTargetView(m_CubeMapResources[j].Get(), &m_RtvDesc, m_CpuRtvHandle[i]);
	//	}
	//}
}

void CCubeRenderTarget::CreateRtvDesc()
{
	m_RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	m_RtvDesc.Format = m_Format;
	m_RtvDesc.Texture2DArray.MipSlice = 0;
	m_RtvDesc.Texture2DArray.PlaneSlice = 0;

	// Only view one element of the array.
	m_RtvDesc.Texture2DArray.ArraySize = 1;
}

void CCubeRenderTarget::UpdateRtvDesc(int FaceIndex)
{
	// Render target to FaceIndexth element.
	m_RtvDesc.Texture2DArray.FirstArraySlice = FaceIndex;
}