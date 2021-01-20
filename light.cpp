#include "Renderer.h"
#include "Light.h"

using namespace std;
using namespace DirectX;

void CLight::Init()
{
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