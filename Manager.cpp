#include "Manager.h"
#include "Renderer.h"
//#include "CommonResource.h"

const int gNumFrameResources = 3;

bool CManager::Init()
{
	if (!CRenderer::Init())
	{
		return false;
	}

	CRenderer::OnResize();
	CRenderer::CreateCommonResources();

	return true;
}

void CManager::OnResize()
{
	CRenderer::OnResize();
}

void CManager::Uninit()
{
	CRenderer::Uninit();
}

void CManager::Update(const GameTimer& gt)
{

}

void CManager::Draw(const GameTimer& gt)
{
	CRenderer::Begin();

	CRenderer::SetUpCommonResources();
	CRenderer::DrawDynamicCubeScene();
	CRenderer::DrawScene();

	CRenderer::End();
}

ID3D12Device* CManager::GetDevice()
{
	return CRenderer::GetDevice();
}
