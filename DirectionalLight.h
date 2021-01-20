#pragma once

#include "Light.h"

class CDirLight :public CLight
{
private:
	DirectX::XMFLOAT3 m_BaseDirection = { 0.0f, -1.0f, 0.0f };
	DirectX::XMFLOAT3 m_Direction = { 0.0f, -1.0f, 0.0f };

public:
	CDirLight() = default;
	~CDirLight() = default;

	void Init()override {}
	void Update(const GameTimer& GlobalTimer)override;

	void SetBaseDirection(DirectX::XMFLOAT3 BaseDirection) { m_BaseDirection = BaseDirection; }

	DirectX::XMFLOAT3 GetDirection3f()const { return m_Direction; }

	DirectX::XMFLOAT3 ComputeDirection3f();

	DirectX::XMVECTOR XM_CALLCONV ComputeDirection();
	DirectX::XMMATRIX XM_CALLCONV ComputeLightView(DirectX::BoundingSphere* SceneBounds)override;
	DirectX::XMMATRIX XM_CALLCONV ComputeShadowTransformWithSceneBounds(DirectX::BoundingSphere* SceneBounds)override;
	DirectX::XMMATRIX XM_CALLCONV ComputeShadowTransformWithCameraFrustum(DirectX::BoundingSphere* SceneBounds, DirectX::XMVECTOR FrustumPoints[8])override;
};