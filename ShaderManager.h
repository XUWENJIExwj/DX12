#pragma once

enum class VertexShaderIndex :int
{
	VS_00_Opaque,
	VS_01_Sky,
	VS_MAX
};

enum class PixelShaderIndex :int
{
	PS_00_Opaque,
	PS_01_Sky,
	PS_MAX
};

enum class ShaderTypeIndex :int
{
	Shader_Type_00_Opaque,
	Shader_Type_01_Sky,
	Shader_Type_MAX
};

struct ShaderType
{
	Microsoft::WRL::ComPtr<ID3DBlob>       vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob>       pixelShader;
	std::vector<D3D12_INPUT_ELEMENT_DESC>* inputLayout;
};

class CShaderManager
{
private:
	static std::vector<ShaderType> m_ShaderTypes;
	static std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

public:
	static void LoadShaders();
	static std::vector<ShaderType>& GetShaderTypes() { return m_ShaderTypes; }
};