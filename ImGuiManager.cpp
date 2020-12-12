#include <iostream>
#include "Renderer.h"
#include "ImGui\imgui_impl_win32.h"
#include "ImGui\imgui_impl_dx12.h"
#include "ImGuiManager.h"

using namespace std;

void CImGuiManager::Init()
{
	DX12App* app = DX12App::GetApp();

	// ImGUI初期化
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

	//iniを生成しないように
	io.IniFilename = NULL;

	//日本語フォントに対応
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
	static bool show_demo_window = true;

	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CRenderer::GetCommondList());
}