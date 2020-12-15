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

void CScene::UpdateAll(const GameTimer& GlobalTimer)
{
	UpdateImGui(GlobalTimer);
	FixedUpdate(GlobalTimer);
	Update(GlobalTimer);
	LateUpdate(GlobalTimer);
	CheckNecessaryCBBufferSize();
	UpdateGameObjectsCB(GlobalTimer);
	UpdateMaterialBuffer(GlobalTimer);
	UpdateMainPassCB(GlobalTimer);
}

void CScene::FixedUpdate(const GameTimer& GlobalTimer)
{
	if (GlobalTimer.GetFixed())
	{
		for (int i = 0; i < (int)RenderLayers::Layer_Max - 1; ++i)
		{
			for (CGameObject* gameObject : m_AllRenderLayers[i])
			{
				gameObject->FixedUpdate(GlobalTimer);
			}

			m_AllRenderLayers[i].remove_if
			(
				[](CGameObject* GameObject)
			{
				return GameObject->Destroy();
			}
			); // ƒ‰ƒ€ƒ_Ž®
		}
	}
}

void CScene::Update(const GameTimer& GlobalTimer)
{
	for (int i = 0; i < (int)RenderLayers::Layer_Max - 1; ++i)
	{
		for (CGameObject* gameObject : m_AllRenderLayers[i])
		{
			gameObject->Update(GlobalTimer);
		}

		m_AllRenderLayers[i].remove_if
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
	for (int i = 0; i < (int)RenderLayers::Layer_Max - 1; ++i)
	{
		for (CGameObject* gameObject : m_AllRenderLayers[i])
		{
			gameObject->LateUpdate(GlobalTimer);
		}

		m_AllRenderLayers[i].remove_if
		(
			[](CGameObject* GameObject)
		{
			return GameObject->Destroy();
		}
		); // ƒ‰ƒ€ƒ_Ž®
	}
}

void CScene::UpdateImGui(const GameTimer & GlobalTimer)
{
	for (int i = 0; i < (int)RenderLayers::Layer_Max; ++i)
	{
		for (CGameObject* gameObject : m_AllRenderLayers[i])
		{
			gameObject->UpdateImGui(GlobalTimer);
		}
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
			if (numFramesDirty > 0 && gameObject->GetGameObjectLayer() != (int)RenderLayers::Layer_Camera)
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
	auto materials = CMaterialManager::GetAllMaterials().data();

	for (int i = 0; i < CMaterialManager::GetAllMaterialsCount(); ++i)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		if (materials[i]->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&materials[i]->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = materials[i]->DiffuseAlbedo;
			matData.CubeMapDiffuseAlbedo = materials[i]->CubeMapDiffuseAlbedo;
			matData.FresnelR0 = materials[i]->FresnelR0;
			matData.Roughness = materials[i]->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = materials[i]->DiffuseSrvHeapIndex;
			matData.NormalMapIndex = materials[i]->NormalSrvHeapIndex;
			matData.HeightMapIndex = materials[i]->HeightSrvHeapIndex;
			matData.UseACForPOM = materials[i]->UseACForPOM;
			matData.MaxSampleCount = materials[i]->MaxSampleCount;
			matData.MinSampleCount = materials[i]->MinSampleCount;
			matData.TangentSign = materials[i]->TangentSign;
			matData.MaterialPad0 = materials[i]->MaterialPad0;
			matData.ShowSelfShadow = materials[i]->ShowSelfShadow;
			matData.MaterialPad1 = materials[i]->MaterialPad1;
			matData.MaterialPad2 = materials[i]->MaterialPad2;
			matData.MaterialPad3 = materials[i]->MaterialPad3;

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

void CScene::UpdateDynamicCubeMapPassCB(const GameTimer& GlobalTimer, int DCMResourcesIndex)
{
	for (int i = 0; i < (int)m_DCMCameras.size(); ++i)
	{
		PassConstants cubeFacePassCB = m_MainPassCB;

		XMMATRIX view = m_DCMCameras[i]->GetView();
		XMMATRIX proj = m_DCMCameras[i]->GetProj();

		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&cubeFacePassCB.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&cubeFacePassCB.InvView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&cubeFacePassCB.Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&cubeFacePassCB.InvProj, XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&cubeFacePassCB.ViewProj, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&cubeFacePassCB.InvViewProj, XMMatrixTranspose(invViewProj));
		cubeFacePassCB.EyePosW = m_DCMCameras[i]->GetPosition3f();

		float dynamicCubeMapSize = (float)CRenderer::GetDynamicCubeMapSize();
		cubeFacePassCB.RenderTargetSize = XMFLOAT2(dynamicCubeMapSize, dynamicCubeMapSize);
		cubeFacePassCB.InvRenderTargetSize = XMFLOAT2(1.0f / dynamicCubeMapSize, 1.0f / dynamicCubeMapSize);

		auto currPassCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB.get();

		// Cube map pass cbuffers are stored in elements from 1 to DCMResourcesIndex * 6.
		currPassCB->CopyData(1 + i + DCMResourcesIndex * 6, cubeFacePassCB);
	}
}

void CScene::SetUpDynamicCubeMapCamera(XMFLOAT3 Center)
{
	// Generate the cube map about the given position.
	XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	float x = Center.x;
	float y = Center.y;
	float z = Center.z;

	// Look along each coordinate axis.
	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(x + 1.0f, y, z), // +X
		XMFLOAT3(x - 1.0f, y, z), // -X
		XMFLOAT3(x, y + 1.0f, z), // +Y
		XMFLOAT3(x, y - 1.0f, z), // -Y
		XMFLOAT3(x, y, z + 1.0f), // +Z
		XMFLOAT3(x, y, z - 1.0f)  // -Z
	};

	// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
	// are looking down +Y or -Y, so we need a different "up" vector.
	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
		XMFLOAT3(0.0f, 1.0f, 0.0f)	 // -Z
	};

	for (int i = 0; i < (int)m_DCMCameras.size(); ++i)
	{
		m_DCMCameras[i]->LookAt(Center, targets[i], ups[i]);
		m_DCMCameras[i]->UpdateViewMatrix();
	}
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
