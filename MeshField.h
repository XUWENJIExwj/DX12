#pragma once

#include "GameObject.h"

class CMeshField :public CGameObject
{
public:
	void Init()override;
	void UpdateImGui(const GameTimer & GlobalTimer)override;
};