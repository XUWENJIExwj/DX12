#include "Quad.h"

using namespace DirectX;

void CQuad::Init()
{
	m_Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterialNormal((int)MaterialNormalIndex::Material_ShadowMap);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["quad"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["quad"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["quad"].BaseVertexLocation;
}