#include "Cube.h"
#include "MaterialManager.h"

using namespace DirectX;

void CCube::Init()
{
	m_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterialNormal((int)MaterialNormalIndex::Material_Tile_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
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

void CCube::UpdateImGui(const GameTimer & GlobalTimer)
{
	static bool showClose = true;
	static int index = (int)MaterialNormalIndex::Material_Tile_00;

	if (showClose)
	{
		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"ObjMaterialList", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);
		if (ImGui::Combo(m_Name.c_str(), &index, CMaterialManager::GetMaterialNormalNames(), CMaterialManager::GetMaterialNormalCount()))
		{
			m_Material = CMaterialManager::GetMaterialNormal(index);
			m_NumFramesDirty = gNumFrameResources;
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}