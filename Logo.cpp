#include "Logo.h"

using namespace DirectX;

void CLogo::Init()
{
	m_Position = XMFLOAT3(0.0f, 2.0f, 5.0f);
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(3.2f, 1.8f, 0.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterialNormal((int)MaterialNormalIndex::Material_Logo_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["quad"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["quad"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["quad"].BaseVertexLocation;
}