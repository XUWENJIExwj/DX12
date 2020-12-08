#include "Manager.h"
#include "Renderer.h"
#include "Game.h"
#include "Camera.h"

const int gNumFrameResources = 3;

CScene*  CManager::m_Scene = nullptr;
CCamera* CManager::m_MainCamera = nullptr;

bool CManager::Init()
{
	if (!CRenderer::Init())
	{
		return false;
	}

	CRenderer::OnResize();
	CRenderer::CreateCommonResources();

	SetScene<CGame>();

	return true;
}

void CManager::OnResize()
{
	CRenderer::OnResize();

	if (m_MainCamera)
	{
		m_MainCamera->SetLens(0.25f * MathHelper::Pi, DX12App::GetApp()->AspectRatio(), 1.0f, 1000.0f);
	}
}

void CManager::Uninit()
{
	m_Scene->Uninit();
	delete m_Scene;
	CRenderer::Uninit();
}

void CManager::Update(const GameTimer& gt)
{
	CFrameResourceManager::Update();
	m_Scene->Update(gt);
	m_Scene->LateUpdate(gt);
	m_Scene->CheckNecessaryCBBufferSize();
	m_Scene->UpdateGameObjectsCB(gt);
	m_Scene->UpdateMaterialBuffer(gt);
	m_Scene->UpdateMainPassCB(gt);
}

void CManager::Draw(const GameTimer& gt)
{
	CRenderer::Begin();

	CRenderer::SetUpCommonResources();
	CRenderer::DrawDynamicCubeScene();
	CRenderer::SetUpBeforeDrawScene();
	m_Scene->Draw();

	CRenderer::End();
}

ID3D12Device* CManager::GetDevice()
{
	return CRenderer::GetDevice();
}
