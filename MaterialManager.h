#pragma once

enum class MaterialNormalIndex :int
{
	Material_Mirror_Dynamic_00, // NormalMap(With No Height)
	Material_Mirror_00,
	Material_Logo_00,
	Material_Bricks_00,
	Material_Tile_00,
	Material_Glass_00,
	Material_Plane_00, // NormalMap(With Height)
	Material_Max
};

enum class MaterialHeightIndex :int
{
	Material_Plane_00,
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
	static std::vector<const char*> m_MaterialNormalNames;
	static std::vector<Material*>   m_MaterialNormal;

	static std::vector<const char*> m_MaterialHeightNames;
	static std::vector<Material*>   m_MaterialHeight;

	static std::vector<const char*> m_MaterialCubeMapNames;
	static std::vector<Material*>   m_MaterialCubeMap;

	static std::vector<std::unique_ptr<Material>> m_AllMaterials;

public:
	static void CreateMaterialNormal();
	static void CreateMaterialHeight();
	static void CreateMaterialCubeMap();
	static void CreateMaterials();
	static void UpdateMaterial();

	static const char**            GetMaterialNormalNames() { return m_MaterialNormalNames.data(); }
	static std::vector<Material*>& GetMaterialNormal() { return m_MaterialNormal; }
	static Material*               GetMaterialNormal(int MaterialIndex) { return m_MaterialNormal[MaterialIndex]; }
	static int                     GetMaterialNormalCount() { return (int)m_MaterialNormal.size(); }

	static const char**            GetMaterialHeightNames() { return m_MaterialHeightNames.data(); }
	static std::vector<Material*>& GetMaterialHeight() { return m_MaterialHeight; }
	static Material*               GetMaterialHeight(int MaterialIndex) { return m_MaterialHeight[MaterialIndex]; }
	static int                     GetMaterialHeightCount() { return (int)m_MaterialHeight.size(); }

	static const char**            GetMaterialCubeMapNames() { return m_MaterialCubeMapNames.data(); }
	static std::vector<Material*>& GetMaterialCubeMap() { return m_MaterialCubeMap; }
	static Material*               GetMaterialCubeMap(int MaterialIndex) { return m_MaterialCubeMap[MaterialIndex]; }
	static int                     GetMaterialCubeMapCount() { return (int)m_MaterialCubeMap.size(); }

	static std::vector<std::unique_ptr<Material>>& GetAllMaterials() { return m_AllMaterials; }
	static Material* GetMaterial(int MaterialIndex) { return m_AllMaterials[MaterialIndex].get(); }
	static int       GetAllMaterialsCount() { return (int)m_AllMaterials.size(); }
};