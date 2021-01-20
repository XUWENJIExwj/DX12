#include "InputManager.h"
#include "CameraFP.h"

using namespace DirectX;
using namespace InputManager;

#define CAMERA_MOVE 10.0f * GlobalTimer.DeltaTime()
#define CAMERA_ROTX 1.0f * CMouse::GetMoveY() * GlobalTimer.DeltaTime()
#define CAMERA_ROTY 1.0f * CMouse::GetMoveX() * GlobalTimer.DeltaTime()

void CCameraFP::Init()
{
	m_Position = XMFLOAT3(0.0f, 4.0f, -10.0f);
	//m_Position = XMFLOAT3(0.0f, 10.0f, 0.0f);
	m_Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	Pitch(0.1f * MathHelper::Pi);

	ComputeProjectionMatrix(0.25f * MathHelper::Pi, DX12App::GetApp()->GetAspectRatio(), 1.0f, 1000.0f);
}

void CCameraFP::Update(const GameTimer& GlobalTimer)
{
	CameraWalk(GlobalTimer);
	CameraRotate(GlobalTimer);
}

void CCameraFP::UpdateImGui(const GameTimer& GlobalTimer)
{
	static bool showClose = true;
	if (showClose)
	{
		m_ViewDirty = true;

		ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);

		ImGuiWindowFlags window_flags = 0;
		ImGui::Begin(m_Name.c_str(), &showClose, window_flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.18f);
		ImGui::InputScalar(u8"x##0", ImGuiDataType_Float, &m_Position.x, NULL, NULL, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		ImGui::InputScalar(u8"y##0", ImGuiDataType_Float, &m_Position.y, NULL, NULL, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		ImGui::InputScalar(u8"z##0", ImGuiDataType_Float, &m_Position.z, NULL, NULL, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		ImGui::Text(u8"Position");
		ImGui::DragFloat(u8"x##1", &m_Right.x, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::DragFloat(u8"y##1", &m_Right.y, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::DragFloat(u8"z##1", &m_Right.z, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Text(u8"Right");
		ImGui::DragFloat(u8"x##2", &m_Up.x, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::DragFloat(u8"y##2", &m_Up.y, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::DragFloat(u8"z##2", &m_Up.z, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Text(u8"Up");
		ImGui::DragFloat(u8"x##3", &m_Look.x, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::DragFloat(u8"y##3", &m_Look.y, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::DragFloat(u8"z##3", &m_Look.z, 0.01f, -1.0f, 1.0f);
		ImGui::SameLine();
		ImGui::Text(u8"Look");
		ImGui::PopItemWidth();
		ImGui::End();
	}
}

void CCameraFP::WalkDepth(float Speed)
{
	// m_Position += Speed * m_Look
	XMVECTOR s = XMVectorReplicate(Speed);
	XMVECTOR l = XMLoadFloat3(&XMFLOAT3(m_Look.x, 0.0f, m_Look.z));;
	XMVector3Normalize(l);
	XMVECTOR p = XMLoadFloat3(&m_Position);
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, l, p));

	m_ViewDirty = true;
}

void CCameraFP::WalkHorizontal(float Speed)
{
	// m_Position += Speed * m_Right
	XMVECTOR s = XMVectorReplicate(Speed);
	XMVECTOR r = XMLoadFloat3(&m_Right);
	XMVECTOR p = XMLoadFloat3(&m_Position);
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, r, p));

	m_ViewDirty = true;
}

void CCameraFP::WalkVertical(float Speed)
{
	// m_Position += Speed * WorldUp
	XMVECTOR s = XMVectorReplicate(Speed);
	XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR u = XMLoadFloat3(&worldUp);
	XMVECTOR p = XMLoadFloat3(&m_Position);
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, u, p));

	m_ViewDirty = true;
}

void CCameraFP::CameraWalk(const GameTimer& GlobalTimer)
{
	if (CKeyboard::IsDown(Keyboard::W))
	{
		WalkDepth(CAMERA_MOVE);
	}
	if (CKeyboard::IsDown(Keyboard::S))
	{
		WalkDepth(-CAMERA_MOVE);
	}
	if (CKeyboard::IsDown(Keyboard::A))
	{
		WalkHorizontal(-CAMERA_MOVE);
	}
	if (CKeyboard::IsDown(Keyboard::D))
	{
		WalkHorizontal(CAMERA_MOVE);
	}
	if (CKeyboard::IsDown(Keyboard::R))
	{
		WalkVertical(CAMERA_MOVE);
	}
	if (CKeyboard::IsDown(Keyboard::F))
	{
		WalkVertical(-CAMERA_MOVE);
	}
}

void CCameraFP::Pitch(float Angle)
{
	// Rotate up and look vector about the right vector.
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), Angle);

	XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
	XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

	m_ViewDirty = true;
}

void CCameraFP::RotateY(float Angle)
{
	// Rotate the basis vectors about the world y-axis.
	XMMATRIX R = XMMatrixRotationY(Angle);

	XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), R));
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
	XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

	m_ViewDirty = true;
}

void CCameraFP::CameraRotate(const GameTimer& GlobalTimer)
{
	if (CMouse::IsModeRelative())
	{
		Pitch(CAMERA_ROTX);
		RotateY(CAMERA_ROTY);
	}
	else if (CMouse::IsRightButtonDown())
	{
		Pitch(1.0f * (CMouse::GetMoveY() - CMouse::GetLastState().y) * GlobalTimer.DeltaTime());
		RotateY(1.0f * (CMouse::GetMoveX() - CMouse::GetLastState().x) * GlobalTimer.DeltaTime());
	}
}