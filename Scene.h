#pragma once

#include <queue>
#include "Common\\GameTimer.h"
#include "FrameResource.h"

enum class RenderLayers :int
{
	Layer_Camera,
	Layer_Light,
	Layer_3D_Opaque,
	Layer_3D_Sky,
	Layer_3D_BillBoard,
	Layer_3D_Opaque_DynamicReflectors,
	Layer_2D_Opaque,
	Layer_Fade,
	Layer_Max
};

class CGameObject;
class CCamera;

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
	PassConstants m_MainPassCB;

	std::vector<CCamera*> m_DCMCameras;

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
	virtual void DrawImGui(const GameTimer& GlobalTimer);

	virtual void UpdateGameObjectsCB(const GameTimer& GlobalTimer);
	virtual void UpdateMaterialBuffer(const GameTimer& GlobalTimer);
	virtual void UpdateMainPassCB(const GameTimer& GlobalTimer);
	virtual void UpdateDynamicCubeMapPassCB(const GameTimer& GlobalTimer, int DCMResourcesIndex);

	void SetUpDynamicCubeMapCamera(DirectX::XMFLOAT3 Center);
	void CheckNecessaryCBBufferSize();

	template<typename T>
	T* AddGameObject(int Layer, std::string Name = "")
	{
		T* gameObject = new T;

		m_AllRenderLayers[Layer].push_back(gameObject);
		gameObject->SetRenderLayer(Layer);
		gameObject->SetName(Name);
		gameObject->Init();

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
			// Œ^‚ð’²‚×‚é
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
		std::vector<T*> gameObjects; // STL‚Ì”z—ñ
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
	std::vector<CCamera*>& GetDynamicCubeMapCameras() { return m_DCMCameras; }
};