#pragma once

#include "GameObject.h"

class CQuad :public CGameObject
{
public:
	void Init()override;
	void Update(const GameTimer& GlobalTimer)override;

	void OnResize()override;
};