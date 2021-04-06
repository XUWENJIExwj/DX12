#pragma once

#include "GameObject.h"

class CCube :public CGameObject
{
public:
	void Init()override;
	void Update(const GameTimer& GlobalTimer)override;
	void LateUpdate(const GameTimer& GlobalTimer)override;
	void UpdateImGui(const GameTimer& GlobalTimer)override;
};