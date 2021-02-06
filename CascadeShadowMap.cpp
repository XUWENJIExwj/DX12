#include "CascadeShadowMap.h"
#include "Camera.h"
#include "CameraFP.h"
#include "Light.h"
#include "DirectionalLight.h"

using namespace std;
using namespace DirectX;

CCascadeShadowMap::CCascadeShadowMap(ID3D12Device* Device, UINT CascadeNum) :m_D3DDevice(Device), m_CascadeNum(CascadeNum)
{
	m_ShadowMapSize = 1024 * 2;
	m_ShadowMap.resize(m_CascadeNum);
	m_ShadowMapDescHandle.resize(m_CascadeNum);

	for (UINT i = 0; i < m_CascadeNum; ++i)
	{
		UINT size = m_ShadowMapSize / (UINT)pow(2, i);
		size = m_ShadowMapSize;
		m_ShadowMap[i] = make_unique<CShadowMap>(m_D3DDevice, size, size);
	}
}

void CCascadeShadowMap::UpdateCSMImGui()
{
	static bool showClose = true;

	if (showClose)
	{
		ImGui::SetNextWindowPos(ImVec2((float)DX12App::GetApp()->GetWindowWidth() - 420, 500), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 210), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"CSM", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		ImGui::Checkbox(u8"VisualCascade", &m_CSMPassCB.VisualCascade);
		ImGui::SameLine();
		ImGui::Checkbox(u8"CancelJitter", &m_CSMPassCB.CancelJitter);
		ImGui::Checkbox(u8"NearFarCorrection", &m_CSMPassCB.NearFarCorrection);
		//ImGui::SameLine();
		//ImGui::Checkbox(u8"BlendCascade", &m_CSMPassCB.BlendCascade);
		if (ImGui::DragInt(u8"PCFBlurSize", &m_CSMPassCB.PCFBlurSize, 0.1f, 1, 10))
		{
			m_CSMPassCB.PCFBlurForLoopStart = m_CSMPassCB.PCFBlurSize / -2;
			m_CSMPassCB.PCFBlurForLoopEnd = m_CSMPassCB.PCFBlurSize / 2 + 1;
		}
		//ImGui::DragFloat(u8"CascadeBlendArea", &m_CSMPassCB.CascadeBlendArea, 0.00001f, -0.1f, 1.0f, "%.4f");
		ImGui::DragFloat(u8"ShadowBiasForCascade0", &m_CSMPassCB.ShadowBias.x, 0.00001f, -0.1f, 0.01f, "%.4f");
		ImGui::DragFloat(u8"ShadowBiasForCascade1", &m_CSMPassCB.ShadowBias.y, 0.00001f, -0.1f, 0.01f, "%.4f");
		ImGui::DragFloat(u8"ShadowBiasForCascade2", &m_CSMPassCB.ShadowBias.z, 0.00001f, -0.1f, 0.01f, "%.4f");
		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void CCascadeShadowMap::CreateDescriptors(
	D3D12_CPU_DESCRIPTOR_HANDLE CpuSrvStartHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE GpuSrvStartHandle,
	D3D12_CPU_DESCRIPTOR_HANDLE CpuDsvStartHandle,
	UINT Offset, UINT CbvSrvUavDescSize, UINT DsvDescSize)
{
	for (UINT i = 0; i < m_CascadeNum; ++i)
	{
		m_ShadowMapDescHandle[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(GpuSrvStartHandle, Offset + i, CbvSrvUavDescSize);
		m_ShadowMap[i]->CreateDescriptors(
			CD3DX12_CPU_DESCRIPTOR_HANDLE(CpuSrvStartHandle, Offset + i, CbvSrvUavDescSize),
			m_ShadowMapDescHandle[i],
			CD3DX12_CPU_DESCRIPTOR_HANDLE(CpuDsvStartHandle, 1 + i, DsvDescSize));
	}
}

void CCascadeShadowMap::CreateSceneAABBPoints(XMVECTOR* SceneAABBPoints, const BoundingBox* SceneBoundingBox)
{
	const XMVECTORF32 extentsMap[] =
	{
		{  1.0f,  1.0f, -1.0f, 1.0f},
		{ -1.0f,  1.0f, -1.0f, 1.0f},
		{  1.0f, -1.0f, -1.0f, 1.0f},
		{ -1.0f, -1.0f, -1.0f, 1.0f},
		{  1.0f,  1.0f,  1.0f, 1.0f},
		{ -1.0f,  1.0f,  1.0f, 1.0f},
		{  1.0f, -1.0f,  1.0f, 1.0f},
		{ -1.0f, -1.0f,  1.0f, 1.0f},
	};

	XMVECTOR extents = XMLoadFloat3(&SceneBoundingBox->Extents); // w = 0（要注意）
	XMVECTOR center = XMLoadFloat3(&SceneBoundingBox->Center); // w = 0（要注意）

	for (int i = 0; i < 8; ++i)
	{
		SceneAABBPoints[i] = XMVectorMultiplyAdd(extentsMap[i], extents, center);
	}
}

void XM_CALLCONV CCascadeShadowMap::ComputeFitCascadeCSMPassCB(
	const FXMMATRIX& CameraInvView, const BoundingBox* SceneBoundingBox,
	CCamera* Camera, CLight* Light)
{
	XMMATRIX lightView = Light->GetView();

	XMVECTOR sceneAABBPointsLiS[8];
	CreateSceneAABBPoints(sceneAABBPointsLiS, SceneBoundingBox);
	for (int i = 0; i < 8; ++i)
	{
		sceneAABBPointsLiS[i] = XMVector3TransformCoord(sceneAABBPointsLiS[i], lightView);
	}

	float frustumIntervalBegin, frustumIntervalEnd;
	XMVECTOR lightOrthographicMin;
	XMVECTOR lightOrthographicMax;
	float cameraNearFarRange = Camera->GetFarZ() - Camera->GetNearZ();
	XMVECTOR worldUnitsPerTexel = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	static float cascadePartitions[] = { 0.03f, 0.20f, 0.60f }; // 要素数 = CASCADE_NUM

	for (UINT i = 0; i < m_CascadeNum; ++i)
	{
		if (i == 0) frustumIntervalBegin = 0.0f;
		else frustumIntervalBegin = cascadePartitions[i - 1];
		frustumIntervalEnd = cascadePartitions[i];
		frustumIntervalBegin *= cameraNearFarRange;
		frustumIntervalEnd *= cameraNearFarRange;

		XMVECTOR frustumPoints[8];
		Camera->ComputeFrustumPointsFromCascadeInterval(frustumPoints, frustumIntervalBegin, frustumIntervalEnd);
		lightOrthographicMin = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
		lightOrthographicMax = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
		XMVECTOR tempfrustumPoint;
		for (int j = 0; j < 8; ++j)
		{
			frustumPoints[j] = XMVector3TransformCoord(frustumPoints[j], CameraInvView); // FrustumPointsをWorld空間へ転換
			tempfrustumPoint = XMVector3TransformCoord(frustumPoints[j], lightView);  // FrustumPointをLight空間へ転換
			lightOrthographicMin = XMVectorMin(tempfrustumPoint, lightOrthographicMin);
			lightOrthographicMax = XMVectorMax(tempfrustumPoint, lightOrthographicMax);
		}

		float lightOrthographicMinZ, lightOrthographicMaxZ;
		lightOrthographicMinZ = XMVectorGetZ(lightOrthographicMin);
		lightOrthographicMaxZ = XMVectorGetZ(lightOrthographicMax);

		// ジッター消去
		if (m_CSMPassCB.CancelJitter)
		{
			float scaleDueToBlureAMT1f = (float)(m_CSMPassCB.PCFBlurSize * 2 + 1) / (float)m_ShadowMapSize;
			XMVECTORF32 scaleDueToBlureAMT = { scaleDueToBlureAMT1f, scaleDueToBlureAMT1f, 0.0f, 0.0f };
			float normalizeByShadowMapSize1f = 1.0f / (float)m_ShadowMapSize;
			XMVECTOR normalizeByShadowMapSize = XMVectorSet(normalizeByShadowMapSize1f, normalizeByShadowMapSize1f, 0.0f, 0.0f);
			XMVECTOR boarderOffset = lightOrthographicMax - lightOrthographicMin;
			boarderOffset *= XMVectorSet(0.5f, 0.5f, 0.5f, 0.5f);
			boarderOffset *= scaleDueToBlureAMT;
			lightOrthographicMax += boarderOffset;
			lightOrthographicMin -= boarderOffset;
			worldUnitsPerTexel = lightOrthographicMax - lightOrthographicMin;
			worldUnitsPerTexel *= normalizeByShadowMapSize;

			lightOrthographicMin /= worldUnitsPerTexel;
			lightOrthographicMin = XMVectorFloor(lightOrthographicMin);
			lightOrthographicMin *= worldUnitsPerTexel;
			lightOrthographicMax /= worldUnitsPerTexel;
			lightOrthographicMax = XMVectorFloor(lightOrthographicMax);
			lightOrthographicMax *= worldUnitsPerTexel;
		}

		// NearFarCorrection
		float nearPlane = 0.0f;
		float farPlane = 10000.0f;
		if (m_CSMPassCB.NearFarCorrection)
		{
			ComputeNearAndFarInCSM(nearPlane, farPlane, lightOrthographicMin, lightOrthographicMax, sceneAABBPointsLiS);
		}
		else
		{
			nearPlane = lightOrthographicMinZ;
			farPlane = lightOrthographicMaxZ;
		}

		XMMATRIX shadowTransform = Light->ComputeShadowTransformFromLightOrthographicAndNearFar(i, nearPlane, farPlane, lightOrthographicMin, lightOrthographicMax);
		XMFLOAT4X4 shadowTransform4x4;
		XMStoreFloat4x4(&shadowTransform4x4, shadowTransform);
		m_CSMPassCB.ShadowTexScale[i].x = shadowTransform4x4(0, 0);
		m_CSMPassCB.ShadowTexScale[i].y = shadowTransform4x4(1, 1);
		m_CSMPassCB.ShadowTexScale[i].z = shadowTransform4x4(2, 2);
		m_CSMPassCB.ShadowTexScale[i].w = 1.0f;

		m_CSMPassCB.ShadowTexOffset[i].x = shadowTransform4x4(3, 0);
		m_CSMPassCB.ShadowTexOffset[i].y = shadowTransform4x4(3, 1);
		m_CSMPassCB.ShadowTexOffset[i].z = shadowTransform4x4(3, 2);
		m_CSMPassCB.ShadowTexOffset[i].w = 0.0f;

		m_CSMPassCB.CascadeFrustumsEyeSpaceDepthsFloat4[i].x = frustumIntervalEnd;
	}
	m_CSMPassCB.CascadeFrustumsEyeSpaceDepthsFloat.x = m_CSMPassCB.CascadeFrustumsEyeSpaceDepthsFloat4[0].x;
	m_CSMPassCB.CascadeFrustumsEyeSpaceDepthsFloat.y = m_CSMPassCB.CascadeFrustumsEyeSpaceDepthsFloat4[1].x;
	m_CSMPassCB.CascadeFrustumsEyeSpaceDepthsFloat.z = m_CSMPassCB.CascadeFrustumsEyeSpaceDepthsFloat4[2].x;
}

struct Triangle
{
	XMVECTOR Point[3];
	BOOL Culled;
};

void XM_CALLCONV CCascadeShadowMap::ComputeNearAndFarInCSM(float& Near, float& Far, FXMVECTOR LightOrthographicMin, FXMVECTOR LightOrthographicMax, XMVECTOR* SceneAABBPointsLiS)
{
	// Initialize the near and far planes
	Near = FLT_MAX;
	Far = -FLT_MAX;

	Triangle triangleList[16];
	int triangleCnt = 1;

	triangleList[0].Point[0] = SceneAABBPointsLiS[0];
	triangleList[0].Point[1] = SceneAABBPointsLiS[1];
	triangleList[0].Point[2] = SceneAABBPointsLiS[2];
	triangleList[0].Culled = false;

	// These are the indices used to tesselate an AABB into a list of triangles.
	static const int aabbTriIndexes[] =
	{
		0,1,2,  1,2,3,
		4,5,6,  5,6,7,
		0,2,4,  2,4,6,
		1,3,5,  3,5,7,
		0,1,4,  1,4,5,
		2,3,6,  3,6,7
	};

	int pointPassesCollision[3];

	// At a high level: 
	// 1. Iterate over all 12 triangles of the AABB.  
	// 2. Clip the triangles against each plane. Create new triangles as needed.
	// 3. Find the min and max z values as the near and far plane.

	//This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.

	float lightOrthographicMinX = XMVectorGetX(LightOrthographicMin);
	float lightOrthographicMaxX = XMVectorGetX(LightOrthographicMax);
	float lightOrthographicMinY = XMVectorGetY(LightOrthographicMin);
	float lightOrthographicMaxY = XMVectorGetY(LightOrthographicMax);

	for (int aabbTriIter = 0; aabbTriIter < 12; ++aabbTriIter)
	{

		triangleList[0].Point[0] = SceneAABBPointsLiS[aabbTriIndexes[aabbTriIter * 3 + 0]];
		triangleList[0].Point[1] = SceneAABBPointsLiS[aabbTriIndexes[aabbTriIter * 3 + 1]];
		triangleList[0].Point[2] = SceneAABBPointsLiS[aabbTriIndexes[aabbTriIter * 3 + 2]];
		triangleCnt = 1;
		triangleList[0].Culled = FALSE;

		// Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
		//add them to the list.
		for (int frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter)
		{

			float edge;
			int component;

			if (frustumPlaneIter == 0)
			{
				edge = lightOrthographicMinX; // todo make float temp
				component = 0;
			}
			else if (frustumPlaneIter == 1)
			{
				edge = lightOrthographicMaxX;
				component = 0;
			}
			else if (frustumPlaneIter == 2)
			{
				edge = lightOrthographicMinY;
				component = 1;
			}
			else
			{
				edge = lightOrthographicMaxY;
				component = 1;
			}

			for (int triIter = 0; triIter < triangleCnt; ++triIter)
			{
				// We don't delete triangles, so we skip those that have been Culled.
				if (!triangleList[triIter].Culled)
				{
					int insideVertCount = 0;
					XMVECTOR tempOrder;
					// Test against the correct frustum plane.
					// This could be written more compactly, but it would be harder to understand.

					if (frustumPlaneIter == 0)
					{
						for (int triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetX(triangleList[triIter].Point[triPtIter]) >
								XMVectorGetX(LightOrthographicMin))
							{
								pointPassesCollision[triPtIter] = 1;
							}
							else
							{
								pointPassesCollision[triPtIter] = 0;
							}
							insideVertCount += pointPassesCollision[triPtIter];
						}
					}
					else if (frustumPlaneIter == 1)
					{
						for (int triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetX(triangleList[triIter].Point[triPtIter]) <
								XMVectorGetX(LightOrthographicMax))
							{
								pointPassesCollision[triPtIter] = 1;
							}
							else
							{
								pointPassesCollision[triPtIter] = 0;
							}
							insideVertCount += pointPassesCollision[triPtIter];
						}
					}
					else if (frustumPlaneIter == 2)
					{
						for (int triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetY(triangleList[triIter].Point[triPtIter]) >
								XMVectorGetY(LightOrthographicMin))
							{
								pointPassesCollision[triPtIter] = 1;
							}
							else
							{
								pointPassesCollision[triPtIter] = 0;
							}
							insideVertCount += pointPassesCollision[triPtIter];
						}
					}
					else
					{
						for (int triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetY(triangleList[triIter].Point[triPtIter]) <
								XMVectorGetY(LightOrthographicMax))
							{
								pointPassesCollision[triPtIter] = 1;
							}
							else
							{
								pointPassesCollision[triPtIter] = 0;
							}
							insideVertCount += pointPassesCollision[triPtIter];
						}
					}

					// Move the points that pass the frustum test to the begining of the array.
					if (pointPassesCollision[1] && !pointPassesCollision[0])
					{
						tempOrder = triangleList[triIter].Point[0];
						triangleList[triIter].Point[0] = triangleList[triIter].Point[1];
						triangleList[triIter].Point[1] = tempOrder;
						pointPassesCollision[0] = TRUE;
						pointPassesCollision[1] = FALSE;
					}
					if (pointPassesCollision[2] && !pointPassesCollision[1])
					{
						tempOrder = triangleList[triIter].Point[1];
						triangleList[triIter].Point[1] = triangleList[triIter].Point[2];
						triangleList[triIter].Point[2] = tempOrder;
						pointPassesCollision[1] = TRUE;
						pointPassesCollision[2] = FALSE;
					}
					if (pointPassesCollision[1] && !pointPassesCollision[0])
					{
						tempOrder = triangleList[triIter].Point[0];
						triangleList[triIter].Point[0] = triangleList[triIter].Point[1];
						triangleList[triIter].Point[1] = tempOrder;
						pointPassesCollision[0] = TRUE;
						pointPassesCollision[1] = FALSE;
					}

					if (insideVertCount == 0)
					{ // All points failed. We're done,  
						triangleList[triIter].Culled = true;
					}
					else if (insideVertCount == 1)
					{// One point passed. Clip the triangle against the Frustum plane
						triangleList[triIter].Culled = false;

						// 
						XMVECTOR vert0ToVert1 = triangleList[triIter].Point[1] - triangleList[triIter].Point[0];
						XMVECTOR vert0ToVert2 = triangleList[triIter].Point[2] - triangleList[triIter].Point[0];

						// Find the collision ratio.
						FLOAT hitPointTimeRatio = edge - XMVectorGetByIndex(triangleList[triIter].Point[0], component);
						// Calculate the distance along the vector as ratio of the hit ratio to the component.
						FLOAT distanceAlongVector01 = hitPointTimeRatio / XMVectorGetByIndex(vert0ToVert1, component);
						FLOAT distanceAlongVector02 = hitPointTimeRatio / XMVectorGetByIndex(vert0ToVert2, component);
						// Add the point plus a percentage of the vector.
						vert0ToVert1 *= distanceAlongVector01;
						vert0ToVert1 += triangleList[triIter].Point[0];
						vert0ToVert2 *= distanceAlongVector02;
						vert0ToVert2 += triangleList[triIter].Point[0];

						triangleList[triIter].Point[1] = vert0ToVert2;
						triangleList[triIter].Point[2] = vert0ToVert1;
					}
					else if (insideVertCount == 2)
					{ // 2 in  // tesselate into 2 triangles


						// Copy the triangle\(if it exists) after the current triangle out of
						// the way so we can override it with the new triangle we're inserting.
						triangleList[triangleCnt] = triangleList[triIter + 1];

						triangleList[triIter].Culled = false;
						triangleList[triIter + 1].Culled = false;

						// Get the vector from the outside point into the 2 inside points.
						XMVECTOR vert2ToVert0 = triangleList[triIter].Point[0] - triangleList[triIter].Point[2];
						XMVECTOR vert2ToVert1 = triangleList[triIter].Point[1] - triangleList[triIter].Point[2];

						// Get the hit point ratio.
						FLOAT hitPointTime_2_0 = edge - XMVectorGetByIndex(triangleList[triIter].Point[2], component);
						FLOAT distanceAlongVector_2_0 = hitPointTime_2_0 / XMVectorGetByIndex(vert2ToVert0, component);
						// Calcaulte the new vert by adding the percentage of the vector plus point 2.
						vert2ToVert0 *= distanceAlongVector_2_0;
						vert2ToVert0 += triangleList[triIter].Point[2];

						// Add a new triangle.
						triangleList[triIter + 1].Point[0] = triangleList[triIter].Point[0];
						triangleList[triIter + 1].Point[1] = triangleList[triIter].Point[1];
						triangleList[triIter + 1].Point[2] = vert2ToVert0;

						//Get the hit point ratio.
						FLOAT hitPointTime_2_1 = edge - XMVectorGetByIndex(triangleList[triIter].Point[2], component);
						FLOAT distanceAlongVector_2_1 = hitPointTime_2_1 / XMVectorGetByIndex(vert2ToVert1, component);
						vert2ToVert1 *= distanceAlongVector_2_1;
						vert2ToVert1 += triangleList[triIter].Point[2];
						triangleList[triIter].Point[0] = triangleList[triIter + 1].Point[1];
						triangleList[triIter].Point[1] = triangleList[triIter + 1].Point[2];
						triangleList[triIter].Point[2] = vert2ToVert1;
						// Cncrement triangle count and skip the triangle we just inserted.
						++triangleCnt;
						++triIter;
					}
					else
					{ // all in
						triangleList[triIter].Culled = false;
					}
				}// end if !Culled loop            
			}
		}
		for (int index = 0; index < triangleCnt; ++index)
		{
			if (!triangleList[index].Culled)
			{
				// Set the near and far plan and the min and max z values respectivly.
				for (int vertind = 0; vertind < 3; ++vertind)
				{
					float triangleCoordZ = XMVectorGetZ(triangleList[index].Point[vertind]);
					if (Near > triangleCoordZ)
					{
						Near = triangleCoordZ;
					}
					if (Far < triangleCoordZ)
					{
						Far = triangleCoordZ;
					}
				}
			}
		}
	}
}