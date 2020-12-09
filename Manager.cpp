#include "Manager.h"
#include "Renderer.h"
#include "Input.h"
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

	CInput::Init();

	SetScene<CGame>();

	return true;
}

void CManager::OnResize()
{
	CRenderer::OnResize();

	if (m_MainCamera)
	{
		m_MainCamera->SetLens(0.25f * MathHelper::Pi, DX12App::GetApp()->GetAspectRatio(), 1.0f, 1000.0f);
	}
}

void CManager::Uninit()
{
	m_Scene->Uninit();
	delete m_Scene;
	CInput::Uninit();
	CRenderer::Uninit();
}

void CManager::Update(const GameTimer& GlobalTimer)
{
	CInput::Update();

	CFrameResourceManager::Update();
	m_Scene->Update(GlobalTimer);
	m_Scene->LateUpdate(GlobalTimer);
	m_Scene->CheckNecessaryCBBufferSize();
	m_Scene->UpdateGameObjectsCB(GlobalTimer);
	m_Scene->UpdateMaterialBuffer(GlobalTimer);
	m_Scene->UpdateMainPassCB(GlobalTimer);
}

void CManager::Draw(const GameTimer& GlobalTimer)
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
