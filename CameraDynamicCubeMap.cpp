#include "CameraDynamicCubeMap.h"

using namespace DirectX;

void CCameraDCM::Init()
{
	SetProjectionMatrix(0.5f * MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
}