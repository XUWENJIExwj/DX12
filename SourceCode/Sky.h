#pragma once

#include "GameObject.h"

class CSky :public CGameObject
{
public:
	void Init()override;
	void UpdateImGui(const GameTimer & GlobalTimer)override;
};