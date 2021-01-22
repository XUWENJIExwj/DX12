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

	static std::unordered_map<std::string, DirectX::BoundingBox>         m_BoxBounds;
	static std::unordered_map<std::string, DirectX::BoundingOrientedBox> m_OrientedBoxBounds;
	static std::unordered_map<std::string, DirectX::BoundingSphere>      m_SphereBounds;
	static std::unordered_map<std::string, DirectX::BoundingFrustum>     m_FrustumBounds;

public:
	static void CreateGeoShapes();
	static MeshGeometry* GetMeshGeometry(int GeoType) { return m_Geometries[GeoType].get(); }
	static DirectX::BoundingBox         GetBoxBounds(std::string SubMeshName) { return m_BoxBounds[SubMeshName]; }
	static DirectX::BoundingOrientedBox GetOrientedBoxBounds(std::string SubMeshName) { return m_OrientedBoxBounds[SubMeshName]; }
	static DirectX::BoundingSphere      GetSphereBounds(std::string SubMeshName) { return m_SphereBounds[SubMeshName]; }
	static DirectX::BoundingFrustum     GetFrustumBounds(std::string SubMeshName) { return m_FrustumBounds[SubMeshName]; }
};