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

	CCube* cube0 = AddGameObject<CCube>((int)RenderLayers::Layer_3D_Opaque, "Brick00");
	CCube* cube1 = AddGameObject<CCube>((int)RenderLayers::Layer_3D_Opaque, "Brick01");
	cube1->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	cube1->SetScale(XMFLOAT3(2.0f, 1.0f, 2.0f));
	cube1->SetWorldMatrix();

	CSphere* sphere = AddGameObject<CSphere>((int)RenderLayers::Layer_3D_Opaque, "Mirror");

	// m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_DynamicReflectors].size() <= CTextureManager::m_DynamicTextureNum
	// 上式を常に成り立たせる必要がある
	CSphere* sphereDR0 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror00");

	CSphere* sphereDR1 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror01");
	sphereDR1->SetPosition(XMFLOAT3(-3.0f, 1.5f, 0.0f));
	sphereDR1->SetWorldMatrix();

	CSphere* sphereDR2 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror02");
	sphereDR2->SetPosition(XMFLOAT3(0.0f, 2.0f, 3.0f));
	sphereDR2->SetWorldMatrix();

	CSphere* sphereDR3 = AddGameObject<CSphereDR>((int)RenderLayers::Layer_3D_Opaque_DynamicReflectors, "DynamicMirror03");
	sphereDR3->SetPosition(XMFLOAT3(0.0f, 1.5f, 2.0f));
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
	BoundingBox::CreateMerged(m_SceneBoundingBox, *meshField->GetBoundingBox(), *cube0->GetBoundingBox());

	// RadialBlurCB
	DX12App* app = DX12App::GetApp();
	m_RadialBlurCB.CenterX = app->GetWindowWidth() / 2;
	m_RadialBlurCB.CenterY = app->GetWindowHeight() / 2;
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

void CGame::UpdateMainPassCB(const GameTimer & GlobalTimer)
{
	DX12App* app = DX12App::GetApp();
	int windowWidth = app->GetWindowWidth();
	int windowHeight = app->GetWindowHeight();

	// Camera
	XMMATRIX view = m_MainCamera->GetView();
	XMMATRIX proj = m_MainCamera->GetProj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
	XMStoreFloat4x4(&m_MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	m_MainPassCB.EyePosW = m_MainCamera->GetPosition3f();
	m_MainPassCB.RenderTargetSize = XMFLOAT2((float)windowWidth, (float)windowHeight);
	m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / windowWidth, 1.0f / windowHeight);
	m_MainPassCB.NearZ = m_MainCamera->GetNearZ();
	m_MainPassCB.FarZ = m_MainCamera->GetFarZ();

	// Time
	m_MainPassCB.TotalTime = GlobalTimer.TotalTime();
	m_MainPassCB.DeltaTime = GlobalTimer.DeltaTime();

	// Light
	m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	for (UINT i = 0; i < m_DirLights.size(); ++i)
	{
		m_MainPassCB.Lights[i].Direction = m_DirLights[i]->GetDirection3f();
		m_MainPassCB.Lights[i].Strength = m_DirLights[i]->GetStrength3f();
	}

	// CSM
	XMMATRIX lightView = m_DirLights[0]->ComputeLightView(&m_SceneBoundingSphere);
	XMStoreFloat4x4(&m_MainPassCB.ShadowView, XMMatrixTranspose(lightView));

	CCascadeShadowMap* csm = CRenderer::GetCascadeShadowMap();
	CSMPassCB* csmPassCB = csm->GetCSMPassCB();
	csm->ComputeFitCascadeCSMPassCB(invView, &m_SceneBoundingBox, m_MainCamera, m_DirLights[0]);

	float shadowMapSize = (float)CRenderer::GetShadowMapSize();
	m_MainPassCB.MaxBorderPadding = (shadowMapSize - 1.0f) / shadowMapSize;
	m_MainPassCB.MinBorderPadding = 1.0f / shadowMapSize;
	m_MainPassCB.CascadeBlendArea = csmPassCB->CascadeBlendArea;
	m_MainPassCB.ShadowBias.x = csmPassCB->ShadowBias.x;
	m_MainPassCB.ShadowBias.y = csmPassCB->ShadowBias.y;
	m_MainPassCB.ShadowBias.z = csmPassCB->ShadowBias.z;
	m_MainPassCB.PCFBlurForLoopStart = csmPassCB->PCFBlurForLoopStart;
	m_MainPassCB.PCFBlurForLoopEnd = csmPassCB->PCFBlurForLoopEnd;
	m_MainPassCB.VisualCascade = (BOOL)csmPassCB->VisualCascade;
	m_MainPassCB.BlendCascade = (BOOL)csmPassCB->BlendCascade;
	m_MainPassCB.CascadeFrustumsEyeSpaceDepthsFloat = csmPassCB->CascadeFrustumsEyeSpaceDepthsFloat;

	for (UINT i = 0; i < csm->GetCascadeNum(); ++i)
	{
		m_MainPassCB.ShadowTexScale[i] = csmPassCB->ShadowTexScale[i];
		m_MainPassCB.ShadowTexOffset[i] = csmPassCB->ShadowTexOffset[i];
		m_MainPassCB.CascadeFrustumsEyeSpaceDepthsFloat4[i] = csmPassCB->CascadeFrustumsEyeSpaceDepthsFloat4[i];
	}

	auto currPassCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void CGame::Draw(const GameTimer& GlobalTimer)
{
	CRenderer::SetUpCommonResources();

	// CreateCascadeShadowMap
	CRenderer::SetUpNullCubeMapResource();
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

	// Draw3DObjInScene
	CRenderer::SetUpSkyCubeMapResources();
	//CRenderer::SetPSO((int)PSOTypeIndex::PSO_01_WireFrame_Opaque);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque]);

	CRenderer::SetPSO((int)PSOTypeIndex::PSO_Solid_Opaque_POM);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Opaque_POM]);

	CRenderer::SetPSO((int)PSOTypeIndex::PSO_Solid_Sky);
	//CRenderer::SetPSO((int)PSOTypeIndex::PSO_03_WireFrame_Sky);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_3D_Sky]);

	// PostProcessing
	CRenderer::DoPostProcessing((int)PSOTypeIndex::PSO_RadialBlur, &m_RadialBlurCB);

	// Draw2DObjInScene
	CRenderer::SetPSO((int)PSOTypeIndex::PSO_ShadowMapDebug);
	CRenderer::DrawGameObjectsWithLayer(m_AllRenderLayers[(int)RenderLayers::Layer_2D_Debug]);
}

void CGame::UpdateSceneImGui(const GameTimer& GlobalTimer)
{
	static bool showClose = true;

	if (showClose)
	{
		ImGui::SetNextWindowPos(ImVec2(20, 340), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"PostProcessing", &showClose, window_flags);
		if (ImGui::TreeNode("RadialBlur"))
		{
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.1f);
			ImGui::InputScalar(u8"CenterX", ImGuiDataType_U32, &m_RadialBlurCB.CenterX, NULL, NULL, "%d", ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::SameLine();
			ImGui::InputScalar(u8"CenterY", ImGuiDataType_U32, &m_RadialBlurCB.CenterY, NULL, NULL, "%d", ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::Checkbox(u8"EffectOn", &m_RadialBlurCB.EffectOn);
			ImGui::SameLine();
			ImGui::PopItemWidth();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);
			ImGui::DragInt(u8"BlurCount", &m_RadialBlurCB.BlurCount, 0.1f, 1, 20);
			ImGui::DragInt(u8"SampleDistance", &m_RadialBlurCB.SampleDistance, 0.5f, 1, 100);
			ImGui::DragInt(u8"SampleStrength", &m_RadialBlurCB.SampleStrength, 0.5f, 0, 100);
			ImGui::PopItemWidth();
			ImGui::TreePop();
		}
		ImGui::End();
	}
}

void CGame::OnResize()
{
	OnResizeLayer((int)RenderLayers::Layer_2D_Debug);
}