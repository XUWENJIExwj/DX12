#include "Quad.h"

using namespace DirectX;

void CQuad::Init()
{
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(300.0f, 300.0f, 1.0f);
	m_Position = XMFLOAT3(m_Scale.x / 2, m_Scale.y / 2, m_Position.z);
	m_World = Compute2DWVPMatrix4x4();

	Init2DScaleRatio();

	m_Material = CMaterialManager::GetMaterialNormal((int)MaterialNormalIndex::Material_ShadowMap);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["quad"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["quad"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["quad"].BaseVertexLocation;
}

void CQuad::Update(const GameTimer& GlobalTimer)
{
	//AnimateMaterial(GlobalTimer);
}

void CQuad::OnResize()
{
	CGameObject::OnResize();
	m_Position = XMFLOAT3(m_Scale.x / 2, m_Scale.y / 2, m_Position.z);
	m_World = Compute2DWVPMatrix4x4();
	m_NumFramesDirty = gNumFrameResources;
}

void CQuad::AnimateMaterial(const GameTimer& GlobalTimer)
{
	float& tu = m_Material->MatTransform(3, 0);
	float& tv = m_Material->MatTransform(3, 1);

	tu += 0.1f * GlobalTimer.DeltaTime();
	tv += 0.0f * GlobalTimer.DeltaTime();
	if (tu >= 1.0f) tu -= 1.0f;
	if (tv >= 1.0f) tv -= 1.0f;

	m_Material->MatTransform(3, 0) = tu;
	m_Material->MatTransform(3, 1) = tv;
	m_Material->NumFramesDirty = gNumFrameResources;
}