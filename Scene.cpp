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
			); // ÉâÉÄÉ_éÆ
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
		); // ÉâÉÄÉ_éÆ
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
		); // ÉâÉÄÉ_éÆ
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

	// Light
	m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	if (m_DirLights.size() > 0)
	{
		for (UINT i = 0; i < m_DirLights.size(); ++i)
		{
			m_MainPassCB.Lights[i].Direction = m_DirLights[i]->GetDirection3f();
			m_MainPassCB.Lights[i].Strength = m_DirLights[i]->GetStrength3f();
		}

		// CSM
		XMMATRIX lightView = m_DirLights[0]->ComputeLightView(&m_SceneBoundingSphere);
		XMStoreFloat4x4(&m_MainPassCB.ShadowView, XMMatrixTranspose(lightView));

		ComputeFitCascadeCSMPassCB(invView);

		//vector<vector<XMVECTOR>> frustumPoints(CRenderer::GetCascadNum());
		//m_MainCamera->ComputeFrustumPointsInWorldSpace(frustumPoints, invView);
		//vector<XMMATRIX> shadowTransforms(CRenderer::GetCascadNum());
		//m_DirLights[0]->ComputeShadowTransformWithCameraFrustum(shadowTransforms, &m_SceneBoundingSphere, frustumPoints);

		//XMFLOAT4X4 shadowTransform;
		//for (UINT i = 0; i < shadowTransforms.size(); ++i)
		//{
		//	XMStoreFloat4x4(&shadowTransform, shadowTransforms[i]);
		//	m_MainPassCB.ShadowTexScale[i].x = shadowTransform(0, 0);
		//	m_MainPassCB.ShadowTexScale[i].y = shadowTransform(1, 1);
		//	m_MainPassCB.ShadowTexScale[i].z = shadowTransform(2, 2);
		//	m_MainPassCB.ShadowTexScale[i].w = 1.0f;

		//	m_MainPassCB.ShadowTexOffset[i].x = shadowTransform(3, 0);
		//	m_MainPassCB.ShadowTexOffset[i].y = shadowTransform(3, 1);
		//	m_MainPassCB.ShadowTexOffset[i].z = shadowTransform(3, 2);
		//	m_MainPassCB.ShadowTexOffset[i].w = 0.0f;
		//}

		float shadowMapSize = (float)CRenderer::GetShadowMapSize();
		m_MainPassCB.MaxBorderPadding = (shadowMapSize - 1.0f) / shadowMapSize;
		m_MainPassCB.MinBorderPadding = 1.0f / shadowMapSize;
		m_MainPassCB.ShadowBias = m_ShadowBias;
		m_MainPassCB.CascadeBlendArea = 0.005f;
		m_MainPassCB.ViualCascade = m_VisualCascade;
		m_MainPassCB.BlendCascade = m_BlendCascade;
		m_MainPassCB.PCFBlurForLoopStart = m_PCFBlurForLoopStart;
		m_MainPassCB.PCFBlurForLoopEnd = m_PCFBlurForLoopEnd;
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
		cubeFacePassCB.EyePosW = m_DCMCameras[i]->GetPosition3f();

		float dynamicCubeMapSize = (float)CRenderer::GetDynamicCubeMapSize();
		cubeFacePassCB.RenderTargetSize = XMFLOAT2(dynamicCubeMapSize, dynamicCubeMapSize);
		cubeFacePassCB.InvRenderTargetSize = XMFLOAT2(1.0f / dynamicCubeMapSize, 1.0f / dynamicCubeMapSize);

		auto currPassCB = CFrameResourceManager::GetCurrentFrameResource()->PassCB.get();

		// Cube map pass cbuffers are stored in elements from 1 + CascadNum to DCMResourcesIndex * 6.
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
		XMFLOAT3(0.0f, 1.0f,  0.0f), // +X
		XMFLOAT3(0.0f, 1.0f,  0.0f), // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f,  0.0f), // +Z
		XMFLOAT3(0.0f, 1.0f,  0.0f)	 // -Z
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

void CScene::SetSceneBoundingSphere(BoundingBox* Bounds)
{
	SetSceneBoundingSphere(Bounds->Extents.x, Bounds->Extents.z, Bounds->Center);
}

void CScene::SetSceneBoundingSphere(float Width, float Height, XMFLOAT3 Center)
{
	m_SceneBoundingSphere.Center = Center;
	m_SceneBoundingSphere.Radius = sqrtf(Width * Width + Height * Height);
}

void CScene::CreateSceneAABBPoints(XMVECTOR* SceneAABBPoints, const BoundingBox* SceneBoundingBox)
{
	const XMVECTORF32 extentsMap[] =
	{
		{  1.0f,  1.0f, -1.0f, 1.0f},
		{ -1.0f,  1.0f, -1.0f, 1.0f},
		{  1.0f, -1.0f, -1.0f, 1.0f},
		{ -1.0f, -1.0f, -1.0f, 1.0f},
		{  1.0f,  1.0f,  1.0f, 1.0f},
		{ -1.0f,  1.0f,  1.0f, 1.0f},
		{  1.0f, -1.0f,  1.0f, 1.0f},
		{ -1.0f, -1.0f,  1.0f, 1.0f},
	};

	XMVECTOR extents = XMLoadFloat3(&SceneBoundingBox->Extents);
	XMVECTOR center = XMLoadFloat3(&SceneBoundingBox->Center);

	for (int i = 0; i < 8; ++i)
	{
		SceneAABBPoints[i] = XMVectorMultiplyAdd(extentsMap[i], extents, center);
	}
}

void XM_CALLCONV CScene::ComputeFitCascadeCSMPassCB(XMMATRIX& CameraInvView)
{
	XMMATRIX lightView = m_DirLights[0]->GetView();

	XMVECTOR sceneAABBPointsLiS[8];
	CreateSceneAABBPoints(sceneAABBPointsLiS, &m_SceneBoundingBox);
	for (int i = 0; i < 8; ++i)
	{
		sceneAABBPointsLiS[i] = XMVector4Transform(sceneAABBPointsLiS[i], lightView);
	}

	float frustumIntervalBegin, frustumIntervalEnd;
	XMVECTOR lightOrthographicMin;
	XMVECTOR lightOrthographicMax;
	float cameraNearFarRange = m_MainCamera->GetFarZ() - m_MainCamera->GetNearZ();
	XMVECTOR worldUnitsPerTexel = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	static int cascadePartitionsZeroToOne[] = { 3, 15,100 }; // óvëfêî = CASCADE_NUM
	static int cascadePartitionsMax = 100;

	for (UINT i = 0; i < CRenderer::GetCascadNum(); ++i)
	{
		if (i == 0) frustumIntervalBegin = 0.0f;
		else frustumIntervalBegin = (float)cascadePartitionsZeroToOne[i - 1];
		frustumIntervalEnd = (float)cascadePartitionsZeroToOne[i];
		frustumIntervalBegin /= (float)cascadePartitionsMax;
		frustumIntervalEnd /= (float)cascadePartitionsMax;
		frustumIntervalBegin *= cameraNearFarRange;
		frustumIntervalEnd *= cameraNearFarRange;

		XMVECTOR frustumPoints[8];
		m_MainCamera->ComputeFrustumPointsFromCascadeInterval(frustumPoints, frustumIntervalBegin, frustumIntervalEnd);
		lightOrthographicMin = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
		lightOrthographicMax = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
		XMVECTOR tempfrustumPoint[8];
		for (int j = 0; j < 8; ++j)
		{
			frustumPoints[j] = XMVector4Transform(frustumPoints[j], CameraInvView); // FrustumPointsÇWorldãÛä‘Ç÷ì]ä∑
			tempfrustumPoint[j] = XMVector4Transform(frustumPoints[j], lightView);  // FrustumPointÇLightãÛä‘Ç÷ì]ä∑
			lightOrthographicMin = XMVectorMin(tempfrustumPoint[j], lightOrthographicMin);
			lightOrthographicMax = XMVectorMax(tempfrustumPoint[j], lightOrthographicMax);
		}

		float scaleDueToBlureAMT1f = (float)(m_PCFBlurSize * 2 + 1) / (float)CRenderer::GetShadowMapSize();
		XMVECTORF32 scaleDueToBlureAMT = { scaleDueToBlureAMT1f, scaleDueToBlureAMT1f, 0.0f, 0.0f };
		float normalizeByShadowMapSize1f = 1.0f / (float)CRenderer::GetShadowMapSize();
		XMVECTOR normalizeByShadowMapSize = XMVectorSet(normalizeByShadowMapSize1f, normalizeByShadowMapSize1f, 0.0f, 0.0f);
		XMVECTOR boarderOffset = lightOrthographicMax - lightOrthographicMin;
		boarderOffset *= XMVectorSet(0.5f, 0.5f, 0.5f, 0.5f);
		boarderOffset *= scaleDueToBlureAMT;
		lightOrthographicMax += boarderOffset;
		lightOrthographicMin -= boarderOffset;
		worldUnitsPerTexel = lightOrthographicMax - lightOrthographicMin;
		worldUnitsPerTexel *= normalizeByShadowMapSize;

		float lightOrthographicMinZ, lightOrthographicMaxZ;
		lightOrthographicMinZ = XMVectorGetZ(lightOrthographicMin);
		lightOrthographicMaxZ = XMVectorGetZ(lightOrthographicMax);
		//lightOrthographicMin /= worldUnitsPerTexel;
		//lightOrthographicMin = XMVectorFloor(lightOrthographicMin);
		//lightOrthographicMin *= worldUnitsPerTexel;
		//lightOrthographicMax /= worldUnitsPerTexel;
		//lightOrthographicMax = XMVectorFloor(lightOrthographicMax);
		//lightOrthographicMax *= worldUnitsPerTexel;

		float nearPlane = 0.0f;
		float farPlane = 10000.0f;
		ComputeNearAndFarInCSM(nearPlane, farPlane, lightOrthographicMin, lightOrthographicMax, sceneAABBPointsLiS);
		//nearPlane = nearPlane * (i + 1) / 2 + lightOrthographicMinZ;
		//farPlane = farPlane * (i + 1) / 2 + lightOrthographicMaxZ;
		nearPlane = nearPlane + lightOrthographicMinZ;
		farPlane = farPlane + lightOrthographicMaxZ;
		XMMATRIX shadowTransform = m_DirLights[0]->ComputeShadowTransformFromLightOrthographicAndNearFar(i, nearPlane, farPlane, lightOrthographicMin, lightOrthographicMax);
		XMFLOAT4X4 shadowTransform4x4;
		XMStoreFloat4x4(&shadowTransform4x4, shadowTransform);
		m_MainPassCB.ShadowTexScale[i].x = shadowTransform4x4(0, 0);
		m_MainPassCB.ShadowTexScale[i].y = shadowTransform4x4(1, 1);
		m_MainPassCB.ShadowTexScale[i].z = shadowTransform4x4(2, 2);
		m_MainPassCB.ShadowTexScale[i].w = 1.0f;

		m_MainPassCB.ShadowTexOffset[i].x = shadowTransform4x4(3, 0);
		m_MainPassCB.ShadowTexOffset[i].y = shadowTransform4x4(3, 1);
		m_MainPassCB.ShadowTexOffset[i].z = shadowTransform4x4(3, 2);
		m_MainPassCB.ShadowTexOffset[i].w = 0.0f;

		m_CascadePartitionsFrustum[i] = frustumIntervalEnd;
		m_MainPassCB.CascadeFrustumsEyeSpaceDepthsFloat4[i].x = m_CascadePartitionsFrustum[i];
	}
	m_MainPassCB.CascadeFrustumsEyeSpaceDepthsFloat.x = m_CascadePartitionsFrustum[0];
	m_MainPassCB.CascadeFrustumsEyeSpaceDepthsFloat.y = m_CascadePartitionsFrustum[1];
	m_MainPassCB.CascadeFrustumsEyeSpaceDepthsFloat.z = m_CascadePartitionsFrustum[2];
}

struct Triangle
{
	XMVECTOR Point[3];
	BOOL Culled;
};

void XM_CALLCONV CScene::ComputeNearAndFarInCSM(float& Near, float& Far, XMVECTOR LightOrthographicMin, XMVECTOR LightOrthographicMax, XMVECTOR* SceneAABBPointsLiS)
{
	// Initialize the near and far planes
	Near = FLT_MAX;
	Far = -FLT_MAX;

	Triangle triangleList[16];
	int triangleCnt = 1;

	triangleList[0].Point[0] = SceneAABBPointsLiS[0];
	triangleList[0].Point[1] = SceneAABBPointsLiS[1];
	triangleList[0].Point[2] = SceneAABBPointsLiS[2];
	triangleList[0].Culled = false;

	// These are the indices used to tesselate an AABB into a list of triangles.
	static const int aabbTriIndexes[] =
	{
		0,1,2,  1,2,3,
		4,5,6,  5,6,7,
		0,2,4,  2,4,6,
		1,3,5,  3,5,7,
		0,1,4,  1,4,5,
		2,3,6,  3,6,7
	};

	int pointPassesCollision[3];

	// At a high level: 
	// 1. Iterate over all 12 triangles of the AABB.  
	// 2. Clip the triangles against each plane. Create new triangles as needed.
	// 3. Find the min and max z values as the near and far plane.

	//This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.

	float lightOrthographicMinX = XMVectorGetX(LightOrthographicMin);
	float lightOrthographicMaxX = XMVectorGetX(LightOrthographicMax);
	float lightOrthographicMinY = XMVectorGetY(LightOrthographicMin);
	float lightOrthographicMaxY = XMVectorGetY(LightOrthographicMax);

	for (int aabbTriIter = 0; aabbTriIter < 12; ++aabbTriIter)
	{

		triangleList[0].Point[0] = SceneAABBPointsLiS[aabbTriIndexes[aabbTriIter * 3 + 0]];
		triangleList[0].Point[1] = SceneAABBPointsLiS[aabbTriIndexes[aabbTriIter * 3 + 1]];
		triangleList[0].Point[2] = SceneAABBPointsLiS[aabbTriIndexes[aabbTriIter * 3 + 2]];
		triangleCnt = 1;
		triangleList[0].Culled = FALSE;

		// Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
		//add them to the list.
		for (int frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter)
		{

			float edge;
			int component;

			if (frustumPlaneIter == 0)
			{
				edge = lightOrthographicMinX; // todo make float temp
				component = 0;
			}
			else if (frustumPlaneIter == 1)
			{
				edge = lightOrthographicMaxX;
				component = 0;
			}
			else if (frustumPlaneIter == 2)
			{
				edge = lightOrthographicMinY;
				component = 1;
			}
			else
			{
				edge = lightOrthographicMaxY;
				component = 1;
			}

			for (int triIter = 0; triIter < triangleCnt; ++triIter)
			{
				// We don't delete triangles, so we skip those that have been Culled.
				if (!triangleList[triIter].Culled)
				{
					int insideVertCount = 0;
					XMVECTOR tempOrder;
					// Test against the correct frustum plane.
					// This could be written more compactly, but it would be harder to understand.

					if (frustumPlaneIter == 0)
					{
						for (int triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetX(triangleList[triIter].Point[triPtIter]) >
								XMVectorGetX(LightOrthographicMin))
							{
								pointPassesCollision[triPtIter] = 1;
							}
							else
							{
								pointPassesCollision[triPtIter] = 0;
							}
							insideVertCount += pointPassesCollision[triPtIter];
						}
					}
					else if (frustumPlaneIter == 1)
					{
						for (int triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetX(triangleList[triIter].Point[triPtIter]) <
								XMVectorGetX(LightOrthographicMax))
							{
								pointPassesCollision[triPtIter] = 1;
							}
							else
							{
								pointPassesCollision[triPtIter] = 0;
							}
							insideVertCount += pointPassesCollision[triPtIter];
						}
					}
					else if (frustumPlaneIter == 2)
					{
						for (int triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetY(triangleList[triIter].Point[triPtIter]) >
								XMVectorGetY(LightOrthographicMin))
							{
								pointPassesCollision[triPtIter] = 1;
							}
							else
							{
								pointPassesCollision[triPtIter] = 0;
							}
							insideVertCount += pointPassesCollision[triPtIter];
						}
					}
					else
					{
						for (int triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetY(triangleList[triIter].Point[triPtIter]) <
								XMVectorGetY(LightOrthographicMax))
							{
								pointPassesCollision[triPtIter] = 1;
							}
							else
							{
								pointPassesCollision[triPtIter] = 0;
							}
							insideVertCount += pointPassesCollision[triPtIter];
						}
					}

					// Move the points that pass the frustum test to the begining of the array.
					if (pointPassesCollision[1] && !pointPassesCollision[0])
					{
						tempOrder = triangleList[triIter].Point[0];
						triangleList[triIter].Point[0] = triangleList[triIter].Point[1];
						triangleList[triIter].Point[1] = tempOrder;
						pointPassesCollision[0] = TRUE;
						pointPassesCollision[1] = FALSE;
					}
					if (pointPassesCollision[2] && !pointPassesCollision[1])
					{
						tempOrder = triangleList[triIter].Point[1];
						triangleList[triIter].Point[1] = triangleList[triIter].Point[2];
						triangleList[triIter].Point[2] = tempOrder;
						pointPassesCollision[1] = TRUE;
						pointPassesCollision[2] = FALSE;
					}
					if (pointPassesCollision[1] && !pointPassesCollision[0])
					{
						tempOrder = triangleList[triIter].Point[0];
						triangleList[triIter].Point[0] = triangleList[triIter].Point[1];
						triangleList[triIter].Point[1] = tempOrder;
						pointPassesCollision[0] = TRUE;
						pointPassesCollision[1] = FALSE;
					}

					if (insideVertCount == 0)
					{ // All points failed. We're done,  
						triangleList[triIter].Culled = true;
					}
					else if (insideVertCount == 1)
					{// One point passed. Clip the triangle against the Frustum plane
						triangleList[triIter].Culled = false;

						// 
						XMVECTOR vert0ToVert1 = triangleList[triIter].Point[1] - triangleList[triIter].Point[0];
						XMVECTOR vert0ToVert2 = triangleList[triIter].Point[2] - triangleList[triIter].Point[0];

						// Find the collision ratio.
						FLOAT hitPointTimeRatio = edge - XMVectorGetByIndex(triangleList[triIter].Point[0], component);
						// Calculate the distance along the vector as ratio of the hit ratio to the component.
						FLOAT distanceAlongVector01 = hitPointTimeRatio / XMVectorGetByIndex(vert0ToVert1, component);
						FLOAT distanceAlongVector02 = hitPointTimeRatio / XMVectorGetByIndex(vert0ToVert2, component);
						// Add the point plus a percentage of the vector.
						vert0ToVert1 *= distanceAlongVector01;
						vert0ToVert1 += triangleList[triIter].Point[0];
						vert0ToVert2 *= distanceAlongVector02;
						vert0ToVert2 += triangleList[triIter].Point[0];

						triangleList[triIter].Point[1] = vert0ToVert2;
						triangleList[triIter].Point[2] = vert0ToVert1;
					}
					else if (insideVertCount == 2)
					{ // 2 in  // tesselate into 2 triangles


						// Copy the triangle\(if it exists) after the current triangle out of
						// the way so we can override it with the new triangle we're inserting.
						triangleList[triangleCnt] = triangleList[triIter + 1];

						triangleList[triIter].Culled = false;
						triangleList[triIter + 1].Culled = false;

						// Get the vector from the outside point into the 2 inside points.
						XMVECTOR vert2ToVert0 = triangleList[triIter].Point[0] - triangleList[triIter].Point[2];
						XMVECTOR vert2ToVert1 = triangleList[triIter].Point[1] - triangleList[triIter].Point[2];

						// Get the hit point ratio.
						FLOAT hitPointTime_2_0 = edge - XMVectorGetByIndex(triangleList[triIter].Point[2], component);
						FLOAT distanceAlongVector_2_0 = hitPointTime_2_0 / XMVectorGetByIndex(vert2ToVert0, component);
						// Calcaulte the new vert by adding the percentage of the vector plus point 2.
						vert2ToVert0 *= distanceAlongVector_2_0;
						vert2ToVert0 += triangleList[triIter].Point[2];

						// Add a new triangle.
						triangleList[triIter + 1].Point[0] = triangleList[triIter].Point[0];
						triangleList[triIter + 1].Point[1] = triangleList[triIter].Point[1];
						triangleList[triIter + 1].Point[2] = vert2ToVert0;

						//Get the hit point ratio.
						FLOAT hitPointTime_2_1 = edge - XMVectorGetByIndex(triangleList[triIter].Point[2], component);
						FLOAT distanceAlongVector_2_1 = hitPointTime_2_1 / XMVectorGetByIndex(vert2ToVert1, component);
						vert2ToVert1 *= distanceAlongVector_2_1;
						vert2ToVert1 += triangleList[triIter].Point[2];
						triangleList[triIter].Point[0] = triangleList[triIter + 1].Point[1];
						triangleList[triIter].Point[1] = triangleList[triIter + 1].Point[2];
						triangleList[triIter].Point[2] = vert2ToVert1;
						// Cncrement triangle count and skip the triangle we just inserted.
						++triangleCnt;
						++triIter;
					}
					else
					{ // all in
						triangleList[triIter].Culled = false;
					}
				}// end if !Culled loop            
			}
		}
		for (int index = 0; index < triangleCnt; ++index)
		{
			if (!triangleList[index].Culled)
			{
				// Set the near and far plan and the min and max z values respectivly.
				for (int vertind = 0; vertind < 3; ++vertind)
				{
					float triangleCoordZ = XMVectorGetZ(triangleList[index].Point[vertind]);
					if (Near > triangleCoordZ)
					{
						Near = triangleCoordZ;
					}
					if (Far < triangleCoordZ)
					{
						Far = triangleCoordZ;
					}
				}
			}
		}
	}
}