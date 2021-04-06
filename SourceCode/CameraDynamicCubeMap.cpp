#include "CameraDynamicCubeMap.h"

using namespace DirectX;

void CCameraDCM::Init()
{
	ComputeProjectionMatrix(0.5f * MathHelper::Pi, 1.0f, 0.1f, 1000.0f);
}