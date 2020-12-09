#include "Renderer.h"
#include "Scene.h"
#include "GameObject.h"
#include "Camera.h"

using namespace DirectX;

void CScene::Uninit()
{
	for (CGameObject* gameObject : m_AllGameObjects)
	{
		gameObject->Uninit();
		delete gameObject;
	}

	m_AllGameObjects.clear();
	m_AllGameObjects.resize(0);
}

void CScene::Update(const GameTimer& GlobalTimer)
{
	for (int i = 0; i < (int)GameObjectsLayer::Layer_Max - 1; ++i)
	{
		for (CGameObject* gameObject : m_GameObjectsLayer[i])
		{
			gameObject->Update(GlobalTimer);
		}

		m_GameObjectsLayer[i].remove_if
		(
			[](CGameObject* GameObject)
		{
			return GameObject->Destroy();
		}
		); // ƒ‰ƒ€ƒ_Ž®
	}
}

void CScene::LateUpdate(const GameTimer& GlobalTimer)
{
	for (int i = 0; i < (int)GameObjectsLayer::Layer_Max - 1; ++i)
	{
		for (CGameObject* object : m_GameObjectsLayer[i])
		{
			object->LateUpdate(GlobalTimer);
		}

		m_GameObjectsLayer[i].remove_if
		(
			[](CGameObject* GameObject)
		{
			return GameObject->Destroy();
		}
		); // ƒ‰ƒ€ƒ_Ž®
	}
}

void CScene::UpdateGameObjectsCB(const GameTimer& GlobalTimer)
{
	auto currObjectCB = CFrameResourceManager::GetCurrentFrameResource()->ObjectCB.get();
	for (auto& gameObject : m_AllGameObjects)
	{
		if (gameObject)
		{
			int numFramesDirty = gameObject->GetNumFramesDirty();
			// Only update the cbuffer data if the constants have changed.  
			// This needs to be tracked per frame resource.
			if (numFramesDirty > 0 && gameObject->GetGameObjectLayer() != (int)GameObjectsLayer::Layer_Camera)
			{
				DirectX::XMMATRIX world = gameObject->GetWorldMatrix();
				DirectX::XMMATRIX texTransform = gameObject->GetTexTransformMatrix();

				ObjectConstants objConstants;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
				objConstants.MaterialIndex = gameObject->GetMaterial()->MatCBIndex;

				currObjectCB->CopyData(gameObject->GetObjCBIndex(), objConstants);

				// Next FrameResource need to be updated too.
				numFramesDirty--;
				gameObject->SetNumFramesDirty(numFramesDirty);
			}
		}
	}
}

void CScene::UpdateMaterialBuffer(const GameTimer& GlobalTimer)
{
	auto currMaterialBuffer = CFrameResourceManager::GetCurrentFrameResource()->MaterialBuffer.get();
	auto materials = CMaterialManager::GetMaterials().data();

	for (int i = 0; i < CMaterialManager::GetMaterialsCount(); i++)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		if (materials[i]->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&materials[i]->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = materials[i]->DiffuseAlbedo;
			matData.FresnelR0 = materials[i]->FresnelR0;
			matData.Roughness = materials[i]->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = materials[i]->DiffuseSrvHeapIndex;

			currMaterialBuffer->CopyData(materials[i]->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			materials[i]->NumFramesDirty--;
		}
	}
}

void CScene::UpdateMainPassCB(const GameTimer& GlobalTimer)
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
	m_MainPassCB.NearZ = 1.0f;
	m_MainPassCB.FarZ = 1000.0f;

	// Time
	m_MainPassCB.TotalTime = GlobalTimer.TotalTime();
	m_MainPassCB.DeltaTime = GlobalTimer.DeltaTime();

	// Light
	m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	m_MainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
	m_MainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	m_MainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	m_MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void CScene::Draw()
{
	//CRenderer::SetPSO((int)PSOTypeIndex::PSO_01_WireFrame_Opaque);
	CRenderer::DrawGameObjectsWithLayer(m_GameObjectsLayer[(int)GameObjectsLayer::Layer_Opaque_3DOBJ]);

	CRenderer::SetPSO((int)PSOTypeIndex::PSO_02_Solid_Sky);
	//CRenderer::SetPSO((int)PSOTypeIndex::PSO_03_WireFrame_Sky);
	CRenderer::DrawGameObjectsWithLayer(m_GameObjectsLayer[(int)GameObjectsLayer::Layer_Sky]);
}

void CScene::CheckNecessaryCBBufferSize()
{
	if (CFrameResourceManager::CreateFrameResources())
	{
		for (CGameObject* gameObject : m_AllGameObjects)
		{
			if (gameObject)
			{
				gameObject->SetNumFramesDirty(gNumFrameResources);
			}
		}
	}
}
