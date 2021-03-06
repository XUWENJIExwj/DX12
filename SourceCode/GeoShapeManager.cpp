#include "..\\Common\\GeometryGenerator.h"
#include "Renderer.h"

using namespace std;
using namespace DirectX;

vector<unique_ptr<MeshGeometry>> CGeoShapeManager::m_Geometries;

unordered_map<string, BoundingBox>         CGeoShapeManager::m_BoxBounds;
unordered_map<string, BoundingOrientedBox> CGeoShapeManager::m_OrientedBoxBounds;
unordered_map<string, BoundingSphere>      CGeoShapeManager::m_SphereBounds;
unordered_map<string, BoundingFrustum>     CGeoShapeManager::m_FrustumBounds;

void CGeoShapeManager::CreateGeoShapes()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData cube = geoGen.CreateCube(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	GeometryGenerator::MeshData quad = geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT cubeVertexOffset = 0;
	UINT gridVertexOffset = (UINT)cube.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
	UINT quadVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT cubeIndexOffset = 0;
	UINT gridIndexOffset = (UINT)cube.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
	UINT quadIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();

	SubmeshGeometry cubeSubmesh;
	cubeSubmesh.IndexCount = (UINT)cube.Indices32.size();
	cubeSubmesh.StartIndexLocation = cubeIndexOffset;
	cubeSubmesh.BaseVertexLocation = cubeVertexOffset;
	BoundingBox::CreateFromPoints(m_BoxBounds["cube"], cube.Vertices.size(), &cube.Vertices[0].Position, sizeof(GeometryGenerator::Vertex));

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;
	BoundingBox::CreateFromPoints(m_BoxBounds["grid"], grid.Vertices.size(), &grid.Vertices[0].Position, sizeof(GeometryGenerator::Vertex));

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;
	BoundingSphere::CreateFromPoints(m_SphereBounds["sphere"], sphere.Vertices.size(), &sphere.Vertices[0].Position, sizeof(GeometryGenerator::Vertex));
	BoundingBox::CreateFromSphere(m_BoxBounds["sphere"], m_SphereBounds["sphere"]);

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;
	BoundingBox::CreateFromPoints(m_BoxBounds["cylinder"], cylinder.Vertices.size(), &cylinder.Vertices[0].Position, sizeof(GeometryGenerator::Vertex));

	SubmeshGeometry quadSubmesh;
	quadSubmesh.IndexCount = (UINT)quad.Indices32.size();
	quadSubmesh.StartIndexLocation = quadIndexOffset;
	quadSubmesh.BaseVertexLocation = quadVertexOffset;
	BoundingBox::CreateFromPoints(m_BoxBounds["quad"], quad.Vertices.size(), &quad.Vertices[0].Position, sizeof(GeometryGenerator::Vertex));

	////
	//// Extract the vertex elements we are interested in and pack the
	//// vertices of all the meshes into one vertex buffer.
	////

	auto totalVertexCount =
		cube.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size() +
		quad.Vertices.size();

	vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < cube.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cube.Vertices[i].Position;
		vertices[k].Normal = cube.Vertices[i].Normal;
		vertices[k].Tangent = cube.Vertices[i].TangentU;
		vertices[k].TexC = cube.Vertices[i].TexC;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tangent = grid.Vertices[i].TangentU;
		vertices[k].TexC = grid.Vertices[i].TexC;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tangent = sphere.Vertices[i].TangentU;
		vertices[k].TexC = sphere.Vertices[i].TexC;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tangent = cylinder.Vertices[i].TangentU;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
	}

	for (size_t i = 0; i < quad.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = quad.Vertices[i].Position;
		vertices[k].Normal = quad.Vertices[i].Normal;
		vertices[k].TexC = quad.Vertices[i].TexC;
		vertices[k].Tangent = quad.Vertices[i].TangentU;
	}

	vector<uint16_t> indices;
	indices.insert(indices.end(), begin(cube.GetIndices16()), end(cube.GetIndices16()));
	indices.insert(indices.end(), begin(grid.GetIndices16()), end(grid.GetIndices16()));
	indices.insert(indices.end(), begin(sphere.GetIndices16()), end(sphere.GetIndices16()));
	indices.insert(indices.end(), begin(cylinder.GetIndices16()), end(cylinder.GetIndices16()));
	indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(uint16_t);

	auto geo = make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(CRenderer::GetDevice(),
		CRenderer::GetCommandList(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(CRenderer::GetDevice(),
		CRenderer::GetCommandList(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["cube"] = cubeSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
	geo->DrawArgs["quad"] = quadSubmesh;

	m_Geometries.push_back(move(geo));
}