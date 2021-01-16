#pragma once

#include "Scene.h"

class CGame :public CScene
{
public:
	CGame() = default;
	~CGame() = default;

	void Init()override;
	void Uninit()override;
	void UpdateAll(const GameTimer& GlobalTimer)override;
	void Draw(const GameTimer& GlobalTimer)override;
};