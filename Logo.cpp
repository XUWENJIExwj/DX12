#include "Logo.h"

using namespace DirectX;

void CLogo::Init()
{
	m_Position = XMFLOAT3(0.0f, 0.0f, 10.0f);
	m_Rotation = XMFLOAT3(-0.2f * MathHelper::Pi, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(0.64f, 1.0f, 0.36f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterialNormal((int)MaterialNormalIndex::Material_Logo_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["grid"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["grid"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["grid"].BaseVertexLocation;
}