#include "Manager.h"
#include "Scene.h"
#include "Camera.h"
#include "DirectionalLight.h"

using namespace std;
using namespace DirectX;

void CDirLight::Update(const GameTimer& GlobalTimer)
{
	if (m_Move)
	{
		m_Rotation.y += 0.2f * GlobalTimer.DeltaTime();
	}
}

XMFLOAT3 CDirLight::ComputeDirection3f()
{
	ComputeDirection();
	return m_Direction;
}

XMVECTOR XM_CALLCONV CDirLight::ComputeDirection()
{
	XMVECTOR direction = XMLoadFloat3(&m_BaseDirection);
	direction = XMVector3TransformNormal(direction, XMMatrixRotationY(m_Rotation.y));
	XMStoreFloat3(&m_Direction, direction);
	return direction;
}

XMMATRIX XM_CALLCONV CDirLight::ComputeLightView(BoundingSphere* SceneBounds)
{
	XMVECTOR lightDir = ComputeDirection();
	XMVECTOR lightPos = -2.0f * SceneBounds->Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&SceneBounds->Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&m_Position, lightPos);
	XMStoreFloat4x4(&m_View, lightView);

	return lightView;
}

XMMATRIX XM_CALLCONV CDirLight::ComputeShadowTransformWithSceneBounds(BoundingSphere* SceneBounds)
{
	XMMATRIX lightView = GetView();

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(XMLoadFloat3(&SceneBounds->Center), lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - SceneBounds->Radius;
	float b = sphereCenterLS.y - SceneBounds->Radius;
	float n = sphereCenterLS.z - SceneBounds->Radius;
	float r = sphereCenterLS.x + SceneBounds->Radius;
	float t = sphereCenterLS.y + SceneBounds->Radius;
	float f = sphereCenterLS.z + SceneBounds->Radius;

	m_NearZ = n;
	m_FarZ = f;

	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	XMStoreFloat4x4(&m_Proj[0], lightProj);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f,  0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 0.0f,
		0.5f,  0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj * T;

	return S;
}

XMMATRIX XM_CALLCONV CDirLight::ComputeShadowTransformWithCameraFrustum(BoundingSphere* SceneBounds, XMVECTOR FrustumPoints[8])
{
	XMMATRIX lightView = GetView();

	XMVECTOR min, max;
	min = max = FrustumPoints[0] = XMVector3TransformCoord(FrustumPoints[0], GetView());

	for (int i = 1; i < 8; ++i)
	{
		FrustumPoints[i] = XMVector3TransformCoord(FrustumPoints[i], lightView);
		min = XMVectorMin(min, FrustumPoints[i]);
		max = XMVectorMax(max, FrustumPoints[i]);
	}

	// lightProj
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
		XMVectorGetX(min), XMVectorGetX(max),
		XMVectorGetY(min), XMVectorGetY(max),
		XMVectorGetZ(min), XMVectorGetZ(max));
	XMStoreFloat4x4(&m_Proj[0], lightProj);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f,  0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 0.0f,
		0.5f,  0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj * T;

	return S;
}

void CDirLight::ComputeShadowTransformWithCameraFrustum(vector<XMMATRIX>& ShadowTransforms, BoundingSphere* SceneBounds, vector<vector<XMVECTOR>>& FrustumPoints)
{
	for (UINT i = 0; i < ShadowTransforms.size(); ++i)
	{
		ShadowTransforms[i] = ComputeShadowTransformWithCameraFrustumForEachCascade(FrustumPoints[i], i);
	}
}

XMMATRIX XM_CALLCONV CDirLight::ComputeShadowTransformWithCameraFrustumForEachCascade(vector<XMVECTOR>& FrustumPoints, int CascadeIndex)
{
	// SceneÇÃãÖñ è„ÇÃlightView
	XMMATRIX lightView = GetView();

	XMVECTOR min, max;
	//XMVECTOR frustumCenter = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//for (int i = 0; i < 8; ++i)
	//{
	//	FrustumPoints[i] = XMVector3TransformCoord(FrustumPoints[i], lightView);
	//	frustumCenter += FrustumPoints[i];
	//}
	//frustumCenter /= 8.0f;

	//XMVECTOR radius = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//for (UINT j = 0; j < 8; ++j)
	//{
	//	XMVECTOR distance = XMVector3Length(FrustumPoints[j] - frustumCenter);
	//	radius = XMVectorMax(radius, distance);
	//}
	//radius = XMVectorCeiling(radius * 16.0f) / 16.0f;
	//max = frustumCenter + radius;
	//min = frustumCenter - radius;

	min = max = FrustumPoints[0] = XMVector3TransformCoord(FrustumPoints[0], lightView);
	for (int i = 1; i < 8; ++i)
	{
		FrustumPoints[i] = XMVector3TransformCoord(FrustumPoints[i], lightView);
		min = XMVectorMin(min, FrustumPoints[i]);
		max = XMVectorMax(max, FrustumPoints[i]);
	}

	float minZ = XMVectorGetZ(min);
	float maxZ = XMVectorGetZ(max);
	// lightProj
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
		XMVectorGetX(min), XMVectorGetX(max),
		XMVectorGetY(min), XMVectorGetY(max),
		minZ, maxZ);
		//XMVectorGetZ(min), XMVectorGetZ(max));
	//lightProj = XMMatrixOrthographicOffCenterLH(
	//	-57.4f, 7.3f,
	//	-27.6f, 40.3f,
	//	XMVectorGetZ(min), XMVectorGetZ(max));
		//-149.5f, -23.2f);
	XMStoreFloat4x4(&m_Proj[CascadeIndex], lightProj);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f,  0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 0.0f,
		0.5f,  0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightProj * T;

	return S;
}

void CDirLight::UpdateImGui(const GameTimer& GlobalTimer)
{
	static bool showClose = true;

	if (showClose && m_Name == "DirLight00")
	{
		ImGui::SetNextWindowPos(ImVec2((float)DX12App::GetApp()->GetWindowWidth() - 120, 500), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(100, 65), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"DirLight", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		ImGui::Checkbox(u8"Move", &m_Move);
		ImGui::PopItemWidth();
		ImGui::End();
	}
}