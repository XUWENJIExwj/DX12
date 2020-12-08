#pragma once

#include "Common\\GameTimer.h"

class CManager
{
public:
	static bool Init();
	static void Uninit();
	static void Update(const GameTimer& gt);
	static void Draw(const GameTimer& gt);
};