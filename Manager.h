#pragma once

#include "Common\\GameTimer.h"

struct ID3D12Device;

class CManager
{
public:
	static bool Init();
	static void OnResize();
	static void Uninit();
	static void Update(const GameTimer& gt);
	static void Draw(const GameTimer& gt);

	static ID3D12Device* GetDevice();
};