#pragma once

enum class TextureIndex :int
{
	Texture_Default_00_Diffuse,// NomalMap‚Ì‚ ‚éTextures‚ğ‚±‚±‚©‚ç‡Ÿ’Ç‰Á
	Texture_Default_01_Diffuse,
	Texture_Logo_00_Diffuse,
	Texture_Bricks_00_Diffuse,
	Texture_Tile_00_Diffuse,
	Texture_Glass_00_Diffuse,

	Texture_Plane_00_Diffuse, // HeightMap‚Ì‚ ‚éTextures‚ğ‚±‚±‚©‚ç‡Ÿ’Ç‰Á

	Texture_Default_00_Normal, // NormalMap(With No Height)‚ğ‚±‚±‚©‚ç‡Ÿ’Ç‰Á
	Texture_Default_01_Normal,
	Texture_Logo_00_Normal,
	Texture_Bricks_00_Normal,
	Texture_Tile_00_Normal,
	Texture_Glass_00_Normal,

	Texture_Plane_00_Normal, // NormalMap(With Height)‚ğ‚±‚±‚©‚ç‡Ÿ’Ç‰Á

	Texture_Plane_00_Height, // HeightMap‚ğ‚±‚±‚©‚ç‡Ÿ’Ç‰Á

	Texture_SkyCube_00_Diffuse, // CubeMap‚ğ‚±‚±‚©‚ç‡Ÿ’Ç‰Á
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