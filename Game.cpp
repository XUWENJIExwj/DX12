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
#include "QuadShadowDebug.h"

using namespace std;
using namespace DirectX;

void CGame::Init()
{
	m_BeginPSOIndex = (int)PSOTypeIndex::PSO_ShadowMap;

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

	// m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_DynamicReflectors].size() <= CTextureManager::m_DynamicTextureNum
	// 上式を常に成り立たせる必要がある
	CSphere* sphereDR0 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror00");

	CSphere* sphereDR1 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror01");
	sphereDR1->SetPosition(XMFLOAT3(-3.0f, 1.0f, 0.0f));
	sphereDR1->SetWorldMatrix();

	CSphere* sphereDR2 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror02");
	sphereDR2->SetPosition(XMFLOAT3(0.0f, 2.0f, 3.0f));
	sphereDR2->SetWorldMatrix();

	CSphere* sphereDR3 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror03");
	sphereDR3->SetPosition(XMFLOAT3(0.0f, 0.5f, 2.0f));
	sphereDR3->SetWorldMatrix();

	CLogo* logo00 = AddGameObject<CLogo>((int)RenderLayers::Layer_3D_Opaque, "Logo00");
	CLogo* logo01 = AddGameObject<CLogo>((int)RenderLayers::Layer_3D_Opaque, "Logo01");
	logo01->SetRotation(XMFLOAT3(0.0f, MathHelper::Pi, 0.0f));
	logo01->SetWorldMatrix();

	CQuadShadowDebug* shadowDebug00 = AddGameObject<CQuadShadowDebug>((int)RenderLayers::Layer_2D_Debug, "ShadowDebug00");
	CQuadShadowDebug* shadowDebug01 = AddGameObject<CQuadShadowDebug>((int)RenderLayers::Layer_2D_Debug, "ShadowDebug01");
	shadowDebug01->SetMaterialNormal((int)MaterialNormalIndex::Material_ShadowMap_01);
	shadowDebug01->SetPositionX(shadowDebug00->GetPosition3f().x + shadowDebug00->GetScale3f().x);
	shadowDebug01->SetOrderColNum(1);
	shadowDebug01->Set2DWVPMatrix();
	CQuadShadowDebug* shadowDebug02 = AddGameObject<CQuadShadowDebug>((int)RenderLayers::Layer_2D_Debug, "ShadowDebug02");
	shadowDebug02->SetMaterialNormal((int)MaterialNormalIndex::Material_ShadowMap_02);
	shadowDebug02->SetPositionX(shadowDebug01->GetPosition3f().x + shadowDebug01->GetScale3f().x);
	shadowDebug02->SetOrderColNum(2);
	shadowDebug02->Set2DWVPMatrix();

	SetSceneBoundingSphere(meshField->GetBoundingBox());

	// SceneのBoundingBoxの生成（全てのObjのBoundingBoxをマージすべきだが、Demoの中では、MeshFieldとCubeのサイズをもって、Objを網羅できるので、それでいく）
	BoundingBox::CreateMerged(m_SceneBoundingBox, *meshField->GetBoundingBox(), *cube->GetBoundingBox());
	//SetSceneBoundingSphere(100.0f, 100.0f, meshField->GetPosition3f());
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
}

void CGame::Draw(const GameTimer& GlobalTimer)
{
	CRenderer::SetUpCommonResources();

	// CreateShadowMap
	CRenderer::SetUpNullCubeMapResource();

	// CreateCascadeShadowMap
	for (UINT i = 0; i < CRenderer::GetCascadNum(); ++i)
	{
		CRenderer::SetUpBeforeCreateCascadeShadowMapReources(i);
		UpdateShadowPassCB(GlobalTimer, i);
		CRenderer::SetUPViewPortAndScissorRectAndPassCBBeforeCreateCascadeShadowMapReources(i);
		CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_DynamicReflectors]);
		CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque]);
		CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_POM]);
		CRenderer::CompleteCreateShadowMapResource(i);
	}

	// CreateDynamicCubeMap
	CRenderer::SetPSO((int)PSOTypeIndex::PSO_Solid_Opaque);
	CRenderer::SetUpSkyCubeMapResources();
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

void CGame::UpdateSceneImGui(const GameTimer& GlobalTimer)
{
	static bool showClose = true;

	if (showClose)
	{
		ImGui::SetNextWindowPos(ImVec2(20, 340), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"SceneManager", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		ImGui::Checkbox(u8"VisualCascade", &m_VisualCascade);
		ImGui::SameLine();
		ImGui::Checkbox(u8"BlendCascade", &m_BlendCascade);
		if (ImGui::DragInt(u8"PCFBlurSize", &m_PCFBlurSize, 0.1f, 1, 5))
		{
			m_PCFBlurForLoopStart = m_PCFBlurSize / -2;
			m_PCFBlurForLoopEnd = m_PCFBlurSize / 2 + 1;
		}
		ImGui::DragFloat(u8"ShadowBias", &m_ShadowBias, 0.00001f, -0.1f, 0.01f, "%.4f");
		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void CGame::OnResize()
{
	OnResizeLayer((int)RenderLayers::Layer_2D_Debug);
}