#include "Renderer.h"

using namespace std;

vector<const char*>         CTextureManager::m_TextureNames;
vector<unique_ptr<Texture>> CTextureManager::m_Textures((int)TextureIndex::Texture_Max);

UINT CTextureManager::m_NullTextureIndex = (int)TextureIndex::Texture_Null_Diffuse;
UINT CTextureManager::m_SkyCubeMapIndex = (int)TextureIndex::Texture_SkyCube_00_Diffuse;
UINT CTextureManager::m_ShadowMapIndex = (int)TextureIndex::Texture_Max;
UINT CTextureManager::m_ShadowMapNum = 3; // CascadeNum
UINT CTextureManager::m_PostProcessIndex = m_ShadowMapIndex + m_ShadowMapNum;
UINT CTextureManager::m_PostProcessNum = 3 * 2; // SRV��UAV�͂��ꂼ��J�E���g����̂ł�����2
UINT CTextureManager::m_NullCubeMapIndex = m_PostProcessIndex + m_PostProcessNum;
UINT CTextureManager::m_DynamicCubeMapIndex = m_NullCubeMapIndex + 1;
UINT CTextureManager::m_DynamicCubeMapsNum = 4; // DynamicCubeMap���K�v��Object�̐��������₵�Ă���

void CTextureManager::LoadTextures()
{
	m_TextureNames =
	{
		"Texture_Null_Diffuse",

		"Texture_Default_00_Diffuse", // NomalMap�̂���Textures���������珇���ǉ�
		"Texture_Logo_00_Diffuse",	
		"Texture_Tile_00_Diffuse",
		"Texture_Glass_00_Diffuse",

		"Texture_Bricks_00_Diffuse", // HeightMap�̂���Textures���������珇���ǉ�
		"Texture_Rocks_00_Diffuse",
		"Texture_Wall_00_Diffuse",
		"Texture_Plane_00_Diffuse",
		"Texture_Plane_01_Diffuse",
		"Texture_Plane_02_Diffuse",
		"Texture_Plane_03_Diffuse",

		"Texture_Null_Normal",

		"Texture_Default_00_Normal", // NormalMap(With No Height)���������珇���ǉ�
		"Texture_Logo_00_Normal",
		"Texture_Tile_00_Normal",
		"Texture_Glass_00_Normal",

		"Texture_Bricks_00_Normal", // NormalMap(With Height)���������珇���ǉ�
		"Texture_Rocks_00_Normal",
		"Texture_Wall_00_Normal",
		"Texture_Plane_00_Normal",
		"Texture_Plane_01_Normal",
		"Texture_Plane_02_Normal",
		"Texture_Plane_03_Normal",
		
		"Texture_Bricks_00_Height", // HeightMap���������珇���ǉ�
		"Texture_Rocks_00_Height",
		"Texture_Wall_00_Height",
		"Texture_Plane_00_Height",
		"Texture_Plane_01_Height",
		"Texture_Plane_02_Height",
		"Texture_Plane_03_Height",

		"Texture_SkyCube_00_Diffuse", // CubeMap���������珇���ǉ�
		"Texture_SkyCube_01_Diffuse",
		"Texture_SkyCube_02_Diffuse",
		"Texture_IndoorCube_00_Diffuse",
	};

	vector<wstring> texFilenames =
	{
		L"Asset\\Textures\\Default_00_Diffuse.dds", // ForNull

		L"Asset\\Textures\\Default_00_Diffuse.dds", // NomalMap�̂���Textures���������珇���ǉ�
		L"Asset\\Textures\\Logo_00_Diffuse.dds",
		L"Asset\\Textures\\Tile_00_Diffuse.dds",
		L"Asset\\Textures\\Glass_00_Diffuse.dds",

		L"Asset\\Textures\\Bricks_00_Diffuse.dds",  // HeightMap�̂���Textures���������珇���ǉ�
		L"Asset\\Textures\\Rocks_00_Diffuse.dds",
		L"Asset\\Textures\\Wall_00_Diffuse.dds",
		L"Asset\\Textures\\Plane_00_Diffuse.dds",
		L"Asset\\Textures\\Plane_01_Diffuse.dds",
		L"Asset\\Textures\\Plane_02_Diffuse.dds",
		L"Asset\\Textures\\Plane_03_Diffuse.dds",

		L"Asset\\Textures\\Default_00_Normal.dds", // NormalMap(With No Height)���������珇���ǉ�
		L"Asset\\Textures\\Default_00_Normal.dds",
		L"Asset\\Textures\\Default_00_Normal.dds",
		L"Asset\\Textures\\Tile_00_Normal.dds",
		L"Asset\\Textures\\Default_00_Normal.dds",

		L"Asset\\Textures\\Bricks_00_Normal.dds", // NormalMap(With Height)���������珇���ǉ�
		L"Asset\\Textures\\Rocks_00_Normal.dds",
		L"Asset\\Textures\\Wall_00_Normal.dds",
		L"Asset\\Textures\\Plane_00_Normal.dds",
		L"Asset\\Textures\\Plane_01_Normal.dds",
		L"Asset\\Textures\\Plane_02_Normal.dds",
		L"Asset\\Textures\\Plane_03_Normal.dds",

		L"Asset\\Textures\\Bricks_00_Height.dds", // HeightMap���������珇���ǉ�
		L"Asset\\Textures\\Rocks_00_Height.dds",
		L"Asset\\Textures\\Wall_00_Height.dds",
		L"Asset\\Textures\\Plane_00_Height.dds",
		L"Asset\\Textures\\Plane_01_Height.dds",
		L"Asset\\Textures\\Plane_02_Height.dds",
		L"Asset\\Textures\\Plane_03_Height.dds",

		L"Asset\\Textures\\SkyCube_00_Diffuse.dds", // CubeMap���������珇���ǉ�
		L"Asset\\Textures\\SkyCube_01_Diffuse.dds",
		L"Asset\\Textures\\SkyCube_02_Diffuse.dds",
		L"Asset\\Textures\\IndoorCube_00_Diffuse.dds",
	};

	// �\�����[�V�����̃r���h��̎��s�t�@�C����bin�t�H���_�̎��s�t�@�C����
	// �Q�Ƃ���Asset�̃f�B���N�g�����قȂ�̂ŁA�����𕪂���
	auto texture = make_unique<Texture>();
	texture->Name = m_TextureNames[0];
	texture->Filename = texFilenames[0];
	if (FAILED(DirectX::CreateDDSTextureFromFile12(CRenderer::GetDevice(),
		CRenderer::GetCommandList(), texture->Filename.c_str(),
		texture->Resource, texture->UploadHeap)))
	{
		for (int i = 0; i < (int)TextureIndex::Texture_Max; ++i)
		{
			texFilenames[i] = L"..\\" + texFilenames[i];
			LoadEachTexture(i, texFilenames[i]);
		}
	}
	else
	{
		m_Textures[0] = move(texture);
		for (int i = 1; i < (int)TextureIndex::Texture_Max; ++i)
		{
			LoadEachTexture(i, texFilenames[i]);
		}
	}
}

void CTextureManager::LoadEachTexture(int Index, const wstring& Filename)
{
	auto texture = make_unique<Texture>();
	texture->Name = m_TextureNames[Index];
	texture->Filename = Filename;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(CRenderer::GetDevice(),
		CRenderer::GetCommandList(), texture->Filename.c_str(),
		texture->Resource, texture->UploadHeap));

	m_Textures[Index] = move(texture);
}
