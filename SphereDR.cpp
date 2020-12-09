#include "Manager.h"
#include "Renderer.h"
#include "Scene.h"
#include "SphereDR.h"

using namespace DirectX;

void CSphereDR::Init()
{
	m_Position = XMFLOAT3(3.0f, 0.0f, 0.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterial((int)MaterialIndex::Material_02_Mirror);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_00_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["sphere"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["sphere"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["sphere"].BaseVertexLocation;

	//CManager::GetScene()->SetUpDynamicCubeMapCamera(m_Position);
}

void CSphereDR::Update(const GameTimer& GlobalTimer)
{
	GlobalRotate(GlobalTimer);
}

void CSphereDR::Draw(const GameTimer& GlobalTimer)
{
	CreateDynamicCubeMapResources(GlobalTimer);
	CRenderer::SetUpBeforeDrawScene();
	CRenderer::SetUpDynamicCubeMapResources();
	CRenderer::DrawSingleGameObject(this, CFrameResourceManager::GetCurrentFrameResource()->ObjectCB->Resource());
	CRenderer::SetUpCubeMapResources();
}

void CSphereDR::GlobalRotate(const GameTimer& GlobalTimer)
{
	XMMATRIX rot = XMMatrixRotationY(0.5f * GlobalTimer.DeltaTime());

	XMStoreFloat4x4(&m_World, XMLoadFloat4x4(&m_World) * rot);

	m_Position.x = m_World(3, 0);
	m_Position.y = m_World(3, 1);
	m_Position.z = m_World(3, 2);

	m_NumFramesDirty = gNumFrameResources;

	CManager::GetScene()->SetUpDynamicCubeMapCamera(m_Position);
}

void CSphereDR::CreateDynamicCubeMapResources(const GameTimer& GlobalTimer)
{
	CManager::GetScene()->SetUpDynamicCubeMapCamera(m_Position);
	CManager::GetScene()->UpdateDynamicCubeMapPassCB(GlobalTimer);

	auto dcmCameras = CManager::GetScene()->GetDynamicCubeMapCameras();
	auto gameObjectsLayer = CManager::GetScene()->GetAllGameObjectsWithLayer();

	// CreateDynamicCubeMapResource
	CRenderer::SetUpBeforeCreateAllDynamicCubeMapResources();

	for (int i = 0; i < dcmCameras.size(); ++i)
	{
		CRenderer::SetUpBeforeCreateEachDynamicCubeMapResource(i);
		CRenderer::DrawGameObjectsWithLayer(gameObjectsLayer[(int)GameObjectsLayer::Layer_3D_Opaque]);
		CRenderer::SetPSO((int)PSOTypeIndex::PSO_02_Solid_Sky);
		CRenderer::DrawGameObjectsWithLayer(gameObjectsLayer[(int)GameObjectsLayer::Layer_3D_Sky]);
		CRenderer::SetPSO((int)PSOTypeIndex::PSO_00_Solid_Opaque);
	}
	CRenderer::CompleteCreateDynamicCubeMapResources();
}