#include "Box.h"

using namespace DirectX;

void CBox::Init()
{ 
	m_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&m_World, XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z));
	m_Material = CMaterialManager::GetMaterial((int)MaterialIndex::Material_00_Bricks);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_00_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["sphere"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["sphere"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["sphere"].BaseVertexLocation;
}