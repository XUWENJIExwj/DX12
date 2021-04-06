#pragma once

#include "..\\Common\\GameTimer.h"

struct ID3D12Device;

class CScene;
class CCamera;

class CManager
{
private:
	static CScene*  m_Scene;
	static CCamera* m_MainCamera;

public:
	static bool Init();
	static void OnResize();
	static void Uninit();
	static void Update(const GameTimer& GlobalTimer);
	static void Draw(const GameTimer& GlobalTimer);
	static void EndTheApp();

	template <typename T>
	static void SetScene()
	{
		if (m_Scene)
		{
			m_Scene->Uninit();
			delete m_Scene;
		}

		T* scene = new T;
		m_Scene = scene;
		scene->Init();
	}

	static void SetMainCamera(CCamera* Camera = nullptr) { m_MainCamera = Camera; }

	static ID3D12Device* GetDevice();
	static CScene*       GetScene() { return m_Scene; }
	static CCamera*      GetMainCamera() { return m_MainCamera; }
};