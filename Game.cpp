#include "Game.h"
#include "Manager.h"
#include "Camera.h"
#include "Sky.h"

using namespace DirectX;

void CGame::Init()
{
	m_MainCamera = AddGameObject<CCamera>((int)GameObjectsLayer::Layer_Camera);
	m_MainCamera->SetLens(0.25f * MathHelper::Pi, DX12App::GetApp()->AspectRatio(), 1.0f, 1000.0f);
	CManager::SetMainCamera(m_MainCamera);

	CSky* sky = AddGameObject<CSky>((int)GameObjectsLayer::Layer_Sky);


	CFrameResourceManager::CreateFrameResources();
}

void CGame::Uninit()
{
	CManager::SetMainCamera();
	CScene::Uninit();
}

void CGame::Update(const GameTimer& gt)
{
	CScene::Update(gt);
}