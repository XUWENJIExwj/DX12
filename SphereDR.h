#pragma once

#include "Sphere.h"

class CSphereDR :public CSphere
{
public:
	void Init()override;
	void Update(const GameTimer& GlobalTimer)override;

	void GlobalRotate(const GameTimer& GlobalTimer);
	void DrawDynamicCubeMapScene(std::list<CGameObject*>* AllGameObjectsWithLayer)override;
};