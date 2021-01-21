#include "QuadShadowDebug.h"

using namespace DirectX;

void CQuadShadowDebug::Init()
{
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(250.0f, 250.0f, 1.0f);
	m_Position = XMFLOAT3(m_Scale.x / 2, m_Scale.y / 2, m_Position.z);
	m_World = Compute2DWVPMatrix4x4();

	Init2DScaleRatio();

	m_Material = CMaterialManager::GetMaterialNormal((int)MaterialNormalIndex::Material_ShadowMap_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["quad"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["quad"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["quad"].BaseVertexLocation;
}

void CQuadShadowDebug::Update(const GameTimer& GlobalTimer)
{
	//AnimateMaterial(GlobalTimer);
}

void CQuadShadowDebug::OnResize()
{
	CGameObject::OnResize();
	m_Position = XMFLOAT3(m_Scale.x / 2 + m_Scale.x * m_OrderColNum, m_Scale.y / 2, m_Position.z);
	m_World = Compute2DWVPMatrix4x4();
	m_NumFramesDirty = gNumFrameResources;
}