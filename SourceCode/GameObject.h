#pragma once

#include "DX12App.h"
#include "ImGuiManager.h"
#include "CommonResource.h"

class CGameObject
{
protected:
	int         m_RenderLayer;
	std::string m_Name;
	bool        m_Destroy = false;

	DirectX::XMFLOAT3 m_Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 m_OldPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 m_Rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT4 m_Quaternion;
	DirectX::XMFLOAT3 m_Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT2 m_ScaleWindowXYRatio = DirectX::XMFLOAT2(1.0f, 1.0f);
	float             m_ScaleXYRatio = 1.0f;

	DirectX::XMFLOAT4X4 m_World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_TexTransform = MathHelper::Identity4x4();

	int  m_NumFramesDirty = gNumFrameResources;
	UINT m_ObjCBIndex = -1;

	Material*     m_Material = nullptr;
	MeshGeometry* m_MeshGeometry = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT m_IndexCount = 0;
	UINT m_StartIndexLocation = 0;
	int  m_BaseVertexLocation = 0;

	std::string          m_BoundsName;
	DirectX::BoundingBox m_BoundingBox;

	GameObjectImGuiCB m_ImGuiCB;

public:
	CGameObject();
	virtual ~CGameObject() = default;

	virtual void Init() = 0;
	virtual void Uninit() {}
	virtual void Update(const GameTimer& GlobalTimer) {}
	virtual void FixedUpdate(const GameTimer& GlobalTimer) {}
	virtual void LateUpdate(const GameTimer& GlobalTimer) {}
	virtual void Draw(const GameTimer& GlobalTimer, int DCMResourcesIndex) {}

	virtual void OnResize();
	void Init2DScaleRatio();

	// DynamicCubeMap�`��p
	void CreateDynamicCubeMapResources(const GameTimer& GlobalTimer, int DCMResourcesIndex);
	virtual void DrawDynamicCubeMapScene(std::list<CGameObject*>* AllRenderLayers) {}

	// ImGui�p
	virtual void UpdateImGui(const GameTimer& GlobalTimer) {}

	void SetRenderLayer(int RenderLayer) { m_RenderLayer = RenderLayer; }
	void SetName(std::string Name) { m_Name = Name; }
	void SetPosition(DirectX::XMFLOAT3 Position) { m_Position = m_OldPosition = Position; }
	void SetPositionX(float PosX) { m_Position.x = m_OldPosition.x = PosX; }
	void SetRotation(DirectX::XMFLOAT3 Rotation) { m_Rotation = Rotation; }
	void SetScale(DirectX::XMFLOAT3 Scale) { m_Scale = Scale; }
	void SetWorldMatrix() { m_World = ComputeWorldMatrix4x4(); }
	void Set2DWVPMatrix() { m_World = Compute2DWVPMatrix4x4(); }
	void SetDestroy() { m_Destroy = true; }
	void SetNumFramesDirty(int NumFramesDirty) { m_NumFramesDirty = NumFramesDirty; }
	void SetObjCBIndex(UINT ObjCBIndex) { m_ObjCBIndex = ObjCBIndex; }
	void SetMaterialNormal(int MaterialIndex) { m_Material = CMaterialManager::GetMaterialNormal(MaterialIndex); }

	void UpdateBoundingBox();
	void ComputeBoundingBox();

	int GetGameObjectLayer() { return m_RenderLayer; }

	DirectX::XMFLOAT3   GetPosition3f()const { return m_Position; }
	DirectX::XMFLOAT3   GetRotation3f()const { return m_Rotation; }
	DirectX::XMFLOAT4   GetQuaternion4f()const { return m_Quaternion; }
	DirectX::XMFLOAT3   GetScale3f()const { return m_Scale; }
	DirectX::XMFLOAT3   GetRightWithRotation3f()const;
	DirectX::XMFLOAT3   GetUpWithRotation3f()const;
	DirectX::XMFLOAT3   GetForwardWithRotation3f()const;
	DirectX::XMFLOAT3   GetRightWithQuaternion3f()const;
	DirectX::XMFLOAT3   GetUpWithQuaternion3f()const;
	DirectX::XMFLOAT3   GetForwardWithQuaternion3f()const;
	DirectX::XMFLOAT4X4 GetWorldMatrix4x4()const { return m_World; }
	DirectX::XMFLOAT4X4 GetTexTransformMatrix4x4()const { return m_TexTransform; }
	DirectX::XMFLOAT4X4 ComputeWorldMatrix4x4()const;
	DirectX::XMFLOAT4X4 Compute2DWVPMatrix4x4()const;

	DirectX::XMVECTOR XM_CALLCONV GetPosition()const { return XMLoadFloat3(&m_Position); }
	DirectX::XMVECTOR XM_CALLCONV GetRotation()const { return XMLoadFloat3(&m_Rotation); }
	DirectX::XMVECTOR XM_CALLCONV GetQuaternion()const { return XMLoadFloat4(&m_Quaternion); }
	DirectX::XMVECTOR XM_CALLCONV GetScale()const { return XMLoadFloat3(&m_Scale); }
	DirectX::XMVECTOR XM_CALLCONV GetRightWithRotation()const;
	DirectX::XMVECTOR XM_CALLCONV GetUpWithRotation()const;
	DirectX::XMVECTOR XM_CALLCONV GetForwardWithRotation()const;
	DirectX::XMVECTOR XM_CALLCONV GetRightWithQuaternion()const;
	DirectX::XMVECTOR XM_CALLCONV GetUpWithQuaternion()const;
	DirectX::XMVECTOR XM_CALLCONV GetForwardWithQuaternion()const;
	DirectX::XMMATRIX XM_CALLCONV GetWorldMatrix()const { return XMLoadFloat4x4(&m_World); }
	DirectX::XMMATRIX XM_CALLCONV GetTexTransformMatrix()const { return XMLoadFloat4x4(&m_TexTransform); }
	DirectX::XMMATRIX XM_CALLCONV ComputeWorldMatrix()const;
	DirectX::XMMATRIX XM_CALLCONV Compute2DWVPMatrix()const;

	DirectX::BoundingBox* GetBoundingBox() { return &m_BoundingBox; }

	int  GetNumFramesDirty() { return m_NumFramesDirty; }
	UINT GetObjCBIndex() { return m_ObjCBIndex; }

	Material*     GetMaterial() { return m_Material; }
	MeshGeometry* GetMeshGeometry() { return m_MeshGeometry; }

	D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() { return m_PrimitiveType; }

	UINT GetIndexCount() { return m_IndexCount; }
	UINT GetStartIndexLocation() { return m_StartIndexLocation; }
	int  GetBaseVertexLocation() { return m_BaseVertexLocation; }

	bool Destroy();
};