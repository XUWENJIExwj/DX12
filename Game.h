#pragma once

#include "Scene.h"

class CGame :public CScene
{
public:
	CGame() = default;
	~CGame() = default;

	void Init()override;
	void Uninit()override;
	void Update(const GameTimer& GlobalTimer)override;
};