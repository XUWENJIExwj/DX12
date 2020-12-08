#pragma once

#include "GameObject.h"

class CCamera :public CGameObject
{
private:
	DirectX::XMFLOAT3 mRight;
	DirectX::XMFLOAT3 mUp;
	DirectX::XMFLOAT3 mLook;

	// Cache frustum properties.
	float mNearZ = 0.0f;
	float mFarZ = 0.0f;
	float mAspect = 0.0f;
	float mFovY = 0.0f;
	float mNearWindowHeight = 0.0f;
	float mFarWindowHeight = 0.0f;

	bool mViewDirty = true;

	// Cache View/Proj matrices.
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();

public:

	CCamera() = default;
	~CCamera() = default;

	void Init()override;
	void Update(const GameTimer& gt)override;
	void LateUpdate(const GameTimer& gt)override;

	// Get frustum properties.
	float GetNearZ()const { return mNearZ; }
	float GetFarZ()const { return mFarZ; }
	float GetAspect()const { return mAspect; }
	float GetFovY()const { return mFovY; }
	float GetFovX()const;

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth()const { return mAspect * mNearWindowHeight; }
	float GetNearWindowHeight()const { return mNearWindowHeight; }
	float GetFarWindowWidth()const{ return mAspect * mFarWindowHeight; }
	float GetFarWindowHeight()const { return mFarWindowHeight; }
	
	// Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf);

	// Define camera space via LookAt parameters.
	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

	// Get View/Proj matrices.
	DirectX::XMMATRIX GetView()const;
	DirectX::XMMATRIX GetProj()const { return XMLoadFloat4x4(&mProj); }

	DirectX::XMFLOAT4X4 GetView4x4f()const;
	DirectX::XMFLOAT4X4 GetProj4x4f()const { return mProj; }

	// Strafe/Walk the camera a distance d.
	void Strafe(float d);
	void Walk(float d);

	// Rotate the camera.
	void Pitch(float angle);
	void RotateY(float angle);
};