#include "Camera.h"
#include "Input.h"

#define CAMERA_MOVE 2.0f * GlobalTimer.DeltaTime()
#define CAMERA_ROT 0.5f * GlobalTimer.DeltaTime()

using namespace DirectX;

void CCamera::Init()
{
	m_Position = XMFLOAT3(0.0f, 0.0f, -5.0f);
	m_Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	SetLens(0.25f * MathHelper::Pi, DX12App::GetApp()->GetAspectRatio(), 1.0f, 1000.0f);
}

void CCamera::Update(const GameTimer & GlobalTimer)
{
	if (CInput::GetKeyPress('W'))
	{
		Walk(CAMERA_MOVE);
	}
	if (CInput::GetKeyPress('S'))
	{
		Walk(-CAMERA_MOVE);
	}
	if (CInput::GetKeyPress('A'))
	{
		Strafe(-CAMERA_MOVE);
	}
	if (CInput::GetKeyPress('D'))
	{
		Strafe(CAMERA_MOVE);
	}

	if (CInput::GetKeyPress(VK_UP))
	{
		Pitch(-CAMERA_ROT);
	}
	if (CInput::GetKeyPress(VK_DOWN))
	{
		Pitch(CAMERA_ROT);
	}
	if (CInput::GetKeyPress(VK_LEFT))
	{
		RotateY(-CAMERA_ROT);
	}
	if (CInput::GetKeyPress(VK_RIGHT))
	{
		RotateY(CAMERA_ROT);
	}
}

void CCamera::LateUpdate(const GameTimer & GlobalTimer)
{
	if (mViewDirty)
	{
		XMVECTOR R = XMLoadFloat3(&m_Right);
		XMVECTOR U = XMLoadFloat3(&m_Up);
		XMVECTOR L = XMLoadFloat3(&m_Look);
		XMVECTOR P = XMLoadFloat3(&m_Position);

		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L);

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&m_Right, R);
		XMStoreFloat3(&m_Up, U);
		XMStoreFloat3(&m_Look, L);

		mView(0, 0) = m_Right.x;
		mView(1, 0) = m_Right.y;
		mView(2, 0) = m_Right.z;
		mView(3, 0) = x;

		mView(0, 1) = m_Up.x;
		mView(1, 1) = m_Up.y;
		mView(2, 1) = m_Up.z;
		mView(3, 1) = y;

		mView(0, 2) = m_Look.x;
		mView(1, 2) = m_Look.y;
		mView(2, 2) = m_Look.z;
		mView(3, 2) = z;

		mView(0, 3) = 0.0f;
		mView(1, 3) = 0.0f;
		mView(2, 3) = 0.0f;
		mView(3, 3) = 1.0f;

		mViewDirty = false;
	}
}

// Get frustum properties.
float CCamera::GetFovX()const
{
	float halfWidth = 0.5f * GetNearWindowWidth();
	return 2.0f * atan(halfWidth / m_NearZ);
}

void CCamera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	m_FovY = fovY;
	m_Aspect = aspect;
	m_NearZ = zn;
	m_FarZ = zf;

	m_NearWindowHeight = 2.0f * m_NearZ * tanf( 0.5f * m_FovY );
	m_FarWindowHeight  = 2.0f * m_FarZ * tanf( 0.5f * m_FovY );

	XMMATRIX P = XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
	XMStoreFloat4x4(&mProj, P);
}

void CCamera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&m_Position, pos);
	XMStoreFloat3(&m_Look, L);
	XMStoreFloat3(&m_Right, R);
	XMStoreFloat3(&m_Up, U);

	mViewDirty = true;
}

void CCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);

	mViewDirty = true;
}

XMMATRIX CCamera::GetView()const
{
	assert(!mViewDirty);
	return XMLoadFloat4x4(&mView);
}

XMFLOAT4X4 CCamera::GetView4x4f()const
{
	assert(!mViewDirty);
	return mView;
}

void CCamera::Strafe(float d)
{
	// mPosition += d * mRight
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&m_Right);
	XMVECTOR p = XMLoadFloat3(&m_Position);
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, r, p));

	mViewDirty = true;
}

void CCamera::Walk(float d)
{
	// mPosition += d * mLook
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&m_Look);
	XMVECTOR p = XMLoadFloat3(&m_Position);
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, l, p));

	mViewDirty = true;
}

void CCamera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), angle);

	XMStoreFloat3(&m_Up,   XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
	XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

	mViewDirty = true;
}

void CCamera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&m_Right,   XMVector3TransformNormal(XMLoadFloat3(&m_Right), R));
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
	XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

	mViewDirty = true;
}