#pragma once

#include "GameObject.h"

class CMeshField :public CGameObject
{
protected:
	DirectX::BoundingBox m_Bounds;

public:
	void Init()override;
	void UpdateImGui(const GameTimer & GlobalTimer)override;

	DirectX::BoundingBox* GetBounds() { return &m_Bounds; }
};