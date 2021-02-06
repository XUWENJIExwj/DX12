#pragma once

#include "GameObject.h"

class CLight :public CGameObject
{
protected:
	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;

	DirectX::XMFLOAT4X4              m_View = MathHelper::Identity4x4();
	std::vector<DirectX::XMFLOAT4X4> m_Proj;

	DirectX::XMFLOAT3 m_Strength = { 0.5f, 0.5f, 0.5f };

public:
	CLight() = default;
	~CLight() = default;

	void Init()override;

	void SetStrength(DirectX::XMFLOAT3 Strength) { m_Strength = Strength; }

	float GetNearZ()const { return m_NearZ; }
	float GetFarZ()const { return m_FarZ; }

	DirectX::XMFLOAT4X4 GetView4x4f()const { return m_View; }
	DirectX::XMFLOAT4X4 GetProj4x4f(int CascadeIndex = 0)const { return m_Proj[CascadeIndex]; }
	DirectX::XMFLOAT3   GetStrength3f()const { return m_Strength; }

	DirectX::XMMATRIX XM_CALLCONV GetView()const { return XMLoadFloat4x4(&m_View); }
	DirectX::XMMATRIX XM_CALLCONV GetProj(int CascadeIndex = 0)const { return XMLoadFloat4x4(&m_Proj[CascadeIndex]); }

	DirectX::XMFLOAT4X4 ComputeLightView4x4(DirectX::BoundingSphere* SceneBounds);

	virtual DirectX::XMMATRIX XM_CALLCONV ComputeLightView(DirectX::BoundingSphere* SceneBounds) = 0;
	virtual DirectX::XMMATRIX XM_CALLCONV ComputeShadowTransformFromLightOrthographicAndNearFar(int CascadeIndex, float Near, float Far, DirectX::FXMVECTOR LightOrthographicMin, DirectX::FXMVECTOR LightOrthographicMax);
};