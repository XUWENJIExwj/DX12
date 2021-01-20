#pragma once

enum class VertexShaderIndex :int
{
	VS_Opaque,
	VS_Opaque_POM,
	VS_Sky,
	VS_LiSPSM,
	VS_LiSPSMWithAlphaTest,
	VS_LiSPSMMapDebug,
	VS_Max
};

enum class PixelShaderIndex :int
{
	PS_Opaque,
	PS_Opaque_POM,
	PS_Sky,
	PS_LiSPSM,
	PS_LiSPSMWithAlphaTest,
	PS_LiSPSMDebug,
	PS_Max
};

enum class ShaderTypeIndex :int
{
	Shader_Type_Opaque,
	Shader_Type_Opaque_POM,
	Shader_Type_Sky,
	Shader_Type_LiSPSM,
	Shader_Type_LiSPSMWithAlphaTest,
	Shader_Type_LiSPSMDebug,
	Shader_Type_Max
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
	static std::vector<ShaderType>               m_ShaderTypes;
	static std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

public:
	static void LoadShaders();
	static std::vector<ShaderType>& GetShaderTypes() { return m_ShaderTypes; }
};