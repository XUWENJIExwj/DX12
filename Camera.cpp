#include "Camera.h"

using namespace std;
using namespace DirectX;

void CCamera::LateUpdate(const GameTimer& GlobalTimer)
{
	UpdateViewMatrix();
}

void CCamera::UpdateViewMatrix()
{
	if (m_ViewDirty)
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

		m_View(0, 0) = m_Right.x;
		m_View(1, 0) = m_Right.y;
		m_View(2, 0) = m_Right.z;
		m_View(3, 0) = x;

		m_View(0, 1) = m_Up.x;
		m_View(1, 1) = m_Up.y;
		m_View(2, 1) = m_Up.z;
		m_View(3, 1) = y;

		m_View(0, 2) = m_Look.x;
		m_View(1, 2) = m_Look.y;
		m_View(2, 2) = m_Look.z;
		m_View(3, 2) = z;

		m_View(0, 3) = 0.0f;
		m_View(1, 3) = 0.0f;
		m_View(2, 3) = 0.0f;
		m_View(3, 3) = 1.0f;

		m_ViewDirty = false;
	}
}

void CCamera::CreateFrustumBounds()
{
	BoundingFrustum::CreateFromMatrix(m_BoundingFrustum, GetProj());
}

void CCamera::ComputeFrustumPointsFromCascadeInterval(XMVECTOR* FrustumPoints, float CascadeIntervalBegin, float CascadeIntervalEnd)
{
	XMVECTORF32 rightTop = { m_BoundingFrustum.RightSlope, m_BoundingFrustum.TopSlope, 1.0f, 1.0f };
	XMVECTORF32 leftBottom = { m_BoundingFrustum.LeftSlope, m_BoundingFrustum.BottomSlope, 1.0f, 1.0f };
	XMVECTORF32 n = { CascadeIntervalBegin, CascadeIntervalBegin, CascadeIntervalBegin, 1.0f };
	XMVECTORF32 f = { CascadeIntervalEnd, CascadeIntervalEnd, CascadeIntervalEnd, 1.0f };
	XMVECTOR rightTopNear = rightTop * n;
	XMVECTOR rightTopFar = rightTop * f;
	XMVECTOR leftBottomNear = leftBottom * n;
	XMVECTOR leftBottomFar = leftBottom * f;

	static const XMVECTORU32 grabX = { 0xFFFFFFFF,0x00000000,0x00000000,0x00000000 };
	static const XMVECTORU32 grabY = { 0x00000000,0xFFFFFFFF,0x00000000,0x00000000 };

	// Near
	FrustumPoints[0] = rightTopNear; // 右上
	FrustumPoints[1] = XMVectorSelect(rightTopNear, leftBottomNear, grabX); // 左上
	FrustumPoints[2] = leftBottomNear; // 左下
	FrustumPoints[3] = XMVectorSelect(rightTopNear, leftBottomNear, grabY); // 右下

	// Far
	FrustumPoints[4] = rightTopFar; // 右上
	FrustumPoints[5] = XMVectorSelect(rightTopFar, leftBottomFar, grabX); // 左上
	FrustumPoints[6] = leftBottomFar; // 左下
	FrustumPoints[7] = XMVectorSelect(rightTopFar, leftBottomFar, grabY); // 右下
}

void CCamera::ComputeFrustumPointsInWorldSpace(vector<vector<XMVECTOR>>& FrustumPoints)
{
	// Camera空間にある点の座標をWorld空間に転換
	XMMATRIX view = GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);

	ComputeFrustumPointsInWorldSpace(FrustumPoints, invView);
}

void CCamera::ComputeFrustumPointsInWorldSpace(vector<vector<XMVECTOR>>& FrustumPoints, const XMMATRIX& InvView)
{
	vector<float> n = { m_BoundingFrustum.Near, m_BoundingFrustum.Far * 0.01f, m_BoundingFrustum.Far * 0.15f };
	vector<float> f = { n[1], n[2], m_BoundingFrustum.Far };
	//vector<float> n = { m_BoundingFrustum.Near, m_BoundingFrustum.Near, m_BoundingFrustum.Near };
	//vector<float> f = { m_BoundingFrustum.Far * 0.02f, m_BoundingFrustum.Far * 0.15f, m_BoundingFrustum.Far };
	for (int i = 0; i < FrustumPoints.size(); ++i)
	{
		FrustumPoints[i].resize(8);
		ComputeFrustumPointsInWorldSpaceForEachCascade(FrustumPoints[i], InvView, n[i], f[i]);
	}
}

void CCamera::ComputeFrustumPointsInWorldSpaceForEachCascade(vector<XMVECTOR>& FrustumPoints, const XMMATRIX& InvView, float Near, float Far)
{
	XMVECTORF32 rightTop = { m_BoundingFrustum.RightSlope, m_BoundingFrustum.TopSlope, 1.0f, 1.0f };
	XMVECTORF32 leftBottom = { m_BoundingFrustum.LeftSlope, m_BoundingFrustum.BottomSlope, 1.0f, 1.0f };
	XMVECTORF32 n = { Near, Near, Near, 1.0f };
	XMVECTORF32 f = { Far, Far, Far, 1.0f };
	XMVECTOR rightTopNear = rightTop * n;
	XMVECTOR rightTopFar = rightTop * f;
	XMVECTOR leftBottomNear = leftBottom * n;
	XMVECTOR leftBottomFar = leftBottom * f;

	static const XMVECTORU32 grabX = { 0xFFFFFFFF,0x00000000,0x00000000,0x00000000 };
	static const XMVECTORU32 grabY = { 0x00000000,0xFFFFFFFF,0x00000000,0x00000000 };

	// Near
	FrustumPoints[0] = rightTopNear; // 右上
	FrustumPoints[1] = XMVectorSelect(rightTopNear, leftBottomNear, grabX); // 左上
	FrustumPoints[2] = leftBottomNear; // 左下
	FrustumPoints[3] = XMVectorSelect(rightTopNear, leftBottomNear, grabY); // 右下

	// Far
	FrustumPoints[4] = rightTopFar; // 右上
	FrustumPoints[5] = XMVectorSelect(rightTopFar, leftBottomFar, grabX); // 左上
	FrustumPoints[6] = leftBottomFar; // 左下
	FrustumPoints[7] = XMVectorSelect(rightTopFar, leftBottomFar, grabY); // 右下

	// Camera空間にある点の座標をWorld空間に転換
	for (int i = 0; i < 8; ++i)
	{
		FrustumPoints[i] = XMVector3TransformCoord(FrustumPoints[i], InvView);
	}
}

// Get frustum properties.
float CCamera::GetFovX()const
{
	float halfWidth = 0.5f * GetNearWindowWidth();
	return 2.0f * atan(halfWidth / m_NearZ);
}

void CCamera::ComputeProjectionMatrix(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	m_FovY = fovY;
	m_Aspect = aspect;
	m_NearZ = zn;
	m_FarZ = zf;

	m_NearWindowHeight = 2.0f * m_NearZ * tanf( 0.5f * m_FovY );
	m_FarWindowHeight  = 2.0f * m_FarZ * tanf( 0.5f * m_FovY );

	XMMATRIX P = XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
	XMStoreFloat4x4(&m_Proj, P);

	CreateFrustumBounds();
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

	m_ViewDirty = true;
}

void CCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);

	m_ViewDirty = true;
}

XMMATRIX CCamera::GetView()const
{
	assert(!m_ViewDirty);
	return XMLoadFloat4x4(&m_View);
}

XMFLOAT4X4 CCamera::GetView4x4f()const
{
	assert(!m_ViewDirty);
	return m_View;
}