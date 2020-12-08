#include "Renderer.h"
//#include "TextureManager.h"

using namespace std;

vector<string>              CTextureManager::m_TexturesName((int)TextureIndex::Texture_Max);
vector<unique_ptr<Texture>> CTextureManager::m_Textures((int)TextureIndex::Texture_Max);

UINT CTextureManager::m_SkyTextureIndex = (int)TextureIndex::Texture_03_SkyCube;
UINT CTextureManager::m_DynamicTextureIndex = m_SkyTextureIndex + 1;

void CTextureManager::LoadTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	m_TexturesName =
	{
		"Texture_00_Bricks",
		"Texture_01_Tile",
		"Texture_02_Default",
		"Texture_03_SkyCube"
	};

	vector<wstring> texFilenames =
	{
		L"Asset\\Textures\\Bricks_00.dds",
		L"Asset\\Textures\\Titles_00.dds",
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