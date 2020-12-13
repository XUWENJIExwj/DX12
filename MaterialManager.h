#pragma once

enum class MaterialIndex :int
{
	Material_Logo_00,
	Material_Bricks_00,
	Material_Tile_00,
	Material_Plane_00,
	Material_Glass_00,
	Material_Mirror_00,
	Material_SkyCube_00,
	Material_Max
};

class CMaterialManager
{
private:
	static std::vector<const char*>               m_MaterialsName;
	static std::vector<std::unique_ptr<Material>> m_Materials;

public:
	static void CreateMaterials();
	static void UpdateMaterial();

	static std::vector<std::unique_ptr<Material>>& GetMaterials() { return m_Materials; }
	static Material* GetMaterial(int MaterialIndex) { return m_Materials[MaterialIndex].get(); }
	static int GetMaterialsCount() { return (int)m_Materials.size(); }
};