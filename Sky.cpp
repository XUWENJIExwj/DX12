#include "Renderer.h"
#include "Sky.h"

using namespace DirectX;

void CSky::Init()
{ 
	XMStoreFloat3(&m_Scale, GetScale() * 1000.0f);
	XMStoreFloat4x4(&m_World, XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z));
	m_Material = CMaterialManager::GetMaterialCubeMap((int)MaterialCubeMapIndex::Material_SkyCube_00);
	m_MeshGeometry = CGeoShapeManager::GetMeshGeometry((int)GeoShapeType::Type_00_GeoShapes);
	m_IndexCount = m_MeshGeometry->DrawArgs["sphere"].IndexCount;
	m_StartIndexLocation = m_MeshGeometry->DrawArgs["sphere"].StartIndexLocation;
	m_BaseVertexLocation = m_MeshGeometry->DrawArgs["sphere"].BaseVertexLocation;
}

void CSky::UpdateImGui(const GameTimer & GlobalTimer)
{
	static bool showClose = true;
	static int index = (int)MaterialCubeMapIndex::Material_SkyCube_00;

	if (showClose)
	{
		ImGui::SetNextWindowPos(ImVec2(20, 190), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 65), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(m_Name.c_str(), &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.55f);

		if (ImGui::Combo("MaterialList", &index, CMaterialManager::GetMaterialCubeMapNames(), CMaterialManager::GetMaterialCubeMapCount()))
		{
			m_Material = CMaterialManager::GetMaterialCubeMap(index);
			m_NumFramesDirty = gNumFrameResources;
			CRenderer::SetCurrentCubeMapIndex(index);
			for (int i = 1; i < CMaterialManager::GetAllMaterialsCount(); ++i)
			{
				CMaterialManager::GetMaterial(i)->CubeMapDiffuseAlbedo = CMaterialManager::GetMaterialCubeMap(index)->DiffuseAlbedo;
				CMaterialManager::GetMaterial(i)->NumFramesDirty = gNumFrameResources;
			}
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}