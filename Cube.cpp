#include "Cube.h"
#include "MaterialManager.h"

using namespace DirectX;

void CCube::Init()
{
	m_Position = XMFLOAT3(0.0f, 0.0f, 100.0f);
	m_Scale = XMFLOAT3(50.0f, 200.0f, 50.0f);
	m_World = ComputeWorldMatrix4x4();
	m_Material = CMaterialManager::GetMaterialNormal((int)MaterialNormalIndex::Material_Tile_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["cube"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["cube"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["cube"].BaseVertexLocation;

	m_BoundsName = "cube";
	ComputeBoundingBox();

	m_ImGuiCB.ShowClose = true;
	m_ImGuiCB.MaterialIndex = (int)MaterialNormalIndex::Material_Tile_00;
}

void CCube::Update(const GameTimer& GlobalTimer)
{
	//m_Rotation.y += 1.0f * GlobalTimer.DeltaTime();
	//XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(GetRotation());
	//m_World = ComputeWorldMatrix4x4();

	//m_NumFramesDirty = gNumFrameResources;
}

void CCube::LateUpdate(const GameTimer& GlobalTimer)
{
	UpdateBoundingBox();
}

void CCube::UpdateImGui(const GameTimer& GlobalTimer)
{
	if (m_ImGuiCB.ShowClose)
	{
		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"ObjMaterialList", &m_ImGuiCB.ShowClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);
		if (ImGui::Combo(m_Name.c_str(), &m_ImGuiCB.MaterialIndex, CMaterialManager::GetMaterialNormalNames(), CMaterialManager::GetMaterialNormalCount()))
		{
			m_Material = CMaterialManager::GetMaterialNormal(m_ImGuiCB.MaterialIndex);
			m_NumFramesDirty = gNumFrameResources;
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}