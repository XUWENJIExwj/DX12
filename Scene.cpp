#include "Renderer.h"
#include "Scene.h"
#include "GameObject.h"
#include "Camera.h"
#include "Light.h"
#include "DirectionalLight.h"

using namespace std;
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
			); // ラムダ式
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
		); // ラムダ式
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
		); // ラムダ式
	}
}

void CScene::UpdateImGui(const GameTimer& GlobalTimer)
{
	UpdateSceneImGui(GlobalTimer);

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
			if (numFramesDirty > 0 && gameObject->GetGameObjectLayer() >= (int)RenderLayers::Layer_3D_Opaque)
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
	static int firstInit = gNumFrameResources;
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
			matData.BitangentSign = materials[i]->BitangentSign;
			matData.UseACForPOM = materials[i]->UseACForPOM;
			matData.MaxSampleCount = materials[i]->MaxSampleCount;
			matData.MinSampleCount = materials[i]->MinSampleCount;
			matData.CascadeDebugIndex = materials[i]->CascadeDebugIndex;
			matData.HeightScale = materials[i]->HeightScale;
			matData.ShadowSoftening = materials[i]->ShadowSoftening;
			matData.ShowSelfShadow = materials[i]->ShowSelfShadow;
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

	float shadowMapSize = (float)CRenderer::GetShadowMapSize();
	m_MainPassCB.MaxBorderPadding = (shadowMapSize - 1.0f) / shadowMapSize;
	m_MainPassCB.MinBorderPadding = 1.0f / shadowMapSize;
	m_MainPassCB.ShadowBias = m_ShadowBias;
	m_MainPassCB.CascadeBlendArea = 0.005f;
	m_MainPassCB.ViualCascade = m_VisualCascade;

	// Light
	if (m_DirLights.size() > 0)
	{
		// CSM
		vector<vector<XMVECTOR>> frustumPoints(CRenderer::GetCascadNum());
		m_MainCamera->ComputeFrustumPointsInWorldSpace(frustumPoints, invView);
		vector<XMMATRIX> shadowTransforms(CRenderer::GetCascadNum());
		m_DirLights[0]->ComputeShadowTransformWithCameraFrustum(shadowTransforms, &m_SceneBounds, frustumPoints);
		XMStoreFloat4x4(&m_MainPassCB.ShadowView, XMMatrixTranspose(m_DirLights[0]->GetView()));

		XMFLOAT4X4 shadowTransform;
		for (UINT i = 0; i < shadowTransforms.size(); ++i)
		{
			XMStoreFloat4x4(&shadowTransform, shadowTransforms[i]);
			m_MainPassCB.ShadowTexScale[i].x = shadowTransform(0, 0);
			m_MainPassCB.ShadowTexScale[i].y = shadowTransform(1, 1);
			m_MainPassCB.ShadowTexScale[i].z = shadowTransform(2, 2);
			m_MainPassCB.ShadowTexScale[i].w = 1.0f;

			m_MainPassCB.ShadowTexOffset[i].x = shadowTransform(3, 0);
			m_MainPassCB.ShadowTexOffset[i].y = shadowTransform(3, 1);
			m_MainPassCB.ShadowTexOffset[i].z = shadowTransform(3, 2);
			m_MainPassCB.ShadowTexOffset[i].w = 0.0f;
		}

		for (UINT i = 0; i < m_DirLights.size(); ++i)
		{
			m_MainPassCB.Lights[i].Direction = m_DirLights[i]->GetDirection3f();
			m_MainPassCB.Lights[i].Strength = m_DirLights[i]->GetStrength3f();
		}
	}
	else
	{
		m_MainPassCB.ShadowView = m_ShadowTransform;
		m_MainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		m_MainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
		m_MainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		m_MainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
		m_MainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		m_MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };
	}

	m_MainPassCB.PCFBlurForLoopStart = m_PCFBlurForLoopStart;
	m_MainPassCB.PCFBlurForLoopEnd = m_PCFBlurForLoopEnd;

	m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	auto currPassCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void CScene::UpdateShadowPassCB(const GameTimer & GlobalTimer)
{
	XMMATRIX view = m_DirLights[0]->GetView();
	XMMATRIX proj = m_DirLights[0]->GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	float size = (float)CRenderer::GetShadowMapSize() / CRenderer::GetCascadNum();

	XMStoreFloat4x4(&m_ShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_ShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_ShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_ShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_ShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_ShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	m_ShadowPassCB.EyePosW = m_DirLights[0]->GetPosition3f();
	m_ShadowPassCB.RenderTargetSize = XMFLOAT2(size, size);
	m_ShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / size, 1.0f / size);
	m_ShadowPassCB.NearZ = m_DirLights[0]->GetNearZ();
	m_ShadowPassCB.FarZ = m_DirLights[0]->GetFarZ();

	auto currPassCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB.get();
	currPassCB->CopyData(1, m_ShadowPassCB);
}

void CScene::UpdateShadowPassCB(const GameTimer& GlobalTimer, int CascadeIndex)
{
	XMMATRIX view = m_DirLights[0]->GetView();
	XMMATRIX proj = m_DirLights[0]->GetProj(CascadeIndex);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	float size = (float)CRenderer::GetShadowMapSize() / CRenderer::GetCascadNum();

	XMStoreFloat4x4(&m_ShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_ShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_ShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_ShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_ShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_ShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	m_ShadowPassCB.EyePosW = m_DirLights[0]->GetPosition3f();
	m_ShadowPassCB.RenderTargetSize = XMFLOAT2(size, size);
	m_ShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / size, 1.0f / size);
	m_ShadowPassCB.NearZ = m_DirLights[0]->GetNearZ();
	m_ShadowPassCB.FarZ = m_DirLights[0]->GetFarZ();

	auto currPassCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB.get();
	currPassCB->CopyData(1 + CascadeIndex, m_ShadowPassCB);
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
		//cubeFacePassCB.ShadowTransform = m_ShadowTransform;
		//cubeFacePassCB.ShadowTransform[0] = m_ShadowTransform;
		cubeFacePassCB.EyePosW = m_DCMCameras[i]->GetPosition3f();

		float dynamicCubeMapSize = (float)CRenderer::GetDynamicCubeMapSize();
		cubeFacePassCB.RenderTargetSize = XMFLOAT2(dynamicCubeMapSize, dynamicCubeMapSize);
		cubeFacePassCB.InvRenderTargetSize = XMFLOAT2(1.0f / dynamicCubeMapSize, 1.0f / dynamicCubeMapSize);

		auto currPassCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB.get();

		// Cube map pass cbuffers are stored in elements from 2 to DCMResourcesIndex * 6.
		currPassCB->CopyData(1 + CRenderer::GetCascadNum() + i + DCMResourcesIndex * 6, cubeFacePassCB);
	}
}

void CScene::OnResizeLayer(int Layer)
{
	for (CGameObject* gameObject : m_AllRenderLayers[Layer])
	{
		gameObject->OnResize();
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

void CScene::SetSceneBounds(DirectX::BoundingBox * Bounds)
{
	SetSceneBounds(Bounds->Extents.x, Bounds->Extents.z, Bounds->Center);
}

void CScene::SetSceneBounds(float Width, float Height, XMFLOAT3 Center)
{
	m_SceneBounds.Center = Center;
	m_SceneBounds.Radius = sqrtf(Width * Width + Height * Height);
}