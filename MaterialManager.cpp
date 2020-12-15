#include "Renderer.h"
#include "ImGuiManager.h"

using namespace std;
using namespace DirectX;

vector<const char*> CMaterialManager::m_MaterialTexNames;
vector<Material*>   CMaterialManager::m_MaterialTex((int)MaterialTexIndex::Material_Max);

vector<const char*> CMaterialManager::m_MaterialCubeMapNames;
vector<Material*>   CMaterialManager::m_MaterialCubeMap((int)MaterialCubeMapIndex::Material_Max);

vector<unique_ptr<Material>> CMaterialManager::m_AllMaterials;

void CMaterialManager::CreateMaterials()
{
	m_AllMaterials.reserve((int)MaterialTexIndex::Material_Max + (int)MaterialCubeMapIndex::Material_Max);

	m_MaterialTexNames =
	{
		"Material_Mirror_00",
		"Material_Mirror_01",
		"Material_Logo_00",
		"Material_Bricks_00",
		"Material_Tile_00",
		"Material_Plane_00",
		"Material_Glass_00",
	};

	for (int i = 0; i < (int)MaterialTexIndex::Material_Max; ++i)
	{
		auto material = make_unique<Material>();
		material->Name = m_MaterialTexNames[i];
		material->MatCBIndex = i;
		material->DiffuseSrvHeapIndex = i;
		material->NormalSrvHeapIndex = i + (int)MaterialTexIndex::Material_Max;
		material->BitangentSign = 1;
		material->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
		material->Roughness = 0.99f;
		m_MaterialTex[i] = material.get();
		m_AllMaterials.push_back(move(material));
	}

	m_MaterialCubeMapNames =
	{
		"Material_SkyCube_00",
		"Material_SkyCube_01",
		"Material_SkyCube_02",
		"Material_IndoorCube_00",
	};

	for (int i = 0; i < (int)MaterialCubeMapIndex::Material_Max; ++i)
	{
		auto material = make_unique<Material>();
		material->Name = m_MaterialCubeMapNames[i];
		material->MatCBIndex = i + (int)MaterialTexIndex::Material_Max;
		material->DiffuseSrvHeapIndex = (int)TextureIndex::Texture_SkyCube_00_Diffuse + i;
		material->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
		material->Roughness = 0.99f;
		m_MaterialCubeMap[i] = material.get();
		m_AllMaterials.push_back(move(material));
	}

	// Customize
	m_MaterialTex[(int)MaterialTexIndex::Material_Mirror_Dynamic_00]->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Mirror_Dynamic_00]->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Mirror_Dynamic_00]->Roughness = 0.1f;

	m_MaterialTex[(int)MaterialTexIndex::Material_Mirror_00]->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Mirror_00]->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Mirror_00]->Roughness = 0.1f;

	m_MaterialTex[(int)MaterialTexIndex::Material_Bricks_00]->BitangentSign = 1;
	m_MaterialTex[(int)MaterialTexIndex::Material_Bricks_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Bricks_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Bricks_00]->Roughness = 0.3f;

	m_MaterialTex[(int)MaterialTexIndex::Material_Tile_00]->BitangentSign = -1;
	m_MaterialTex[(int)MaterialTexIndex::Material_Tile_00]->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Tile_00]->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Tile_00]->Roughness = 0.1f;

	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->HeightSrvHeapIndex = (int)TextureIndex::Texture_Plane_00_Height;
	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->UseACForPOM = 0;
	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->MaxSampleCount = 16;
	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->MinSampleCount = 8;
	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->BitangentSign = 1;
	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->HeightScale = 0.1f;
	// m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->ShowSelfShadow = true; // boolŒ^‚ÍƒoƒO‚é
	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_MaterialTex[(int)MaterialTexIndex::Material_Plane_00]->Roughness = 0.1f;
}

void CMaterialManager::UpdateMaterial()
{
	static bool showClose = true;
	static int texIndex = (int)MaterialTexIndex::Material_Plane_00;
	static int cubeMapIndex = (int)MaterialCubeMapIndex::Material_SkyCube_00;

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

	static bool reverseBitangent;

	if (showClose)
	{
		ImGui::SetNextWindowPos(ImVec2(840, 20), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin("MaterialManager", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);
		ImGui::Combo("MaterialTexList", &texIndex, m_MaterialTexNames.data(), GetMaterialTexCount());
		ImGui::PopItemWidth();

		reverseBitangent = m_MaterialTex[texIndex]->BitangentSign > 0 ? false : true;

		if (ImGui::ColorEdit4("DiffuseAlbedo##0", (float*)&m_MaterialTex[texIndex]->DiffuseAlbedo, ImGuiColorEditFlags_Float | misc_flags) ||
			ImGui::ColorEdit3("FresnelR0##0", (float*)&m_MaterialTex[texIndex]->FresnelR0, ImGuiColorEditFlags_Float | misc_flags) ||
			ImGui::SliderFloat("Roughness##0", &m_MaterialTex[texIndex]->Roughness, 0.0f, 1.0f) ||
			ImGui::Checkbox("ReverseBitangent##0", &reverseBitangent) ||
			ImGui::SliderFloat("Height##0", &m_MaterialTex[texIndex]->HeightScale, 0.0f, 0.3f))
		{
			m_MaterialTex[texIndex]->BitangentSign = reverseBitangent ? -1 : 1;

			m_MaterialTex[texIndex]->NumFramesDirty = gNumFrameResources;
		}

		ImGui::Separator();

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		ImGui::Combo("MaterialCubeMapList", &cubeMapIndex, m_MaterialCubeMapNames.data(), GetMaterialCubeMapCount());
		ImGui::PopItemWidth();

		if (ImGui::ColorEdit4("DiffuseAlbedo##1", (float*)&m_MaterialCubeMap[cubeMapIndex]->DiffuseAlbedo, ImGuiColorEditFlags_Float | misc_flags) &&
			CRenderer::GetCurrentCubeMapIndex() == cubeMapIndex)
		{
			for (int i = 1; i < (int)m_AllMaterials.size(); ++i)
			{
				m_AllMaterials[i]->CubeMapDiffuseAlbedo = m_MaterialCubeMap[cubeMapIndex]->DiffuseAlbedo;
				m_AllMaterials[i]->NumFramesDirty = gNumFrameResources;
			}
		}
		
		ImGui::End();
	}
}