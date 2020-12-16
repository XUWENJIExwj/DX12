#pragma once

enum class TextureIndex :int
{
	Texture_Default_00_Diffuse,// NomalMap�̂���Textures���������珇���ǉ�
	Texture_Default_01_Diffuse,
	Texture_Logo_00_Diffuse,
	Texture_Tile_00_Diffuse,
	Texture_Glass_00_Diffuse,

	Texture_Bricks_00_Diffuse, // HeightMap�̂���Textures���������珇���ǉ�
	Texture_Rocks_00_Diffuse,
	Texture_Wall_00_Diffuse,
	Texture_Plane_00_Diffuse,
	Texture_Plane_01_Diffuse,
	Texture_Plane_02_Diffuse,
	Texture_Plane_03_Diffuse,

	Texture_Default_00_Normal, // NormalMap(With No Height)���������珇���ǉ�
	Texture_Default_01_Normal,
	Texture_Logo_00_Normal,
	Texture_Tile_00_Normal,
	Texture_Glass_00_Normal,

	Texture_Bricks_00_Normal, // NormalMap(With Height)���������珇���ǉ�
	Texture_Rocks_00_Normal,
	Texture_Wall_00_Normal,
	Texture_Plane_00_Normal,
	Texture_Plane_01_Normal,
	Texture_Plane_02_Normal,
	Texture_Plane_03_Normal,

	Texture_Bricks_00_Height, // HeightMap���������珇���ǉ�
	Texture_Rocks_00_Height,
	Texture_Wall_00_Height,
	Texture_Plane_00_Height,
	Texture_Plane_01_Height,
	Texture_Plane_02_Height,
	Texture_Plane_03_Height,

	Texture_SkyCube_00_Diffuse, // CubeMap���������珇���ǉ�
	Texture_SkyCube_01_Diffuse,
	Texture_SkyCube_02_Diffuse,
	Texture_IndoorCube_00_Diffuse,
	Texture_Max
};

class CTextureManager
{
private:
	static std::vector<const char*>              m_TextureNames;
	static std::vector<std::unique_ptr<Texture>> m_Textures;

	static UINT m_SkyCubeMapIndex;
	static UINT m_DynamicCubeMapIndex;
	static UINT m_DynamicCubeMapsNum;

public:
	static void LoadTextures();

	static std::vector<std::unique_ptr<Texture>>& GetTextures() { return m_Textures; }
	static UINT GetTexturesNum() { return (UINT)m_Textures.size(); }
	static UINT GetSkyCubeMapIndex() { return m_SkyCubeMapIndex; }
	static UINT GetDynamicCubeMapIndex() { return m_DynamicCubeMapIndex; }
	static UINT GetDynamicCubeMapsNum() { return m_DynamicCubeMapsNum; }
};