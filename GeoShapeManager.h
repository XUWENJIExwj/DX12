#pragma once

enum class GeoShapeType :int
{
	Type_GeoShapes,
	Type_Max
};

class CGeoShapeManager
{
private:
	static std::vector<std::unique_ptr<MeshGeometry>> m_Geometries;

public:
	static void CreateGeoShapes();
	static MeshGeometry* GetMeshGeometry(int GeoType) { return m_Geometries[GeoType].get(); }
};