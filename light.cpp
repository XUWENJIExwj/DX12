#include "Light.h"

using namespace DirectX;

XMFLOAT4X4 CLight::ComputeShadowTransform4x4(BoundingSphere* SceneBounds)
{
	XMFLOAT4X4 shadowTransform;
	XMStoreFloat4x4(&shadowTransform, ComputeShadowTransform(SceneBounds));
	return shadowTransform;
}