#pragma once

#include "Light.h"

class CDirLight :public CLight
{
private:
	DirectX::XMFLOAT3 m_BaseDirection = { 0.0f, -1.0f, 0.0f };
	DirectX::XMFLOAT3 m_Direction = { 0.0f, -1.0f, 0.0f };

	bool m_Move = false;

public:
	CDirLight() = default;
	~CDirLight() = default;

	void Update(const GameTimer& GlobalTimer)override;
	void UpdateImGui(const GameTimer& GlobalTimer)override;

	void SetBaseDirection(DirectX::XMFLOAT3 BaseDirection) { m_BaseDirection = BaseDirection; }

	DirectX::XMFLOAT3 GetDirection3f()const { return m_Direction; }

	DirectX::XMFLOAT3 ComputeDirection3f();

	DirectX::XMVECTOR XM_CALLCONV ComputeDirection();
	DirectX::XMMATRIX XM_CALLCONV ComputeLightView(DirectX::BoundingSphere* SceneBounds)override;
};