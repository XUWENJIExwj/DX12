#include "Game.h"
#include "Manager.h"
#include "Camera.h"
#include "Sky.h"
#include "Box.h"

using namespace DirectX;

void CGame::Init()
{
	m_MainCamera = AddGameObject<CCamera>((int)GameObjectsLayer::Layer_Camera);
	//m_MainCamera->SetLens(0.25f * MathHelper::Pi, DX12App::GetApp()->GetAspectRatio(), 1.0f, 1000.0f);
	CManager::SetMainCamera(m_MainCamera);

	CBox* box = AddGameObject<CBox>((int)GameObjectsLayer::Layer_Opaque_3DOBJ);
	CSky* sky = AddGameObject<CSky>((int)GameObjectsLayer::Layer_Sky);


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