#include "Manager.h"
#include "Scene.h"
#include "DirectionalLight.h"

using namespace DirectX;

void CDirLight::Update(const GameTimer& GlobalTimer)
{
	m_Rotation.y += 0.2f * GlobalTimer.DeltaTime();
}

XMFLOAT3 CDirLight::ComputeDirection3f()
{
	ComputeDirection();
	return m_Direction;
}

DirectX::XMVECTOR XM_CALLCONV CDirLight::ComputeDirection()
{
	XMVECTOR direction = XMLoadFloat3(&m_BaseDirection);
	direction = XMVector3TransformNormal(direction, XMMatrixRotationY(m_Rotation.y));
	XMStoreFloat3(&m_Direction, direction);
	return direction;
}

XMMATRIX XM_CALLCONV CDirLight::ComputeShadowTransform(BoundingSphere* SceneBounds)
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = ComputeDirection();
	XMVECTOR lightPos = -2.0f * SceneBounds->Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&SceneBounds->Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&m_Position, lightPos);
	XMStoreFloat4x4(&m_View, lightView);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

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
	XMStoreFloat4x4(&m_Proj, lightProj);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f,  0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 0.0f,
		0.5f,  0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj * T;

	return S;
}