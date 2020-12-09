#include "Sphere.h"

using namespace DirectX;

void CSphere::Init()
{
	m_Position = XMFLOAT3(0.0f, 2.0f, 0.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterial((int)MaterialIndex::Material_02_Mirror);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_00_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["sphere"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["sphere"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["sphere"].BaseVertexLocation;
}