#include "Cube.h"
#include "MaterialManager.h"

using namespace DirectX;

void CCube::Init()
{
	m_Scale = XMFLOAT3(2.0f, 1.0f, 2.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterial((int)MaterialIndex::Material_Logo_00);
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
	static int index = (int)MaterialIndex::Material_Bricks_00;

	if (showClose)
	{
		ImGui::SetNextWindowPos(ImVec2(400, 20), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(m_Name.c_str(), &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		if (ImGui::Combo("MaterialList", &index, CMaterialManager::GetMaterialsName(), CMaterialManager::GetMaterialsCount()))
		{
			m_Material = CMaterialManager::GetMaterial(index);
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}