#pragma once

enum class GeoShapeType :int
{
	Type_00_GeoShapes,
	Type_Max
};

class CGeoShapeManager
{
private:
	static std::vector<std::unique_ptr<MeshGeometry>> m_Geometries;

public:
	static void CreateGeoShapes(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	static MeshGeometry* GetMeshGeometry(int GeoType) { return m_Geometries[GeoType].get(); }
};