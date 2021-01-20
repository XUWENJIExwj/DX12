#pragma once

#include <queue>
#include "Common\\GameTimer.h"
#include "FrameResource.h"

enum class RenderLayers :int
{
	Layer_Camera,
	Layer_DirLight,
	Layer_3D_Opaque,
	Layer_3D_Opaque_POM,
	Layer_3D_Sky,
	Layer_3D_BillBoard,
	Layer_3D_Opaque_DynamicReflectors,
	Layer_2D_Opaque,
	Layer_2D_Debug,
	Layer_Fade,
	Layer_Max
};

class CGameObject;
class CCamera;
class CDirLight;

struct AllGameObjectsListEmptyReference
{
	std::queue<std::list<CGameObject*>::iterator> objIteratorStore;
	std::queue<UINT> objCBIndexStore;
};

class CScene
{
protected:
	std::list<CGameObject*> m_AllGameObjects;
	std::list<CGameObject*> m_AllRenderLayers[(int)RenderLayers::Layer_Max];
	AllGameObjectsListEmptyReference m_ListEmptyReference;

	UINT          m_ObjectCBCounts = 0;
	CCamera*      m_MainCamera;
	CDirLight*    m_MainDirLight;
	PassConstants m_MainPassCB;
	PassConstants m_ShadowPassCB;

	std::vector<CCamera*>   m_DCMCameras;
	std::vector<CDirLight*> m_DirLights;

	DirectX::BoundingSphere m_SceneBounds;
	DirectX::XMFLOAT4X4     m_ShadowTransform = MathHelper::Identity4x4();

	int m_BeginPSOIndex; // SceneÇÃInitÇ≈ÇÃê›íËÇñYÇÍÇ∏Ç…

public:
	CScene() = default;
	virtual ~CScene() = default;

	virtual void Init() = 0;
	virtual void Uninit();
	virtual void UpdateAll(const GameTimer& GlobalTimer);
	virtual void FixedUpdate(const GameTimer& GlobalTimer);
	virtual void Update(const GameTimer& GlobalTimer);
	virtual void LateUpdate(const GameTimer& GlobalTimer);
	virtual void Draw(const GameTimer& GlobalTimer) = 0;

	virtual void UpdateImGui(const GameTimer& GlobalTimer);
	virtual void UpdateGameObjectsCB(const GameTimer& GlobalTimer);
	virtual void UpdateMaterialBuffer(const GameTimer& GlobalTimer);
	virtual void UpdateMainPassCB(const GameTimer& GlobalTimer);
	virtual void UpdateShadowPassCB(const GameTimer& GlobalTimer);
	virtual void UpdateShadowPassCB(const GameTimer& GlobalTimer, int CascadeIndex);
	virtual void UpdateDynamicCubeMapPassCB(const GameTimer& GlobalTimer, int DCMResourcesIndex);

	virtual void OnResize() {}
	void OnResizeLayer(int Layer);

	void SetUpDynamicCubeMapCamera(DirectX::XMFLOAT3 Center);
	void CheckNecessaryCBBufferSize();

	void SetSceneBounds(DirectX::BoundingBox* Bounds);
	void SetSceneBounds(DirectX::BoundingSphere* Bounds) { m_SceneBounds = *Bounds; }
	void SetSceneBounds(float Width, float Height, DirectX::XMFLOAT3 Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

	template<typename T>
	T* AddGameObject(int Layer, std::string Name = "")
	{
		T* gameObject = new T;

		m_AllRenderLayers[Layer].push_back(gameObject);
		gameObject->SetRenderLayer(Layer);
		gameObject->SetName(Name);
		gameObject->Init();

		if (Layer == (int)RenderLayers::Layer_DirLight)
		{
			m_DirLights.push_back((CDirLight*)gameObject);
		}

		if (m_ListEmptyReference.objIteratorStore.size() > 0)
		{
			gameObject->SetObjCBIndex(m_ListEmptyReference.objCBIndexStore.front());
			m_ListEmptyReference.objCBIndexStore.pop();
			*m_ListEmptyReference.objIteratorStore.front() = gameObject;
			m_ListEmptyReference.objIteratorStore.pop();
		}
		else
		{
			gameObject->SetObjCBIndex((UINT)m_AllGameObjects.size());
			m_AllGameObjects.push_back(gameObject);
		}
		//gameObject->SetImGuiInfo();

		return gameObject;
	}

	template<typename T>
	T* GetGameObject(int Layer)
	{
		for (CGameObject* gameObject : m_AllRenderLayers[Layer])
		{
			// å^Çí≤Ç◊ÇÈ
			if (typeid(*gameObject) == typeid(T))
			{
				return (T*)gameObject;
			}
		}

		return nullptr;
	}

	template <typename T>
	std::vector<T*>& GetGameObjects(int Layer)
	{
		std::vector<T*> gameObjects; // STLÇÃîzóÒ
		for (CGameObject* gameObject : m_AllRenderLayers[Layer])
		{
			if (typeid(*gameObject) == typeid(T))
			{
				gameObjects.push_back((T*)gameObject);
			}
		}

		return gameObjects;
	}

	std::list<CGameObject*>& GetAllGameObjects() { return m_AllGameObjects; }
	std::list<CGameObject*>& GetRenderLayer(int Layer) { return m_AllRenderLayers[Layer]; }
	std::list<CGameObject*>* GetAllRenderLayers() { return m_AllRenderLayers; }
	AllGameObjectsListEmptyReference* GetListEmptyReference() { return &m_ListEmptyReference; }
	UINT GetAllGameObjectsCount() { return (UINT)m_AllGameObjects.size(); }
	DirectX::BoundingSphere* GetSceneBounds() { return &m_SceneBounds; }
	std::vector<CCamera*>& GetDynamicCubeMapCameras() { return m_DCMCameras; }
	int GetBeginPSOIndex() { return m_BeginPSOIndex; }
};