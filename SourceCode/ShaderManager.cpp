#include "Renderer.h"
//#include "ShaderManager.h"

using Microsoft::WRL::ComPtr;
using namespace std;

vector<ShaderType> CShaderManager::m_ShaderTypes((int)ShaderTypeIndex::Shader_Type_Max);
vector<D3D12_INPUT_ELEMENT_DESC> CShaderManager::m_InputLayout;

void CShaderManager::ComplieShaders()
{
	LoadInputLayout();

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO shadowDebugDefines[] =
	{
		"SHADOW", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO quadDebugDefines[] =
	{
		"QUAD", "1",
		NULL, NULL
	};

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Opaque].vertexShader = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Opaque].pixelShader = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Opaque_POM].vertexShader = d3dUtil::CompileShader(L"Shaders\\ParallaxOcclusionMapping.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Opaque_POM].pixelShader = d3dUtil::CompileShader(L"Shaders\\ParallaxOcclusionMapping.hlsl", nullptr, "PS", "ps_5_1");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Sky].vertexShader = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Sky].pixelShader = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "PS", "ps_5_1");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMap].vertexShader = d3dUtil::CompileShader(L"Shaders\\ShadowMap.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMap].pixelShader = d3dUtil::CompileShader(L"Shaders\\ShadowMap.hlsl", nullptr, "PS", "ps_5_1");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMapWithAlphaTest].vertexShader = d3dUtil::CompileShader(L"Shaders\\ShadowMap.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMapWithAlphaTest].pixelShader = d3dUtil::CompileShader(L"Shaders\\ShadowMap.hlsl", alphaTestDefines, "PS", "ps_5_1");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMapDebug].vertexShader = d3dUtil::CompileShader(L"Shaders\\QuadDebug.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMapDebug].pixelShader = d3dUtil::CompileShader(L"Shaders\\QuadDebug.hlsl", shadowDebugDefines, "PS", "ps_5_1");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_QuadDebug].vertexShader = d3dUtil::CompileShader(L"Shaders\\QuadDebug.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_QuadDebug].pixelShader = d3dUtil::CompileShader(L"Shaders\\QuadDebug.hlsl", quadDebugDefines, "PS", "ps_5_1");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_RadialBlur].computeShader = d3dUtil::CompileShader(L"Shaders\\RadialBlur.hlsl", nullptr, "RadialBlurCS", "cs_5_0");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_GaussBlurHorizontal].computeShader = d3dUtil::CompileShader(L"Shaders\\GaussBlur.hlsl", nullptr, "HorizontalBlurCS", "cs_5_0");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_GaussBlurVertical].computeShader = d3dUtil::CompileShader(L"Shaders\\GaussBlur.hlsl", nullptr, "VerticalBlurCS", "cs_5_0");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_LuminanceMap].computeShader = d3dUtil::CompileShader(L"Shaders\\LuminanceMap.hlsl", nullptr, "LuminanceMapCS", "cs_5_0");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_BlendOpAdd].computeShader = d3dUtil::CompileShader(L"Shaders\\Blend.hlsl", nullptr, "BlendOpAddCS", "cs_5_0");
}

void CShaderManager::LoadInputLayout()
{
	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 6, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 4 * 9, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	for (int i = 0; i < (int)ShaderTypeIndex::Shader_Type_Max; ++i)
	{
		m_ShaderTypes[i].inputLayout = &m_InputLayout;
	}
}

void CShaderManager::LoadCompiledShaders()
{
	LoadInputLayout();

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Opaque].vertexShader = d3dUtil::LoadBinary(L"CompiledShaders\\DefaultVS.cso");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Opaque].pixelShader = d3dUtil::LoadBinary(L"CompiledShaders\\DefaultPS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Opaque_POM].vertexShader = d3dUtil::LoadBinary(L"CompiledShaders\\ParallaxOcclusionMappingVS.cso");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Opaque_POM].pixelShader = d3dUtil::LoadBinary(L"CompiledShaders\\ParallaxOcclusionMappingPS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Sky].vertexShader = d3dUtil::LoadBinary(L"CompiledShaders\\SkyVS.cso");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_Sky].pixelShader = d3dUtil::LoadBinary(L"CompiledShaders\\SkyPS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMap].vertexShader = d3dUtil::LoadBinary(L"CompiledShaders\\ShadowMapVS.cso");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMap].pixelShader = d3dUtil::LoadBinary(L"CompiledShaders\\ShadowMapPS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMapWithAlphaTest].vertexShader = d3dUtil::LoadBinary(L"CompiledShaders\\ShadowMapWithAlphaTestVS.cso");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMapWithAlphaTest].pixelShader = d3dUtil::LoadBinary(L"CompiledShaders\\ShadowMapWithAlphaTestPS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMapDebug].vertexShader = d3dUtil::LoadBinary(L"CompiledShaders\\ShadowDebugVS.cso");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_ShadowMapDebug].pixelShader = d3dUtil::LoadBinary(L"CompiledShaders\\ShadowDebugPS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_QuadDebug].vertexShader = d3dUtil::LoadBinary(L"CompiledShaders\\QuadDebugVS.cso");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_QuadDebug].pixelShader = d3dUtil::LoadBinary(L"CompiledShaders\\QuadDebugPS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_RadialBlur].computeShader = d3dUtil::LoadBinary(L"CompiledShaders\\RadialBlurCS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_GaussBlurHorizontal].computeShader = d3dUtil::LoadBinary(L"CompiledShaders\\HorizontalBlurCS.cso");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_GaussBlurVertical].computeShader = d3dUtil::LoadBinary(L"CompiledShaders\\VerticalBlurCS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_LuminanceMap].computeShader = d3dUtil::LoadBinary(L"CompiledShaders\\LuminanceMapCS.cso");

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_BlendOpAdd].computeShader = d3dUtil::LoadBinary(L"CompiledShaders\\BlendOpAddCS.cso");
}
