#include "Renderer.h"
#include "Light.h"

using namespace std;
using namespace DirectX;

void CLight::Init()
{
	//m_View.resize(CRenderer::GetCascadNum());
	m_Proj.resize(CRenderer::GetCascadNum());
}

DirectX::XMFLOAT4X4 CLight::ComputeLightView4x4(DirectX::BoundingSphere * SceneBounds)
{
	XMStoreFloat4x4(&m_View, ComputeLightView(SceneBounds));
	return m_View;
}

XMFLOAT4X4 CLight::ComputeShadowTransformWithSceneBounds4x4(BoundingSphere* SceneBounds)
{
	XMFLOAT4X4 shadowTransform;
	XMStoreFloat4x4(&shadowTransform, ComputeShadowTransformWithSceneBounds(SceneBounds));
	return shadowTransform;
}

XMMATRIX XM_CALLCONV CLight::ComputeShadowTransformFromLightOrthographicAndNearFar(int CascadeIndex, float Near, float Far, XMVECTOR LightOrthographicMin, XMVECTOR LightOrthographicMax)
{
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
		XMVectorGetX(LightOrthographicMin), XMVectorGetX(LightOrthographicMax),
		XMVectorGetY(LightOrthographicMin), XMVectorGetY(LightOrthographicMax),
		Near, Far);
	XMStoreFloat4x4(&m_Proj[CascadeIndex], lightProj);

	XMMATRIX T(
		0.5f,  0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 0.0f,
		0.5f,  0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightProj * T;

	return S;
}