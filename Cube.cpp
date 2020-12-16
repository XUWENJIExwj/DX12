#include "Cube.h"
#include "MaterialManager.h"

using namespace DirectX;

void CCube::Init()
{
	m_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterialTex((int)MaterialTexIndex::Material_Tile_00);
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

void CCube::UpdateImGui(const GameTimer & GlobalTimer)
{
	static bool showClose = true;
	static int index = (int)MaterialTexIndex::Material_Tile_00;

	if (showClose)
	{
		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"ObjMaterialList", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.55f);
		if (ImGui::Combo(m_Name.c_str(), &index, CMaterialManager::GetMaterialTexNames(), CMaterialManager::GetMaterialTexCount()))
		{
			m_Material = CMaterialManager::GetMaterialTex(index);
			m_NumFramesDirty = gNumFrameResources;
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}