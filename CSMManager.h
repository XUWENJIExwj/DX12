#pragma once

#include "Common\\d3dUtil.h"
#include "Common\\MathHelper.h"

struct CSMPassCB
{

};

class CCSMManager
{
private:
	static CSMPassCB m_CSMPassCB;

public:
	CSMPassCB* GetCSMPassCB() { return &m_CSMPassCB; }
};