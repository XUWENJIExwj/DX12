#include "Manager.h"
#include "Renderer.h"
#include "Scene.h"
#include "GameObject.h"

using namespace std;
using namespace DirectX;

CGameObject::CGameObject()
{
	XMStoreFloat4(&m_Quaternion, XMQuaternionIdentity());
}

void CGameObject::OnResize()
{
	DX12App* app = DX12App::GetApp();
	float width = (float)app->GetWindowWidth();
	float height = (float)app->GetWindowHeight();
	float aspectRatio = app->GetAspectRatio();

	if (aspectRatio >= 1.0f)
	{
		m_Scale.y = m_ScaleWindowXYRatio.y * height;
		m_Scale.x = m_Scale.y * m_ScaleXYRatio;
	}
	else
	{
		m_Scale.x = m_ScaleWindowXYRatio.x * width;
		m_Scale.y = m_Scale.x / m_ScaleXYRatio;
	}
}

void CGameObject::Init2DScaleRatio()
{
	DX12App* app = DX12App::GetApp();
	float width = (float)app->GetWindowWidth();
	float height = (float)app->GetWindowHeight();

	m_ScaleWindowXYRatio = XMFLOAT2(m_Scale.x / width, m_Scale.y / height);
	m_ScaleXYRatio = m_Scale.x / m_Scale.y;
}

void CGameObject::CreateDynamicCubeMapResources(const GameTimer & GlobalTimer, int DCMResourcesIndex)
{
	CManager::GetScene()->SetUpDynamicCubeMapCamera(m_Position);
	CManager::GetScene()->UpdateDynamicCubeMapPassCB(GlobalTimer, DCMResourcesIndex);

	auto dcmCameras = CManager::GetScene()->GetDynamicCubeMapCameras();
	auto allGameObjectsWithLayer = CManager::GetScene()->GetAllRenderLayers();

	CRenderer::SetUpBeforeCreateAllDynamicCubeMapResources(DCMResourcesIndex);

	for (int i = 0; i < (int)dcmCameras.size(); ++i)
	{
		CRenderer::SetUpBeforeCreateEachDynamicCubeMapResource(DCMResourcesIndex, i);
		DrawDynamicCubeMapScene(allGameObjectsWithLayer);
	}
	CRenderer::CompleteCreateDynamicCubeMapResources(DCMResourcesIndex);
}

void CGameObject::UpdateBoundingBox()
{
	if (m_NumFramesDirty > 0)
	{
		ComputeBoundingBox();
	}
}

void CGameObject::ComputeBoundingBox()
{
	m_BoundingBox = CGeoShapeManager::GetBoxBounds(m_BoundsName);
	m_BoundingBox.Transform(m_BoundingBox, GetWorldMatrix());
}

XMFLOAT3 CGameObject::GetRightWithRotation3f()const
{
	XMMATRIX rot;
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

	XMFLOAT4X4 r;
	XMStoreFloat4x4(&r, rot);

	XMFLOAT3 right;
	right.x = r(0, 0);
	right.y = r(0, 1);
	right.z = r(0, 2);

	return right;
}

XMVECTOR CGameObject::GetRightWithRotation()const
{
	return XMLoadFloat3(&GetRightWithRotation3f());
}

XMFLOAT3 CGameObject::GetUpWithRotation3f()const
{
	XMMATRIX rot;
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

	XMFLOAT4X4 r;
	XMStoreFloat4x4(&r, rot);

	XMFLOAT3 up;
	up.x = r(1, 0);
	up.y = r(1, 1);
	up.z = r(1, 2);

	return up;
}

XMVECTOR CGameObject::GetUpWithRotation()const
{
	return XMLoadFloat3(&GetUpWithRotation3f());
}

XMFLOAT3 CGameObject::GetForwardWithRotation3f()const
{
	XMMATRIX rot;
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

	XMFLOAT4X4 r;
	XMStoreFloat4x4(&r, rot);

	XMFLOAT3 forward;
	forward.x = r(2, 0);
	forward.y = r(2, 1);
	forward.z = r(2, 2);

	return forward;
}

XMVECTOR CGameObject::GetForwardWithRotation()const
{
	return XMLoadFloat3(&GetForwardWithRotation3f());
}

XMFLOAT3 CGameObject::GetRightWithQuaternion3f()const
{
	XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&m_Quaternion));

	XMFLOAT4X4 r;
	XMStoreFloat4x4(&r, rot);

	XMFLOAT3 right;
	right.x = r(0, 0);
	right.y = r(0, 1);
	right.z = r(0, 2);

	return right;
}

XMVECTOR CGameObject::GetRightWithQuaternion()const
{
	return XMLoadFloat3(&GetRightWithQuaternion3f());
}

XMFLOAT3 CGameObject::GetUpWithQuaternion3f()const
{
	XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&m_Quaternion));

	XMFLOAT4X4 r;
	XMStoreFloat4x4(&r, rot);

	XMFLOAT3 up;
	up.x = r(1, 0);
	up.y = r(1, 1);
	up.z = r(1, 2);

	return up;
}

XMVECTOR CGameObject::GetUpWithQuaternion()const
{
	return XMLoadFloat3(&GetUpWithQuaternion3f());
}

XMFLOAT3 CGameObject::GetForwardWithQuaternion3f()const
{
	XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&m_Quaternion));

	XMFLOAT4X4 r;
	XMStoreFloat4x4(&r, rot);

	XMFLOAT3 forward;
	forward.x = r(2, 0);
	forward.y = r(2, 1);
	forward.z = r(2, 2);

	return forward;
}

XMVECTOR CGameObject::GetForwardWithQuaternion()const
{
	return XMLoadFloat3(&GetForwardWithQuaternion3f());
}

DirectX::XMFLOAT4X4 CGameObject::ComputeWorldMatrix4x4()const
{
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, ComputeWorldMatrix());

	return world;
}

XMFLOAT4X4 CGameObject::Compute2DWVPMatrix4x4() const
{
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, Compute2DWVPMatrix());

	return world;
}

DirectX::XMMATRIX CGameObject::ComputeWorldMatrix()const
{
	XMMATRIX scl = XMMatrixScalingFromVector(GetScale());
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(GetRotation());
	XMMATRIX trs = XMMatrixTranslationFromVector(GetPosition());

	return scl * rot * trs;
}

XMMATRIX XM_CALLCONV CGameObject::Compute2DWVPMatrix()const
{
	DX12App* app = DX12App::GetApp();
	float width = (float)app->GetWindowWidth();
	float height = (float)app->GetWindowHeight();

	XMMATRIX world = ComputeWorldMatrix();
	XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, width, 0.0f, height, 0.0f, 1.0f);
	//XMMATRIX projection = XMMatrixOrthographicLH(width, height, 0.0f, 1.0f);

	world *= projection;
	
	return world;
}

bool CGameObject::Destroy()
{
	if (m_Destroy)
	{
		UINT objCBIndex = m_ObjCBIndex;

		auto allGameObjects = CManager::GetScene()->GetAllGameObjects();
		auto iteratorAll = find_if(
			allGameObjects.begin(), allGameObjects.end(),
			[objCBIndex](CGameObject* GameObject) { return GameObject->GetObjCBIndex() == objCBIndex; });
		auto listEmptyReference = CManager::GetScene()->GetListEmptyReference();
		listEmptyReference->objIteratorStore.push(iteratorAll);
		listEmptyReference->objCBIndexStore.push(m_ObjCBIndex);

		auto gameObjectsWithLayer = CManager::GetScene()->GetRenderLayer(this->GetGameObjectLayer());
		auto iteratorLayer=find_if(
			gameObjectsWithLayer.begin(), gameObjectsWithLayer.end(),
			[objCBIndex](CGameObject* GameObject) { return GameObject->GetObjCBIndex() == objCBIndex; });
		gameObjectsWithLayer.erase(iteratorLayer);

		Uninit();
		delete this;
		*iteratorAll = nullptr;
		return true;
	}
	return false;
}