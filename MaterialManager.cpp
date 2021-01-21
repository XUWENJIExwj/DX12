#include "Renderer.h"
#include "ImGuiManager.h"

using namespace std;
using namespace DirectX;

vector<const char*> CMaterialManager::m_MaterialNormalNames;
vector<Material*>   CMaterialManager::m_MaterialNormal((int)MaterialNormalIndex::Material_Max);

vector<const char*> CMaterialManager::m_MaterialHeightNames;
vector<Material*>   CMaterialManager::m_MaterialHeight((int)MaterialHeightIndex::Material_Max);

vector<const char*> CMaterialManager::m_MaterialCubeMapNames;
vector<Material*>   CMaterialManager::m_MaterialCubeMap((int)MaterialCubeMapIndex::Material_Max);

vector<unique_ptr<Material>> CMaterialManager::m_AllMaterials;

void CMaterialManager::CreateMaterials()
{
	m_AllMaterials.reserve((int)MaterialNormalIndex::Material_Max + (int)MaterialHeightIndex::Material_Max + (int)MaterialCubeMapIndex::Material_Max);
	CreateMaterialNormal();
	CreateMaterialHeight();
	CreateMaterialCubeMap();
}

void CMaterialManager::CreateMaterialNormal()
{
	m_MaterialNormalNames =
	{
		"Material_Null",

		"Material_Mirror_00", // NormalMap(With No Height)
		"Material_Logo_00",
		"Material_Tile_00",
		"Material_Glass_00",

		"Material_Bricks_00", // NormalMap(With Height)
		"Material_Rocks_00",
		"Material_Wall_00",
		"Material_Plane_00",
		"Material_Plane_01",
		"Material_Plane_02",
		"Material_Plane_03",

		"Material_ShadowMap_00",
		"Material_ShadowMap_01",
		"Material_ShadowMap_02",
	};

	for (int i = 0; i < (int)MaterialNormalIndex::Material_Max; ++i)
	{
		auto material = make_unique<Material>();
		material->Name = m_MaterialNormalNames[i];
		material->MatCBIndex = i;
		material->DiffuseSrvHeapIndex = i + (int)TextureIndex::Texture_Null_Diffuse;
		material->NormalSrvHeapIndex = i + (int)TextureIndex::Texture_Null_Normal;
		material->BitangentSign = 1;
		material->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
		material->Roughness = 0.99f;
		m_MaterialNormal[i] = material.get();
		m_AllMaterials.push_back(move(material));
	}

	// Customize
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Null]->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Null]->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Null]->Roughness = 0.1f;

	m_MaterialNormal[(int)MaterialNormalIndex::Material_Mirror_00]->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Mirror_00]->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Mirror_00]->Roughness = 0.1f;

	m_MaterialNormal[(int)MaterialNormalIndex::Material_Tile_00]->BitangentSign = -1;
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Tile_00]->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Tile_00]->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Tile_00]->Roughness = 0.1f;

	m_MaterialNormal[(int)MaterialNormalIndex::Material_Bricks_00]->BitangentSign = 1;
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Bricks_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Bricks_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Bricks_00]->Roughness = 0.3f;

	m_MaterialNormal[(int)MaterialNormalIndex::Material_Plane_00]->BitangentSign = 1;
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Plane_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Plane_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_MaterialNormal[(int)MaterialNormalIndex::Material_Plane_00]->Roughness = 0.1f;

	for (UINT i = 0; i < CTextureManager::GetShadowMapNum(); ++i)
	{
		m_MaterialNormal[(int)MaterialNormalIndex::Material_ShadowMap_00 + i]->DiffuseSrvHeapIndex = CTextureManager::GetShadowMapIndex(i);
		m_MaterialNormal[(int)MaterialNormalIndex::Material_ShadowMap_00 + i]->NormalSrvHeapIndex = (int)TextureIndex::Texture_Null_Normal;
		m_MaterialNormal[(int)MaterialNormalIndex::Material_ShadowMap_00 + i]->FresnelR0 = XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_MaterialNormal[(int)MaterialNormalIndex::Material_ShadowMap_00 + i]->CascadeDebugIndex = i;
	}

}

void CMaterialManager::CreateMaterialHeight()
{
	m_MaterialHeightNames =
	{
		"Material_Bricks_00",
		"Material_Rocks_00",
		"Material_Wall_00",
		"Material_Plane_00",
		"Material_Plane_01",
		"Material_Plane_02",
		"Material_Plane_03",
	};

	int materialCountBeforeThis = (int)MaterialNormalIndex::Material_Max;

	for (int i = 0; i < (int)MaterialHeightIndex::Material_Max; ++i)
	{
		auto material = make_unique<Material>();
		material->Name = m_MaterialHeightNames[i];
		material->MatCBIndex = i + materialCountBeforeThis;
		material->DiffuseSrvHeapIndex = i + (int)TextureIndex::Texture_Bricks_00_Diffuse;
		material->NormalSrvHeapIndex = i + (int)TextureIndex::Texture_Bricks_00_Normal;
		material->HeightSrvHeapIndex = i + (int)TextureIndex::Texture_Bricks_00_Height;
		material->BitangentSign = 1;
		material->UseACForPOM = 0;
		material->MaxSampleCount = 128;
		material->MinSampleCount = 8;
		material->HeightScale = 0.01f;
		material->ShadowSoftening = 0.9f;
		material->ShowSelfShadow = true;
		material->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
		material->Roughness = 0.1f;
		m_MaterialHeight[i] = material.get();
		m_AllMaterials.push_back(move(material));
	}

	// Customize
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Bricks_00]->UseACForPOM = 1;
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Bricks_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Bricks_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Bricks_00]->Roughness = 0.3f;

	m_MaterialHeight[(int)MaterialHeightIndex::Material_Rocks_00]->UseACForPOM = 1;
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Rocks_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Rocks_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Rocks_00]->Roughness = 0.3f;

	m_MaterialHeight[(int)MaterialHeightIndex::Material_Wall_00]->UseACForPOM = 1;
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Wall_00]->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Wall_00]->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	m_MaterialHeight[(int)MaterialHeightIndex::Material_Wall_00]->Roughness = 0.3f;
}

void CMaterialManager::CreateMaterialCubeMap()
{
	m_MaterialCubeMapNames =
	{
		"Material_SkyCube_00",
		"Material_SkyCube_01",
		"Material_SkyCube_02",
		"Material_IndoorCube_00",
	};

	int materialCountBeforeThis = (int)MaterialNormalIndex::Material_Max + (int)MaterialHeightIndex::Material_Max;

	for (int i = 0; i < (int)MaterialCubeMapIndex::Material_Max; ++i)
	{
		auto material = make_unique<Material>();
		material->Name = m_MaterialCubeMapNames[i];
		material->MatCBIndex = i + materialCountBeforeThis;
		material->DiffuseSrvHeapIndex = i + (int)TextureIndex::Texture_SkyCube_00_Diffuse;
		material->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
		material->Roughness = 0.99f;
		m_MaterialCubeMap[i] = material.get();
		m_AllMaterials.push_back(move(material));
	}
}

void CMaterialManager::UpdateMaterial()
{
	static DX12App* app = DX12App::GetApp();
	static bool showClose = true;
	static int normalMapIndex = (int)MaterialNormalIndex::Material_Tile_00;
	static int heightMapIndex = (int)MaterialHeightIndex::Material_Plane_00;
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

	static bool reverseBitangentNormalMap;
	static bool reverseBitangentHeightMap;
	static bool showSelfShadow;

	if (showClose)
	{
		ImGui::SetNextWindowPos(ImVec2((float)app->GetWindowWidth() - 420, (float)app->GetWindowHeight() - 440), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(400, 420), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"MaterialManager", &showClose, window_flags);

		// MaterialNormal
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		ImGui::Combo(u8"MaterialNormalList", &normalMapIndex, m_MaterialNormalNames.data(), GetMaterialNormalCount());
		ImGui::PopItemWidth();
		reverseBitangentNormalMap = m_MaterialNormal[normalMapIndex]->BitangentSign > 0 ? false : true;
		if (ImGui::ColorEdit4(u8"DiffuseAlbedo##0", (float*)&m_MaterialNormal[normalMapIndex]->DiffuseAlbedo, ImGuiColorEditFlags_Float | misc_flags)) m_MaterialNormal[normalMapIndex]->NumFramesDirty = gNumFrameResources;
		if (ImGui::ColorEdit3(u8"FresnelR0##0", (float*)&m_MaterialNormal[normalMapIndex]->FresnelR0, ImGuiColorEditFlags_Float | misc_flags)) m_MaterialNormal[normalMapIndex]->NumFramesDirty = gNumFrameResources;
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);
		if (ImGui::SliderFloat(u8"Roughness##0", &m_MaterialNormal[normalMapIndex]->Roughness, 0.0f, 1.0f)) m_MaterialNormal[normalMapIndex]->NumFramesDirty = gNumFrameResources;
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Checkbox(u8"ReverseBitangent##0", &reverseBitangentNormalMap))
		{
			m_MaterialNormal[normalMapIndex]->BitangentSign = reverseBitangentNormalMap ? -1 : 1;
			m_MaterialNormal[normalMapIndex]->NumFramesDirty = gNumFrameResources;
		}
		ImGui::Separator();

		// MaterialHeight
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		ImGui::Combo(u8"MaterialHeightList", &heightMapIndex, m_MaterialHeightNames.data(), GetMaterialHeightCount());
		ImGui::PopItemWidth();
		reverseBitangentHeightMap = m_MaterialHeight[heightMapIndex]->BitangentSign > 0 ? false : true;
		if (ImGui::ColorEdit4(u8"DiffuseAlbedo##1", (float*)&m_MaterialHeight[heightMapIndex]->DiffuseAlbedo, ImGuiColorEditFlags_Float | misc_flags)) m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		if (ImGui::ColorEdit3(u8"FresnelR0##1", (float*)&m_MaterialHeight[heightMapIndex]->FresnelR0, ImGuiColorEditFlags_Float | misc_flags)) m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);
		if (ImGui::SliderFloat(u8"Roughness##1", &m_MaterialHeight[heightMapIndex]->Roughness, 0.0f, 1.0f)) m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Checkbox(u8"ReverseBitangent##1", &reverseBitangentHeightMap))
		{
			m_MaterialHeight[heightMapIndex]->BitangentSign = reverseBitangentHeightMap ? -1 : 1;
			m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		}
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.2f);
		if (ImGui::SliderInt(u8"MinSampleCount##1", &m_MaterialHeight[heightMapIndex]->MinSampleCount, 8, 64)) m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		ImGui::SameLine();
		if (ImGui::SliderInt(u8"MaxSampleCount##1", &m_MaterialHeight[heightMapIndex]->MaxSampleCount, 64, 256)) m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		if (ImGui::SliderFloat(u8"HeightScale##1", &m_MaterialHeight[heightMapIndex]->HeightScale, 0.0f, 0.1f)) m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		showSelfShadow = m_MaterialHeight[heightMapIndex]->ShowSelfShadow;
		if (ImGui::Checkbox(u8"ShowSelfShadow##1", &showSelfShadow))
		{
			m_MaterialHeight[heightMapIndex]->ShowSelfShadow = showSelfShadow;
			m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		}
		ImGui::SameLine();
		if (ImGui::SliderFloat(u8"ShadowSoftening##1", &m_MaterialHeight[heightMapIndex]->ShadowSoftening, 0.0f, 2.0f)) m_MaterialHeight[heightMapIndex]->NumFramesDirty = gNumFrameResources;
		ImGui::PopItemWidth();
		ImGui::Separator();

		// MaterialCubeMap
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
		ImGui::Combo(u8"MaterialCubeMapList", &cubeMapIndex, m_MaterialCubeMapNames.data(), GetMaterialCubeMapCount());
		ImGui::PopItemWidth();
		if (ImGui::ColorEdit4(u8"DiffuseAlbedo##2", (float*)&m_MaterialCubeMap[cubeMapIndex]->DiffuseAlbedo, ImGuiColorEditFlags_Float | misc_flags) &&
			CRenderer::GetCurrentSkyCubeMapIndex() == cubeMapIndex)
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