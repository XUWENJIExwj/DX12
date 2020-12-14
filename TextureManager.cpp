#include "Renderer.h"

using namespace std;

vector<const char*>         CTextureManager::m_TexturesName;
vector<unique_ptr<Texture>> CTextureManager::m_Textures((int)TextureIndex::Texture_Max);

UINT CTextureManager::m_SkyTextureIndex = (int)TextureIndex::Texture_SkyCube_00_Diffuse;
UINT CTextureManager::m_DynamicTextureIndex = (int)TextureIndex::Texture_Max;
UINT CTextureManager::m_DynamicTextureNum = 4;

void CTextureManager::LoadTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	m_TexturesName =
	{
		"Texture_Default_00_Diffuse",
		"Texture_Default_00_Normal",
		"Texture_Logo_00_Diffuse",
		"Texture_Logo_00_Normal",	
		"Texture_Bricks_00_Diffuse",
		"Texture_Bricks_00_Normal",
		"Texture_Tile_00_Diffuse",
		"Texture_Tile_00_Normal",
		"Texture_Plane_00_Diffuse",
		"Texture_Plane_00_Normal",
		"Texture_Glass_00_Diffuse",
		"Texture_Glass_00_Normal",
		"Texture_SkyCube_00_Diffuse",
		"Texture_SkyCube_01_Diffuse",
		"Texture_SkyCube_02_Diffuse",
		"Texture_IndoorCube_00_Diffuse",
	};

	vector<wstring> texFilenames =
	{
		L"Asset\\Textures\\Default_00_Diffuse.dds",
		L"Asset\\Textures\\Default_00_Normal.dds",
		L"Asset\\Textures\\Logo_00_Diffuse.dds",
		L"Asset\\Textures\\Default_00_Normal.dds",
		L"Asset\\Textures\\Bricks_00_Diffuse.dds",
		L"Asset\\Textures\\Bricks_00_Normal.dds",
		L"Asset\\Textures\\Tile_00_Diffuse.dds",
		L"Asset\\Textures\\Tile_00_Normal.dds",
		L"Asset\\Textures\\Plane_00_Diffuse.dds",
		L"Asset\\Textures\\Plane_00_Normal.dds",
		L"Asset\\Textures\\Glass_00_Diffuse.dds",
		L"Asset\\Textures\\Default_00_Normal.dds",
		L"Asset\\Textures\\SkyCube_00_Diffuse.dds",
		L"Asset\\Textures\\SkyCube_01_Diffuse.dds",
		L"Asset\\Textures\\SkyCube_02_Diffuse.dds",
		L"Asset\\Textures\\IndoorCube_00_Diffuse.dds",
	};

	for (int i = 0; i < (int)TextureIndex::Texture_Max; ++i)
	{
		auto texMap = make_unique<Texture>();
		texMap->Name = m_TexturesName[i];
		texMap->Filename = texFilenames[i];
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(Device,
			CommandList, texMap->Filename.c_str(),
			texMap->Resource, texMap->UploadHeap));

		m_Textures[i] = move(texMap);
	}
}