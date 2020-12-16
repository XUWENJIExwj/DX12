#include "Renderer.h"

using namespace std;

vector<const char*>         CTextureManager::m_TextureNames;
vector<unique_ptr<Texture>> CTextureManager::m_Textures((int)TextureIndex::Texture_Max);

UINT CTextureManager::m_SkyCubeMapIndex = (int)TextureIndex::Texture_SkyCube_00_Diffuse;
UINT CTextureManager::m_DynamicCubeMapIndex = (int)TextureIndex::Texture_Max;
UINT CTextureManager::m_DynamicCubeMapsNum = 4; // DynamicCubeMapが必要なObjectの数だけ増やしていく

void CTextureManager::LoadTextures()
{
	m_TextureNames =
	{
		"Texture_Default_00_Diffuse", // NomalMapのあるTexturesをここから順次追加
		"Texture_Default_01_Diffuse",
		"Texture_Logo_00_Diffuse",	
		"Texture_Tile_00_Diffuse",
		"Texture_Glass_00_Diffuse",

		"Texture_Bricks_00_Diffuse", // HeightMapのあるTexturesをここから順次追加
		"Texture_Rocks_00_Diffuse",
		"Texture_Wall_00_Diffuse",
		"Texture_Plane_00_Diffuse",
		"Texture_Plane_01_Diffuse",
		"Texture_Plane_02_Diffuse",
		"Texture_Plane_03_Diffuse",

		"Texture_Default_00_Normal", // NormalMap(With No Height)をここから順次追加
		"Texture_Default_01_Normal",
		"Texture_Logo_00_Normal",
		"Texture_Tile_00_Normal",
		"Texture_Glass_00_Normal",

		"Texture_Bricks_00_Normal", // NormalMap(With Height)をここから順次追加
		"Texture_Rocks_00_Normal",
		"Texture_Wall_00_Normal",
		"Texture_Plane_00_Normal",
		"Texture_Plane_01_Normal",
		"Texture_Plane_02_Normal",
		"Texture_Plane_03_Normal",
		
		"Texture_Bricks_00_Height", // HeightMapをここから順次追加
		"Texture_Rocks_00_Height",
		"Texture_Wall_00_Height",
		"Texture_Plane_00_Height",
		"Texture_Plane_01_Height",
		"Texture_Plane_02_Height",
		"Texture_Plane_03_Height",

		"Texture_SkyCube_00_Diffuse", // CubeMapをここから順次追加
		"Texture_SkyCube_01_Diffuse",
		"Texture_SkyCube_02_Diffuse",
		"Texture_IndoorCube_00_Diffuse",
	};

	vector<wstring> texFilenames =
	{
		L"Asset\\Textures\\Default_00_Diffuse.dds", // NomalMapのあるTexturesをここから順次追加
		L"Asset\\Textures\\Default_00_Diffuse.dds",
		L"Asset\\Textures\\Logo_00_Diffuse.dds",
		L"Asset\\Textures\\Tile_00_Diffuse.dds",
		L"Asset\\Textures\\Glass_00_Diffuse.dds",

		L"Asset\\Textures\\Bricks_00_Diffuse.dds",  // HeightMapのあるTexturesをここから順次追加
		L"Asset\\Textures\\Rocks_00_Diffuse.dds",
		L"Asset\\Textures\\Wall_00_Diffuse.dds",
		L"Asset\\Textures\\Plane_00_Diffuse.dds",
		L"Asset\\Textures\\Plane_01_Diffuse.dds",
		L"Asset\\Textures\\Plane_02_Diffuse.dds",
		L"Asset\\Textures\\Plane_03_Diffuse.dds",

		L"Asset\\Textures\\Default_00_Normal.dds", // NormalMap(With No Height)をここから順次追加
		L"Asset\\Textures\\Default_00_Normal.dds",
		L"Asset\\Textures\\Default_00_Normal.dds",
		L"Asset\\Textures\\Tile_00_Normal.dds",
		L"Asset\\Textures\\Default_00_Normal.dds",

		L"Asset\\Textures\\Bricks_00_Normal.dds", // NormalMap(With Height)をここから順次追加
		L"Asset\\Textures\\Rocks_00_Normal.dds",
		L"Asset\\Textures\\Wall_00_Normal.dds",
		L"Asset\\Textures\\Plane_00_Normal.dds",
		L"Asset\\Textures\\Plane_01_Normal.dds",
		L"Asset\\Textures\\Plane_02_Normal.dds",
		L"Asset\\Textures\\Plane_03_Normal.dds",

		L"Asset\\Textures\\Bricks_00_Height.dds", // HeightMapをここから順次追加
		L"Asset\\Textures\\Rocks_00_Height.dds",
		L"Asset\\Textures\\Wall_00_Height.dds",
		L"Asset\\Textures\\Plane_00_Height.dds",
		L"Asset\\Textures\\Plane_01_Height.dds",
		L"Asset\\Textures\\Plane_02_Height.dds",
		L"Asset\\Textures\\Plane_03_Height.dds",

		L"Asset\\Textures\\SkyCube_00_Diffuse.dds", // CubeMapをここから順次追加
		L"Asset\\Textures\\SkyCube_01_Diffuse.dds",
		L"Asset\\Textures\\SkyCube_02_Diffuse.dds",
		L"Asset\\Textures\\IndoorCube_00_Diffuse.dds",
	};

	for (int i = 0; i < (int)TextureIndex::Texture_Max; ++i)
	{
		auto texture = make_unique<Texture>();
		texture->Name = m_TextureNames[i];
		texture->Filename = texFilenames[i];
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(CRenderer::GetDevice(),
			CRenderer::GetCommandList(), texture->Filename.c_str(),
			texture->Resource, texture->UploadHeap));

		m_Textures[i] = move(texture);
	}
}