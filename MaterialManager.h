#pragma once

enum class MaterialTexIndex :int
{
	Material_Mirror_00,
	Material_Logo_00,
	Material_Bricks_00,
	Material_Tile_00,
	Material_Plane_00,
	Material_Glass_00,
	Material_Max
};

enum class MaterialCubeMapIndex :int
{
	Material_SkyCube_00,
	Material_SkyCube_01,
	Material_SkyCube_02,
	Material_IndoorCube_00,
	Material_Max
};

class CMaterialManager
{
private:
	static std::vector<const char*> m_MaterialTexNames;
	static std::vector<Material*>   m_MaterialTex;

	static std::vector<const char*> m_MaterialCubeMapNames;
	static std::vector<Material*>   m_MaterialCubeMap;

	static std::vector<std::unique_ptr<Material>> m_AllMaterials;

public:
	static void CreateMaterials();
	static void UpdateMaterial();

	static const char**                            GetMaterialTexNames() { return m_MaterialTexNames.data(); }
	static std::vector<Material*>& GetMaterialTex() { return m_MaterialTex; }
	static Material* GetMaterialTex(int MaterialIndex) { return m_MaterialTex[MaterialIndex]; }
	static int GetMaterialTexCount() { return (int)m_MaterialTex.size(); }

	static const char**                            GetMaterialCubeMapNames() { return m_MaterialCubeMapNames.data(); }
	static std::vector<Material*>& GetMaterialCubeMap() { return m_MaterialCubeMap; }
	static Material* GetMaterialCubeMap(int MaterialIndex) { return m_MaterialCubeMap[MaterialIndex]; }
	static int GetMaterialCubeMapCount() { return (int)m_MaterialCubeMap.size(); }

	static std::vector<std::unique_ptr<Material>>& GetAllMaterials() { return m_AllMaterials; }
	static Material* GetMaterial(int MaterialIndex) { return m_AllMaterials[MaterialIndex].get(); }
	static int GetAllMaterialsCount() { return (int)m_AllMaterials.size(); }
};