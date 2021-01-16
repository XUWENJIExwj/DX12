#include "Game.h"
#include "Manager.h"
#include "Renderer.h"
#include "Camera.h"
#include "CameraFP.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "CameraDynamicCubeMap.h"
#include "Logo.h"
#include "Sky.h"
#include "MeshField.h"
#include "Cube.h"
#include "Sphere.h"
#include "SphereDR.h"
#include "Quad.h"

using namespace std;
using namespace DirectX;

void CGame::Init()
{
	m_MainCamera = AddGameObject<CCameraFP>((int)RenderLayers::Layer_Camera, "MainCamera");
	CManager::SetMainCamera(m_MainCamera);

	m_MainDirLight = AddGameObject<CDirLight>((int)RenderLayers::Layer_DirLight, "DirLight00");
	m_MainDirLight->SetBaseDirection(XMFLOAT3(0.57735f, -0.57735f, 0.57735f));
	m_MainDirLight->SetStrength(XMFLOAT3(0.9f, 0.8f, 0.7f));

	CDirLight* dirLight1 = AddGameObject<CDirLight>((int)RenderLayers::Layer_DirLight, "DirLight01");
	dirLight1->SetBaseDirection(XMFLOAT3(-0.57735f, -0.57735f, 0.57735f));
	dirLight1->SetStrength(XMFLOAT3(0.4f, 0.4f, 0.4f));

	CDirLight* dirLight2 = AddGameObject<CDirLight>((int)RenderLayers::Layer_DirLight, "DirLight02");
	dirLight2->SetBaseDirection(XMFLOAT3(0.0f, -0.707f, -0.707f));
	dirLight2->SetStrength(XMFLOAT3(0.2f, 0.2f, 0.2f));

	for (int i = 0; i < 6; ++i)
	{
		m_DCMCameras.push_back(AddGameObject<CCameraDCM>((int)RenderLayers::Layer_Camera, "DCMCamera0" + to_string(i)));
	}
	
	CSky* sky = AddGameObject<CSky>((int)RenderLayers::Layer_3D_Sky, "Sky");
	CMeshField* meshField= AddGameObject<CMeshField>((int)RenderLayers::Layer_3D_Opaque_POM, "MeshField");
	CCube* cube = AddGameObject<CCube>((int)RenderLayers::Layer_3D_Opaque, "Brick");
	CSphere* sphere = AddGameObject<CSphere>((int)RenderLayers::Layer_3D_Opaque, "Mirror");

	// m_GameObjectsLayer[(int)GameObjectsLayer::Layer_3D_Opaque_DynamicReflectors].size() <= CTextureManager::m_DynamicTextureNum
	// ãŽ®‚ðí‚É¬‚è—§‚½‚¹‚é•K—v‚ª‚ ‚é
	CSphere* sphereDR0 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror00");
	//CLogo* logo = AddGameObject<CLogo>((int)GameObjectsLayer::Layer_3D_Opaque);

	//CSphere* sphereDR1 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror01");
	//sphereDR1->SetPosition(XMFLOAT3(-3.0f, 1.0f, 0.0f));
	//sphereDR1->SetWorldMatrix();

	CSphere* sphereDR2 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror02");
	sphereDR2->SetPosition(XMFLOAT3(0.0f, 2.0f, 3.0f));
	sphereDR2->SetWorldMatrix();

	//CSphere* sphereDR3 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror03");
	//sphereDR3->SetPosition(XMFLOAT3(0.0f, 0.5f, 2.0f));
	//sphereDR3->SetWorldMatrix();

	CQuad* quad = AddGameObject<CQuad>((int)RenderLayers::Layer_2D_Debug, "Debug");

	SetSceneBounds(10.0f, 15.0f, meshField->GetPosition3f());
	CFrameResourceManager::CreateFrameResources();
}

void CGame::Uninit()
{
	CManager::SetMainCamera();
	CScene::Uninit();
}

void CGame::UpdateAll(const GameTimer& GlobalTimer)
{
	CScene::UpdateAll(GlobalTimer);

	UpdateShadowPassCB(GlobalTimer);
}

void CGame::Draw(const GameTimer& GlobalTimer)
{
	CRenderer::SetUpCommonResources();

	// CreateShadowMap
	CRenderer::SetUpNullCubeMapResource();
	CRenderer::SetUpBeforeCreateShadowMapReource();
	CRenderer::SetPSO((int)PSOTypeIndex::PSO_ShadowMap);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_DynamicReflectors]);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque]);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_POM]);
	CRenderer::CompleteCreateShadowMapResource();

	CRenderer::SetUpSkyCubeMapResources();

	// CreateDynamicCubeMap
	CRenderer::CreateDynamicCubeMapResources(GlobalTimer, m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_DynamicReflectors]);

	CRenderer::SetUpBeforeDrawScene();
	CRenderer::DrawGameObjectsWithDynamicCubeMap(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_DynamicReflectors]);

	CRenderer::SetUpSkyCubeMapResources();
	//CRenderer::SetPSO((int)PSOTypeIndex::PSO_01_WireFrame_Opaque);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque]);

	CRenderer::SetPSO((int)PSOTypeIndex::PSO_Solid_Opaque_POM);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_POM]);

	CRenderer::SetPSO((int)PSOTypeIndex::PSO_Solid_Sky);
	//CRenderer::SetPSO((int)PSOTypeIndex::PSO_03_WireFrame_Sky);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Sky]);

	CRenderer::SetPSO((int)PSOTypeIndex::PSO_ShadowMapDebug);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_2D_Debug]);
}
