#include "Renderer.h"
//#include "TextureManager.h"

using namespace std;

vector<string>              CTextureManager::m_TexturesName((int)TextureIndex::Texture_Max);
vector<unique_ptr<Texture>> CTextureManager::m_Textures((int)TextureIndex::Texture_Max);

UINT CTextureManager::m_SkyTextureIndex = (int)TextureIndex::Texture_SkyCube_00;
UINT CTextureManager::m_DynamicTextureIndex = (int)TextureIndex::Texture_Max;
UINT CTextureManager::m_DynamicTextureNum = 3;

void CTextureManager::LoadTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	m_TexturesName =
	{
		"Texture_Logo_00",
		"Texture_Bricks_00",
		"Texture_Tile_00",
		"Texture_Glass_00",
		"Texture_Default_00",
		"Texture_SkyCube_00"
	};

	vector<wstring> texFilenames =
	{
		L"Asset\\Textures\\Logo_00.dds",
		L"Asset\\Textures\\Bricks_00.dds",
		L"Asset\\Textures\\Titles_00.dds",
		L"Asset\\Textures\\Glass_00.dds",
		L"Asset\\Textures\\Default_00.dds",
		L"Asset\\Textures\\SkyCube_00.dds"
	};

	for (int i = 0; i < (int)TextureIndex::Texture_Max; ++i)
	{
		auto texMap = make_unique<Texture>();
		texMap->Name = m_TexturesName[i];
		texMap->Filename = texFilenames[i];
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(Device,
			CommandList, texMap->Filename.c_str(),
			texMap->Resource, texMap->UploadHeap));

		m_Textures[i] = std::move(texMap);
	}
}