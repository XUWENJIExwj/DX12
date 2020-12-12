#include "Manager.h"
#include "Renderer.h"
#include "InputManager.h"
#include "Game.h"
#include "Camera.h"

using namespace DirectX;
using namespace InputManager;

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
	CFrameResourceManager::ComputeConstantBufferSize();

	CKeyboard::Init();
	CMouse::Init();

	SetScene<CGame>();

	return true;
}

void CManager::OnResize()
{
	CRenderer::OnResize();

	if (m_MainCamera)
	{
		m_MainCamera->SetProjectionMatrix(0.25f * MathHelper::Pi, DX12App::GetApp()->GetAspectRatio(), 1.0f, 1000.0f);
	}
}

void CManager::Uninit()
{
	m_Scene->Uninit();
	delete m_Scene;
	CRenderer::Uninit();
}

void CManager::Update(const GameTimer& GlobalTimer)
{
	CKeyboard::Update();
	CMouse::Update();

	EndTheApp();

	CFrameResourceManager::CycleFrameResources();

	m_Scene->UpdateAll(GlobalTimer);
}

void CManager::Draw(const GameTimer& GlobalTimer)
{
	CRenderer::Begin();

	m_Scene->Draw(GlobalTimer);

	CRenderer::End();
}

void CManager::EndTheApp()
{
	if (CKeyboard::IsPressed(Keyboard::Escape))
	{
		SendMessage(DX12App::GetApp()->GetMainWindowHandle(), WM_DESTROY, 0, 0);
	}
}

ID3D12Device* CManager::GetDevice()
{
	return CRenderer::GetDevice();
}
