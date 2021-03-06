#include "MeshField.h"

using namespace DirectX;

void CMeshField::Init()
{
	m_Position = XMFLOAT3(0.0f, -0.5f, 0.0f);
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(30.0f, 1.0f, 30.0f);
	m_World = ComputeWorldMatrix4x4();
	XMStoreFloat4x4(&m_TexTransform, XMMatrixScaling(m_Scale.x, m_Scale.z, 1.0f));
	m_Material = CMaterialManager::GetMaterialHeight((int)MaterialHeightIndex::Material_Plane_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["grid"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["grid"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["grid"].BaseVertexLocation;

	m_BoundsName = "grid";
	ComputeBoundingBox();

	m_ImGuiCB.ShowClose = true;
	m_ImGuiCB.MaterialIndex = (int)MaterialHeightIndex::Material_Plane_00;
}

void CMeshField::LateUpdate(const GameTimer& GlobalTimer)
{
	UpdateBoundingBox();
}

void CMeshField::UpdateImGui(const GameTimer& GlobalTimer)
{
	if (m_ImGuiCB.ShowClose)
	{
		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"ObjMaterialList", &m_ImGuiCB.ShowClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);
		if (ImGui::Combo(m_Name.c_str(), &m_ImGuiCB.MaterialIndex, CMaterialManager::GetMaterialHeightNames(), CMaterialManager::GetMaterialHeightCount()))
		{
			m_Material = CMaterialManager::GetMaterialHeight(m_ImGuiCB.MaterialIndex);
			m_NumFramesDirty = gNumFrameResources;
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}