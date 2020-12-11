#include "Manager.h"
#include "Renderer.h"
#include "Scene.h"

using namespace std;

vector<unique_ptr<FrameResource>> CFrameResourceManager::m_FrameResources;

FrameResource* CFrameResourceManager::m_CurrentFrameResource = nullptr;
int            CFrameResourceManager::m_CurrentFrameResourceIndex = 0;

UINT CFrameResourceManager::m_ObjectCBCount = 0;
UINT CFrameResourceManager::m_ObjCBByteSize;
UINT CFrameResourceManager::m_PassCBByteSize;

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

    PassCB = make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	MaterialBuffer = make_unique<UploadBuffer<MaterialData>>(device, materialCount, false);
    ObjectCB = make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
}

void CFrameResourceManager::ComputeConstantBufferSize()
{
	m_ObjCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	m_PassCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
}

bool CFrameResourceManager::CreateFrameResources()
{
	UINT objCBCount = CManager::GetScene()->GetAllGameObjectsCount();

	// Camera‚à‚È‚¢‚±‚Æ‚É‚È‚é‚Ì‚ÅAƒGƒ‰[‚É‚µ‚Ä‚¨‚­
	assert(objCBCount > 0);

	if (m_ObjectCBCount < objCBCount)
	{
		m_ObjectCBCount = objCBCount;

		for (int i = 0; i < gNumFrameResources; ++i)
		{
			m_FrameResources.push_back(make_unique<FrameResource>(CRenderer::GetDevice(),
				CRenderer::GetDynamicCubeOn() ? 1 + CTextureManager::GetDynamicTextureNum() * 6 : 1, m_ObjectCBCount, CMaterialManager::GetMaterialsCount()));
		}
		return true;
	}
	return false;
}

void CFrameResourceManager::CycleFrameResources()
{
	// Cycle through the circular frame resource array.
	m_CurrentFrameResourceIndex = (m_CurrentFrameResourceIndex + 1) % gNumFrameResources;
	m_CurrentFrameResource = m_FrameResources[m_CurrentFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (m_CurrentFrameResource->Fence != 0 && CRenderer::GetFence()->GetCompletedValue() < m_CurrentFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(CRenderer::GetFence()->SetEventOnCompletion(m_CurrentFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}