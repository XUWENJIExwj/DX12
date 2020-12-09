#pragma once

#include <queue>
#include "Common\\GameTimer.h"
#include "FrameResource.h"

enum class GameObjectsLayer :int
{
	Layer_Camera,
	Layer_Light,
	Layer_Opaque_3DOBJ,
	Layer_Opaque_DynamicReflectors_3DOBJ,
	Layer_Sky,
	Layer_BillBoard,
	Layer_Opaque_2DOBJ,
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
	std::list<CGameObject*> m_GameObjectsLayer[(int)GameObjectsLayer::Layer_Max];
	AllGameObjectsListEmptyReference m_ListEmptyReference;

	UINT          m_ObjectCBCounts = 0;
	CCamera*      m_MainCamera;
	PassConstants m_MainPassCB;

public:
	CScene() = default;
	virtual ~CScene() = default;

	virtual void Init() = 0;
	virtual void Uninit();
	virtual void Update(const GameTimer& GlobalTimer);
	virtual void LateUpdate(const GameTimer& GlobalTimer);
	virtual void UpdateGameObjectsCB(const GameTimer& GlobalTimer);
	virtual void UpdateMaterialBuffer(const GameTimer& GlobalTimer);
	virtual void UpdateMainPassCB(const GameTimer& GlobalTimer);
	virtual void Draw();

	void CheckNecessaryCBBufferSize();

	template<typename T>
	T* AddGameObject(int Layer)
	{
		T* gameObject = new T;

		m_GameObjectsLayer[Layer].push_back(gameObject);
		gameObject->SetGameObjectLayer(Layer);
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
		for (CGameObject* gameObject : m_GameObjectsLayer[Layer])
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
		for (CGameObject* gameObject : m_GameObjectsLayer[Layer])
		{
			if (typeid(*gameObject) == typeid(T))
			{
				gameObjects.push_back((T*)gameObject);
			}
		}

		return gameObjects;
	}

	std::list<CGameObject*>& GetAllGameObjects() { return m_AllGameObjects; }
	std::list<CGameObject*>& GetGameObjectsWithLayer(int Layer) { return m_GameObjectsLayer[Layer]; }
	AllGameObjectsListEmptyReference* GetListEmptyReference() { return &m_ListEmptyReference; }
	UINT GetAllGameObjectsCount() { return (UINT)m_AllGameObjects.size(); }
};