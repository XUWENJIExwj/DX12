#include "Game.h"
#include "Manager.h"
#include "Renderer.h"
#include "Camera.h"
#include "CameraFP.h"
#include "CameraDynamicCubeMap.h"
#include "Logo.h"
#include "Sky.h"
#include "MeshField.h"
#include "Cube.h"
#include "Sphere.h"
#include "SphereDR.h"

using namespace DirectX;

void CGame::Init()
{
	m_MainCamera = AddGameObject<CCameraFP>((int)GameObjectsLayer::Layer_Camera);
	CManager::SetMainCamera(m_MainCamera);

	for (int i = 0; i < 6; ++i)
	{
		m_DCMCameras.push_back(AddGameObject<CCameraDCM>((int)GameObjectsLayer::Layer_Camera));
	}
	
	CSky* sky = AddGameObject<CSky>((int)GameObjectsLayer::Layer_3D_Sky);
	CMeshField* meshField= AddGameObject<CMeshField>((int)GameObjectsLayer::Layer_3D_Opaque);
	CCube* cube = AddGameObject<CCube>((int)GameObjectsLayer::Layer_3D_Opaque);
	CSphere* sphere = AddGameObject<CSphere>((int)GameObjectsLayer::Layer_3D_Opaque);

	// m_GameObjectsLayer[(int)GameObjectsLayer::Layer_3D_Opaque_DynamicReflectors] <= CTextureManager::m_DynamicTextureNum
	// ãŽ®‚ðí‚É¬‚è—§‚½‚¹‚é•K—v‚ª‚ ‚é
	CSphere* sphereDR = AddGameObject<CSphereDR>((int)GameObjectsLayer::Layer_3D_Opaque_DynamicReflectors);
	//CLogo* logo = AddGameObject<CLogo>((int)GameObjectsLayer::Layer_3D_Opaque);

	CSphere* sphereDR2 = AddGameObject<CSphereDR>((int)GameObjectsLayer::Layer_3D_Opaque_DynamicReflectors);
	sphereDR2->SetPosition(XMFLOAT3(-3.0f, 1.0f, 0.0f));
	sphereDR2->SetWorldMatrix();

	CSphere* sphereDR3 = AddGameObject<CSphereDR>((int)GameObjectsLayer::Layer_3D_Opaque_DynamicReflectors);
	sphereDR3->SetPosition(XMFLOAT3(0.0f, 2.0f, 3.0f));
	sphereDR3->SetWorldMatrix();

	CSphere* sphereDR4 = AddGameObject<CSphereDR>((int)GameObjectsLayer::Layer_3D_Opaque_DynamicReflectors);
	sphereDR4->SetPosition(XMFLOAT3(0.0f, 0.5f, 2.0f));
	sphereDR4->SetWorldMatrix();

	CFrameResourceManager::CreateFrameResources();
}

void CGame::Uninit()
{
	CManager::SetMainCamera();
	CScene::Uninit();
}

void CGame::Update(const GameTimer& GlobalTimer)
{
	CScene::Update(GlobalTimer);
}

void CGame::Draw(const GameTimer& GlobalTimer)
{
	CRenderer::SetUpCommonResources();
	CRenderer::SetUpCubeMapResources();

	// CreateDynamicCubeMap
	CRenderer::CreateDynamicCubeMapResources(GlobalTimer, m_GameObjectsLayer[(int)GameObjectsLayer::Layer_3D_Opaque_DynamicReflectors]);

	CRenderer::SetUpBeforeDrawScene();
	CRenderer::DrawGameObjectsWithDynamicCubeMap(m_GameObjectsLayer[(int)GameObjectsLayer::Layer_3D_Opaque_DynamicReflectors]);

	CRenderer::SetUpCubeMapResources();
	//CRenderer::SetPSO((int)PSOTypeIndex::PSO_01_WireFrame_Opaque);
	CRenderer::DrawGameObjectsWithLayer(m_GameObjectsLayer[(int)GameObjectsLayer::Layer_3D_Opaque]);

	CRenderer::SetPSO((int)PSOTypeIndex::PSO_02_Solid_Sky);
	//CRenderer::SetPSO((int)PSOTypeIndex::PSO_03_WireFrame_Sky);
	CRenderer::DrawGameObjectsWithLayer(m_GameObjectsLayer[(int)GameObjectsLayer::Layer_3D_Sky]);
}
