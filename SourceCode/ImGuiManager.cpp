#include <iostream>
#include "Manager.h"
#include "InputManager.h"
#include "Renderer.h"
#include "Scene.h"
#include "..\\ImGui\imgui_impl_win32.h"
#include "..\\ImGui\imgui_impl_dx12.h"
#include "ImGuiManager.h"

using namespace std;
using namespace InputManager;
using namespace DirectX;

void CImGuiManager::Init()
{
	DX12App* app = DX12App::GetApp();

	// ImGUI������
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	if (!ImGui_ImplWin32_Init(app->GetMainWindowHandle()))
	{
		cout << "ImGui_ImplWin32_Init failed\n";
		ImGui::DestroyContext();
		UnregisterClass(app->GetWindowClassName(), app->GetAppInstanceHandle());
		exit(1);
	}

	ID3D12DescriptorHeap* srvHeap = CRenderer::GetSrvHeap();
	if (!ImGui_ImplDX12_Init(CRenderer::GetDevice(), 3, CRenderer::GetBackBufferFormat(),
		srvHeap, srvHeap->GetCPUDescriptorHandleForHeapStart(), srvHeap->GetGPUDescriptorHandleForHeapStart()))
	{
		std::cout << "ImGui_ImplDX12_Init failed\n";
		ImGui::DestroyContext();
		UnregisterClass(app->GetWindowClassName(), app->GetAppInstanceHandle());
		std::exit(1);
	}

	//ini�𐶐����Ȃ��悤��
	io.IniFilename = NULL;

	//���{��t�H���g�ɑΉ�
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\meiryo.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
}

void CImGuiManager::Uninit()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void CImGuiManager::Update()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void CImGuiManager::Draw()
{
	static bool show_demo_window = false;

	// "["��DemoWindow�̕\����؂�ւ���
	if (CKeyboard::IsPressed(Keyboard::OemOpenBrackets))
	{
		show_demo_window = !show_demo_window;
	}

	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	static bool showClose = true;
	ImGuiWindowFlags window_flags = 0;
	DX12App* app = DX12App::GetApp();

	ImGui::SetNextWindowPos(ImVec2((float)app->GetWindowWidth() - 420, 20), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(400, 170), ImGuiCond_Once);

	if (showClose)
	{
		ImGui::Begin(u8"������@", &showClose, window_flags);
		ImGui::Text(u8"�ړ��FW�i�O�j�ES�i��j�EA�i���j�ED�i�E�j�ER�i��j�EF�i���j");
		ImGui::Text(u8"Mouse�̕\��/��\���FLeftAlt");
		ImGui::Text(u8"Camera����FMouse/ImGui��Window");
		ImGui::Text(u8"�@Mouse�\�����FMouse���E�N���b�N�����܂܁AMouse���ړ�");
		ImGui::Text(u8"�@Mouse��\�����FMouse���ړ�");
		ImGui::Text(u8"���̑��FImGui��Window�ŃI�u�W�F�N�g�̃v���p�e�B��ύX");
		ImGui::End();
	}

	static bool showCloseb = true;

	ImGui::SetNextWindowPos(ImVec2((app->GetWindowWidth() - 500) * 0.5f, 20), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(420, 170), ImGuiCond_Once);

	if (showCloseb)
	{
		ImGui::Begin(u8"�A�s�[���|�C���g", &showCloseb, window_flags);
		ImGui::Text(u8"�EDX12�̃t���[�����[�N��Ɗw��1����\�z");
		ImGui::Text(u8"�E�����̃I�u�W�F�N�g�ɓ��I���}�b�s���O�̎���");
		ImGui::Text(u8"�EParallaxOcclusionMapping�̎����E�Z���t�V���h�E�t��");
		ImGui::Text(u8"�EMaterialManager������āA���ꂼ���Material���Ǘ�����@�\�̎���");
		ImGui::Text(u8"�@�E�t���l�����ˁARoughness�A���ʋ�Ȃǂ̒���");
		ImGui::Text(u8"�@�E���}�b�s���O�̐F�ω��ɂ��Object�̐F�ω��̊m�F");
		ImGui::Text(u8"�ECacadeShadowMapping�̎���");
		ImGui::Text(u8"�EPostProcessing�̎���");
		ImGui::Text(u8"�@�ERadialBlur");
		ImGui::Text(u8"�@�EGaussBlur");
		ImGui::Text(u8"�@�EBloom");
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CRenderer::GetCommandList());
}