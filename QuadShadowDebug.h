#pragma once

#include "Quad.h"

class CQuadShadowDebug :public CQuad
{
protected:
	UINT m_OrderColNum = 0;

public:
	void Init()override;
	void Update(const GameTimer& GlobalTimer)override;

	void SetOrderColNum(UINT OrderColNum) { m_OrderColNum = OrderColNum; }

	void OnResize()override;
};