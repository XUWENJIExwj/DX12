#include "MeshField.h"

using namespace DirectX;

void CMeshField::Init()
{
	m_Position = XMFLOAT3(0.0f, -2.0f, 15.0f);
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(3.84f, 1.0f, 2.16f);
	m_World = ComputeWorldMatrix4x4();
	XMStoreFloat4x4(&m_TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
	m_Material = CMaterialManager::GetMaterial((int)MaterialIndex::Material_Tile_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_00_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["grid"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["grid"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["grid"].BaseVertexLocation;
}