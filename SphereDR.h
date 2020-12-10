#pragma once

#include "Sphere.h"

class CSphereDR :public CSphere
{
public:
	void Init()override;
	void Update(const GameTimer& GlobalTimer)override;
	void Draw(const GameTimer& GlobalTimer, int DCMResourcesIndex = 0)override;

	void GlobalRotate(const GameTimer& GlobalTimer);

	void CreateDynamicCubeMapResources(const GameTimer& GlobalTimer, int DCMResourcesIndex = 0)override;
};