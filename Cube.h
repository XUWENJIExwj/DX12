#pragma once

#include "GameObject.h"

class CCube :public CGameObject
{
public:
	void Init()override;
	void Update(const GameTimer& GlobalTimer)override;
};