#pragma once

#include "GameObject.h"

class CCamera :public CGameObject
{
protected:
	DirectX::XMFLOAT3 m_Right;
	DirectX::XMFLOAT3 m_Up;
	DirectX::XMFLOAT3 m_Look;

	// Cache frustum properties.
	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;
	float m_Aspect = 0.0f;
	float m_FovY = 0.0f;
	float m_NearWindowHeight = 0.0f;
	float m_FarWindowHeight = 0.0f;

	bool m_ViewDirty = true;

	// Cache View/Proj matrices.
	DirectX::XMFLOAT4X4 m_View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_Proj = MathHelper::Identity4x4();

	DirectX::BoundingFrustum m_Bounds;

public:
	CCamera() = default;
	~CCamera() = default;

	//virtual void Init()override = 0;
	virtual void LateUpdate(const GameTimer& GlobalTimer)override;
	virtual void UpdateViewMatrix();

	void CreateFrustumBounds();
	void ComputeFrustumPointsInWorldSpace(DirectX::XMVECTOR FrustumPoints[8]);
	void ComputeFrustumPointsInWorldSpace(DirectX::XMVECTOR FrustumPoints[8], const DirectX::XMMATRIX& InvView);

	// Get frustum properties.
	float GetNearZ()const { return m_NearZ; }
	float GetFarZ()const { return m_FarZ; }
	float GetAspect()const { return m_Aspect; }
	float GetFovY()const { return m_FovY; }
	float GetFovX()const;

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth()const { return m_Aspect * m_NearWindowHeight; }
	float GetNearWindowHeight()const { return m_NearWindowHeight; }
	float GetFarWindowWidth()const { return m_Aspect * m_FarWindowHeight; }
	float GetFarWindowHeight()const { return m_FarWindowHeight; }

	// Set frustum.
	void ComputeProjectionMatrix(float fovY, float aspect, float zn, float zf);

	// Define camera space via LookAt parameters.
	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

	// Get View/Proj matrices.
	DirectX::XMFLOAT4X4 GetView4x4f()const;
	DirectX::XMFLOAT4X4 GetProj4x4f()const { return m_Proj; }

	DirectX::XMMATRIX XM_CALLCONV GetView()const;
	DirectX::XMMATRIX XM_CALLCONV GetProj()const { return XMLoadFloat4x4(&m_Proj); }
};