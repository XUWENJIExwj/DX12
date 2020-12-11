#include "CameraFP.h"
#include "Input.h"

#define CAMERA_MOVE 2.0f * GlobalTimer.DeltaTime()
#define CAMERA_ROT 0.5f * GlobalTimer.DeltaTime()

using namespace DirectX;

void CCameraFP::Init()
{
	m_Position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	m_Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	//Pitch(0.2f * MathHelper::Pi);

	SetProjectionMatrix(0.25f * MathHelper::Pi, DX12App::GetApp()->GetAspectRatio(), 1.0f, 1000.0f);
}

void CCameraFP::Update(const GameTimer& GlobalTimer)
{
	CameraWalk(GlobalTimer);
	CameraRotate(GlobalTimer);
}

void CCameraFP::WalkDepth(float d)
{
	// mPosition += d * m_Look
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&XMFLOAT3(m_Look.x, 0.0f, m_Look.z));;
	XMVector3Normalize(l);
	XMVECTOR p = XMLoadFloat3(&m_Position);
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, l, p));

	m_ViewDirty = true;
}

void CCameraFP::WalkHorizontal(float d)
{
	// mPosition += d * m_Right
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&m_Right);
	XMVECTOR p = XMLoadFloat3(&m_Position);
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, r, p));

	m_ViewDirty = true;
}

void CCameraFP::WalkVertical(float d)
{
	// mPosition += d * WorldUp
	XMVECTOR s = XMVectorReplicate(d);
	XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR u = XMLoadFloat3(&worldUp);
	XMVECTOR p = XMLoadFloat3(&m_Position);
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, u, p));

	m_ViewDirty = true;
}

void CCameraFP::CameraWalk(const GameTimer& GlobalTimer)
{
	if (CInput::GetKeyPress('W'))
	{
		WalkDepth(CAMERA_MOVE);
	}
	if (CInput::GetKeyPress('S'))
	{
		WalkDepth(-CAMERA_MOVE);
	}
	if (CInput::GetKeyPress('A'))
	{
		WalkHorizontal(-CAMERA_MOVE);
	}
	if (CInput::GetKeyPress('D'))
	{
		WalkHorizontal(CAMERA_MOVE);
	}
	if (CInput::GetKeyPress('R'))
	{
		WalkVertical(CAMERA_MOVE);
	}
	if (CInput::GetKeyPress('F'))
	{
		WalkVertical(-CAMERA_MOVE);
	}
}

void CCameraFP::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), angle);

	XMStoreFloat3(&m_Up,   XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
	XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

	m_ViewDirty = true;
}

void CCameraFP::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&m_Right,   XMVector3TransformNormal(XMLoadFloat3(&m_Right), R));
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
	XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

	m_ViewDirty = true;
}

void CCameraFP::CameraRotate(const GameTimer& GlobalTimer)
{
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