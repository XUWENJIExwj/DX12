#include "Renderer.h"
#include "Sky.h"

using namespace DirectX;

void CSky::Init()
{ 
	XMStoreFloat3(&m_Scale, GetScale() * 1000.0f);
	XMStoreFloat4x4(&m_World, XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z));
	m_Material = CMaterialManager::GetMaterialCubeMap((int)MaterialCubeMapIndex::Material_SkyCube_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["sphere"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["sphere"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["sphere"].BaseVertexLocation;

	m_ImGuiCB.ShowClose = true;
	m_ImGuiCB.MaterialIndex = (int)MaterialCubeMapIndex::Material_SkyCube_00;
}

void CSky::UpdateImGui(const GameTimer & GlobalTimer)
{
	if (m_ImGuiCB.ShowClose)
	{
		ImGui::SetNextWindowPos(ImVec2(20, 190), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 130), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"ObjMaterialList", &m_ImGuiCB.ShowClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);

		if (ImGui::Combo(m_Name.c_str(), &m_ImGuiCB.MaterialIndex, CMaterialManager::GetMaterialCubeMapNames(), CMaterialManager::GetMaterialCubeMapCount()))
		{
			m_Material = CMaterialManager::GetMaterialCubeMap(m_ImGuiCB.MaterialIndex);
			m_NumFramesDirty = gNumFrameResources;
			CRenderer::SetCurrentSkyCubeMapIndex(m_ImGuiCB.MaterialIndex);
			for (int i = 1; i < CMaterialManager::GetAllMaterialsCount(); ++i)
			{
				CMaterialManager::GetMaterial(i)->CubeMapDiffuseAlbedo = CMaterialManager::GetMaterialCubeMap(m_ImGuiCB.MaterialIndex)->DiffuseAlbedo;
				CMaterialManager::GetMaterial(i)->NumFramesDirty = gNumFrameResources;
			}
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}