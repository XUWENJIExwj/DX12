#include "Renderer.h"
//#include "MaterialManager.h"

using namespace std;
using namespace DirectX;

vector<string>               CMaterialManager::m_MaterialsName;
vector<unique_ptr<Material>> CMaterialManager::m_Materials((int)MaterialIndex::Material_Max);

void CMaterialManager::CreateMaterials()
{
	auto bricks = std::make_unique<Material>();
	bricks->Name = "bricks";
	bricks->MatCBIndex = 0;
	bricks->DiffuseSrvHeapIndex = 0;
	bricks->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks->Roughness = 0.3f;

	auto tile = std::make_unique<Material>();
	tile->Name = "tile";
	tile->MatCBIndex = 1;
	tile->DiffuseSrvHeapIndex = 1;
	tile->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	tile->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	tile->Roughness = 0.1f;

	auto mirror = std::make_unique<Material>();
	mirror->Name = "mirror";
	mirror->MatCBIndex = 2;
	mirror->DiffuseSrvHeapIndex = 2;
	mirror->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mirror->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	mirror->Roughness = 0.1f;

	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->MatCBIndex = 3;
	sky->DiffuseSrvHeapIndex = 3;
	sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	sky->Roughness = 1.0f;

	m_Materials[(int)MaterialIndex::Material_00_Bricks] = std::move(bricks);
	m_Materials[(int)MaterialIndex::Material_01_Tile] = std::move(tile);
	m_Materials[(int)MaterialIndex::Material_02_Mirror] = std::move(mirror);
	m_Materials[(int)MaterialIndex::Material_03_Sky] = std::move(sky);
}