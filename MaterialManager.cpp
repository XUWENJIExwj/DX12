#include "Renderer.h"
#include "ImGuiManager.h"

using namespace std;
using namespace DirectX;

vector<const char*>          CMaterialManager::m_MaterialsName;
vector<unique_ptr<Material>> CMaterialManager::m_Materials((int)MaterialIndex::Material_Max);

void CMaterialManager::CreateMaterials()
{
	m_MaterialsName =
	{
		"Material_Logo_00",
		"Material_Bricks_00",
		"Material_Tile_00",
		"Material_Plane_00",
		"Material_Glass_00",
		"Material_Mirror_00",
		"Material_SkyCube_00",
	};

	for (int i = 0; i < (int)MaterialIndex::Material_Max; ++i)
	{
		auto material = make_unique<Material>();
		material->Name = m_MaterialsName[i];// m_MaterialsName[i];
		material->MatCBIndex = i;
		material->DiffuseSrvHeapIndex = i;
		material->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
		material->Roughness = 1.0f;
		m_Materials[i] = move(material);
	}

	// Customize
	m_Materials[(int)MaterialIndex::Material_Logo_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials[(int)MaterialIndex::Material_Logo_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_Materials[(int)MaterialIndex::Material_Logo_00]->Roughness = 1.0f;

	m_Materials[(int)MaterialIndex::Material_Bricks_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials[(int)MaterialIndex::Material_Bricks_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_Materials[(int)MaterialIndex::Material_Bricks_00]->Roughness = 0.3f;

	m_Materials[(int)MaterialIndex::Material_Tile_00]->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	m_Materials[(int)MaterialIndex::Material_Tile_00]->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	m_Materials[(int)MaterialIndex::Material_Tile_00]->Roughness = 0.1f;

	m_Materials[(int)MaterialIndex::Material_Plane_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials[(int)MaterialIndex::Material_Plane_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_Materials[(int)MaterialIndex::Material_Plane_00]->Roughness = 0.1f;

	m_Materials[(int)MaterialIndex::Material_Glass_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials[(int)MaterialIndex::Material_Glass_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_Materials[(int)MaterialIndex::Material_Glass_00]->Roughness = 1.0f;

	m_Materials[(int)MaterialIndex::Material_Mirror_00]->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_Materials[(int)MaterialIndex::Material_Mirror_00]->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	m_Materials[(int)MaterialIndex::Material_Mirror_00]->Roughness = 0.1f;

	m_Materials[(int)MaterialIndex::Material_SkyCube_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Materials[(int)MaterialIndex::Material_SkyCube_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_Materials[(int)MaterialIndex::Material_SkyCube_00]->Roughness = 1.0f;
}

void CMaterialManager::UpdateMaterial()
{
	static bool showClose = true;
	static int index = (int)MaterialIndex::Material_Plane_00;

	static bool alpha_preview = true;
	static bool alpha_half_preview = false;
	static bool drag_and_drop = true;
	static bool options_menu = true;
	static bool hdr = false;
	ImGuiColorEditFlags misc_flags = 
		(hdr ? ImGuiColorEditFlags_HDR : 0) |
		(drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) |
		(alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) |
		(options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

	if (showClose)
	{
		ImGui::SetNextWindowPos(ImVec2(700, 20), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin("MaterialManager", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		ImGui::Combo("MaterialList", &index, m_MaterialsName.data(), GetMaterialsCount());
		ImGui::PopItemWidth();

		if (ImGui::ColorEdit4("DiffuseAlbedo", (float*)&m_Materials[index]->DiffuseAlbedo, ImGuiColorEditFlags_Float | misc_flags) ||
			ImGui::ColorEdit3("FresnelR0", (float*)&m_Materials[index]->FresnelR0, ImGuiColorEditFlags_Float | misc_flags) ||
			ImGui::DragFloat("Roughness", &m_Materials[index]->Roughness, 0.01f, 0.0f, 0.99f))
		{
			m_Materials[index]->NumFramesDirty = gNumFrameResources;
		}
		
		ImGui::End();
	}
}