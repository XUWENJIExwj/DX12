#include "Cube.h"

using namespace DirectX;

void CCube::Init()
{
	m_Scale = XMFLOAT3(2.0f, 1.0f, 2.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterial((int)MaterialIndex::Material_00_Bricks);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_00_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["cube"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["cube"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["cube"].BaseVertexLocation;
}

void CCube::Update(const GameTimer& GlobalTimer)
{
	m_Rotation.y += 1.0f * GlobalTimer.DeltaTime();
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(GetRotation());
	m_World = ComputeWorldMatrix4x4();

	m_NumFramesDirty = gNumFrameResources;
}