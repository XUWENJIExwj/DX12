#pragma once

#include "ImGui\\imgui.h"
#include "Common\\GameTimer.h"

class CImGuiManager
{
public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();
};