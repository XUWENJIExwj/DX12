#pragma once

enum class TextureIndex :int
{
	Texture_Logo_00,
	Texture_Bricks_00,
	Texture_Tile_00,
	Texture_Glass_00,
	Texture_Default_00,
	Texture_SkyCube_00,
	Texture_Max
};

class CTextureManager
{
private:
	static std::vector<std::string>              m_TexturesName;
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