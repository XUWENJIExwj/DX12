#pragma once

#include "ImGui\\imgui.h"

class CImGuiManager
{
public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();
};