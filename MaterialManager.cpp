#include "Renderer.h"
//#include "MaterialManager.h"

using namespace std;
using namespace DirectX;

vector<string>               CMaterialManager::m_MaterialsName;
vector<unique_ptr<Material>> CMaterialManager::m_Materials((int)MaterialIndex::Material_Max);

void CMaterialManager::CreateMaterials()
{
	auto logo = std::make_unique<Material>();
	logo->Name = "logo";
	logo->MatCBIndex = 0;
	logo->DiffuseSrvHeapIndex = 0;
	logo->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	logo->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	logo->Roughness = 1.0f;

	auto bricks = std::make_unique<Material>();
	bricks->Name = "bricks";
	bricks->MatCBIndex = 1;
	bricks->DiffuseSrvHeapIndex = 1;
	bricks->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks->Roughness = 0.3f;

	auto tile = std::make_unique<Material>();
	tile->Name = "tile";
	tile->MatCBIndex = 2;
	tile->DiffuseSrvHeapIndex = 2;
	tile->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	tile->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	tile->Roughness = 0.1f;

	auto glass = std::make_unique<Material>();
	glass->Name = "glass";
	glass->MatCBIndex = 3;
	glass->DiffuseSrvHeapIndex = 3;
	glass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	glass->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	glass->Roughness = 1.0f;

	auto mirror = std::make_unique<Material>();
	mirror->Name = "mirror";
	mirror->MatCBIndex = 4;
	mirror->DiffuseSrvHeapIndex = 4;
	mirror->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mirror->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	mirror->Roughness = 0.1f;

	auto skycube = std::make_unique<Material>();
	skycube->Name = "skycube";
	skycube->MatCBIndex = 5;
	skycube->DiffuseSrvHeapIndex = 5;
	skycube->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skycube->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	skycube->Roughness = 1.0f;

	m_Materials[(int)MaterialIndex::Material_Logo_00] = std::move(logo);
	m_Materials[(int)MaterialIndex::Material_Bricks_00] = std::move(bricks);
	m_Materials[(int)MaterialIndex::Material_Tile_00] = std::move(tile);
	m_Materials[(int)MaterialIndex::Material_Glass_00] = std::move(glass);
	m_Materials[(int)MaterialIndex::Material_Mirror_00] = std::move(mirror);
	m_Materials[(int)MaterialIndex::Material_SkyCube_00] = std::move(skycube);
}