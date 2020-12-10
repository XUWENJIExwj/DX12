#pragma once

enum class TextureIndex :int
{
	Texture_00_Bricks,
	Texture_01_Tile,
	Texture_02_Default,
	Texture_03_SkyCube,
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
	static UINT GetSkyTextureIndex() { return m_SkyTextureIndex; }
	static UINT GetDynamicTextureIndex() { return m_DynamicTextureIndex; }
	static UINT GetDynamicTextureNum() { return m_DynamicTextureNum; }
};