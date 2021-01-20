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
	static DirectX::BoundingBox         GetBoxBounds(std::string Name) { return m_BoxBounds[Name]; }
	static DirectX::BoundingOrientedBox GetOrientedBoxBounds(std::string Name) { return m_OrientedBoxBounds[Name]; }
	static DirectX::BoundingSphere      GetSphereBounds(std::string Name) { return m_SphereBounds[Name]; }
	static DirectX::BoundingFrustum     GetFrustumBounds(std::string Name) { return m_FrustumBounds[Name]; }
};