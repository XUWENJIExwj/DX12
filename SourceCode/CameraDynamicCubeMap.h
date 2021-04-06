#pragma once

#include "Camera.h"

class CCameraDCM :public CCamera
{
public:
	CCameraDCM() = default;
	~CCameraDCM() = default;

	void Init()override;
	void LateUpdate(const GameTimer& GlobalTimer)override {}
};