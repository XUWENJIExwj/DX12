#pragma once

#include "Camera.h"

class CCameraFP :public CCamera
{
public:
	CCameraFP() = default;
	~CCameraFP() = default;

	void Init()override;
	void Update(const GameTimer& GlobalTimer)override;
	void UpdateImGui(const GameTimer& GlobalTimer)override;

	// Move the camera
	void WalkDepth(float d);
	void WalkHorizontal(float d);
	void WalkVertical(float d);
	void CameraWalk(const GameTimer& GlobalTimer);

	// Rotate the camera.
	void Pitch(float angle);
	void RotateY(float angle);
	void CameraRotate(const GameTimer& GlobalTimer);
};