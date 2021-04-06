#pragma once

#include "..\\ImGui\\imgui.h"
#include "..\\Common\\GameTimer.h"

struct GameObjectImGuiCB
{
	bool ShowClose = false;
	int  MaterialIndex = -1;
};

class CImGuiManager
{
public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();
};