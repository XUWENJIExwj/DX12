#include "DX12App.h"
#include "Manager.h"
//#include "Scene.h"
#include "Renderer.h"
#include "FrameResource.h"

using namespace std;

vector<unique_ptr<FrameResource>> m_FrameResources;

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

    PassCB = make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	MaterialBuffer = make_unique<UploadBuffer<MaterialData>>(device, materialCount, false);
    ObjectCB = make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
}

FrameResource::~FrameResource()
{

}

void CFrameResourceManager::Init()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		//m_FrameResources.push_back(make_unique<FrameResource>(CRenderer::GetDevice(),
		//	CRenderer::GetDynamicCubeOn() ? 7 : 1, 1, 1));

		//m_FrameResources[i] = make_unique<FrameResource>(CRenderer::GetDevice(),
		//	7, 1, 1);

		//CRenderer::GetDynamicCubeOn() ? 7 : 1, (UINT)CManager::GetScene()->GetAllGameObjectsCount(), (UINT)mMaterials.size()));
	}
}