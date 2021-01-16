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
	void WalkDepth(float Speed);
	void WalkHorizontal(float Speed);
	void WalkVertical(float Speed);
	void CameraWalk(const GameTimer& GlobalTimer);

	// Rotate the camera.
	void Pitch(float Angle);
	void RotateY(float Angle);
	void CameraRotate(const GameTimer& GlobalTimer);
};