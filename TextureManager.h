#pragma once

enum class TextureIndex :int
{
	Texture_Default_00_Diffuse,
	Texture_Default_01_Diffuse,
	Texture_Logo_00_Diffuse,
	Texture_Bricks_00_Diffuse,
	Texture_Tile_00_Diffuse,
	Texture_Plane_00_Diffuse,
	Texture_Glass_00_Diffuse,
	Texture_Default_00_Normal,
	Texture_Default_01_Normal,
	Texture_Logo_00_Normal,
	Texture_Bricks_00_Normal,
	Texture_Tile_00_Normal,
	Texture_Plane_00_Normal,
	Texture_Glass_00_Normal,
	//Texture_Plane_00_Height, // HeightMap
	Texture_SkyCube_00_Diffuse, // CubeMap
	Texture_SkyCube_01_Diffuse,
	Texture_SkyCube_02_Diffuse,
	Texture_IndoorCube_00_Diffuse,
	Texture_Max
};

class CTextureManager
{
private:
	static std::vector<const char*>              m_TexturesName;
	static std::vector<std::unique_ptr<Texture>> m_Textures;

	static UINT m_SkyTextureIndex;
	static UINT m_DynamicTextureIndex;
	static UINT m_DynamicTextureNum;

public:
	static void LoadTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);

	static std::vector<std::unique_ptr<Texture>>& GetTextures() { return m_Textures; }
	static UINT GetTextureNum() { return (UINT)m_Textures.size(); }
	static UINT GetSkyTextureIndex() { return m_SkyTextureIndex; }
	static UINT GetDynamicTextureIndex() { return m_DynamicTextureIndex; }
	static UINT GetDynamicTextureNum() { return m_DynamicTextureNum; }
};