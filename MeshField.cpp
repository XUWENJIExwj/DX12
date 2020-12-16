#include "MeshField.h"

using namespace DirectX;

void CMeshField::Init()
{
	m_Position = XMFLOAT3(0.0f, -3.0f, 0.0f);
	m_Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Scale = XMFLOAT3(5.0f, 1.0f, 5.0f);
	m_World = ComputeWorldMatrix4x4();
	XMStoreFloat4x4(&m_TexTransform, XMMatrixScaling(8.0f, 8.0f, 1.0f));
	m_Material = CMaterialManager::GetMaterialHeight((int)MaterialHeightIndex::Material_Plane_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["grid"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["grid"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["grid"].BaseVertexLocation;
}

void CMeshField::UpdateImGui(const GameTimer & GlobalTimer)
{
	static bool showClose = true;
	static int index = (int)MaterialHeightIndex::Material_Plane_00;

	if (showClose)
	{
		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"ObjMaterialList", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);
		if (ImGui::Combo(m_Name.c_str(), &index, CMaterialManager::GetMaterialHeightNames(), CMaterialManager::GetMaterialHeightCount()))
		{
			m_Material = CMaterialManager::GetMaterialHeight(index);
			m_NumFramesDirty = gNumFrameResources;
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}