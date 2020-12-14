#include "Renderer.h"
//#include "ShaderManager.h"

using Microsoft::WRL::ComPtr;
using namespace std;

vector<ShaderType> CShaderManager::m_ShaderTypes((int)ShaderTypeIndex::Shader_Type_MAX);
vector<D3D12_INPUT_ELEMENT_DESC> CShaderManager::m_InputLayout;

void CShaderManager::LoadShaders()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 6, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 4 * 9, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].vertexShader = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].pixelShader = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_00_Opaque].inputLayout = &m_InputLayout;

	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_01_Sky].vertexShader = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "VS", "vs_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_01_Sky].pixelShader = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "PS", "ps_5_1");
	m_ShaderTypes[(int)ShaderTypeIndex::Shader_Type_01_Sky].inputLayout = &m_InputLayout;
}