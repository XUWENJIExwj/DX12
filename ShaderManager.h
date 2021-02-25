#pragma once

enum class ShaderTypeIndex :int
{
	Shader_Type_Opaque,
	Shader_Type_Opaque_POM,
	Shader_Type_Sky,
	Shader_Type_ShadowMap,
	Shader_Type_ShadowMapWithAlphaTest,
	Shader_Type_ShadowMapDebug,
	Shader_Type_QuadDebug,
	Shader_Type_RadialBlur,
	Shader_Type_GaussBlurHorizontal,
	Shader_Type_GaussBlurVertical,
	Shader_Type_LuminanceMap,
	Shader_Type_BlendOpAdd,
	Shader_Type_Max
};

struct ShaderType
{
	Microsoft::WRL::ComPtr<ID3DBlob>       vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob>       pixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob>       computeShader;
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