#pragma once

enum class MaterialIndex :int
{
	Material_00_Bricks,
	Material_01_Tile,
	Material_02_Mirror,
	Material_03_Sky,
	Material_Max
};

class CMaterialManager
{
private:
	static std::vector<std::string>               m_MaterialsName;
	static std::vector<std::unique_ptr<Material>> m_Materials;

public:
	static void CreateMaterials();

	static Material* GetMaterial(int MaterialIndex) { return m_Materials[MaterialIndex].get(); }
	static int GetMaterialsCount() { return (int)m_Materials.size(); }
};