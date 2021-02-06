#include "Manager.h"
#include "Scene.h"
#include "Camera.h"
#include "DirectionalLight.h"

using namespace std;
using namespace DirectX;

void CDirLight::Update(const GameTimer& GlobalTimer)
{
	if (m_Move)
	{
		m_Rotation.y += 0.2f * GlobalTimer.DeltaTime();
	}
}

XMFLOAT3 CDirLight::ComputeDirection3f()
{
	ComputeDirection();
	return m_Direction;
}

XMVECTOR XM_CALLCONV CDirLight::ComputeDirection()
{
	XMVECTOR direction = XMLoadFloat3(&m_BaseDirection);
	direction = XMVector3TransformNormal(direction, XMMatrixRotationY(m_Rotation.y));
	XMStoreFloat3(&m_Direction, direction);
	return direction;
}

XMMATRIX XM_CALLCONV CDirLight::ComputeLightView(BoundingSphere* SceneBounds)
{
	XMVECTOR lightDir = ComputeDirection();
	XMVECTOR lightPos = -2.0f * SceneBounds->Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&SceneBounds->Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&m_Position, lightPos);
	XMStoreFloat4x4(&m_View, lightView);

	return lightView;
}

void CDirLight::UpdateImGui(const GameTimer& GlobalTimer)
{
	static bool showClose = true;

	if (showClose && m_Name == "DirLight00")
	{
		ImGui::SetNextWindowPos(ImVec2((float)DX12App::GetApp()->GetWindowWidth() - 120, 500), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(100, 65), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(u8"DirLight", &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
		ImGui::Checkbox(u8"Move", &m_Move);
		ImGui::PopItemWidth();
		ImGui::End();
	}
}