#include "Sky.h"

using namespace DirectX;

void CSky::Init()
{ 
	XMStoreFloat3(&m_Scale, GetScale() * 1000.0f);
	XMStoreFloat4x4(&m_World, XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z));
	m_Material = CMaterialManager::GetMaterial((int)MaterialIndex::Material_SkyCube_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_00_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["sphere"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["sphere"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["sphere"].BaseVertexLocation;
}