#include <iostream>
#include "Manager.h"
#include "InputManager.h"
#include "Renderer.h"
#include "Scene.h"
#include "ImGui\imgui_impl_win32.h"
#include "ImGui\imgui_impl_dx12.h"
#include "ImGuiManager.h"

using namespace std;
using namespace InputManager;
using namespace DirectX;

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
	static bool show_demo_window = false;

	// "["でDemoWindowの表示を切り替える
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
	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);

	if (showClose)
	{
		ImGui::Begin(u8"操作方法", &showClose, window_flags);
		ImGui::Text(u8"移動：W・S・A・D");
		ImGui::Text(u8"Mouseの表示/非表示：LeftAlt");
		ImGui::Text(u8"Camera操作：Mouse/ImGuiのWindow");
		ImGui::Text(u8"　Mouse表示時：MouseのRightButtonを押したまま、Mouseを移動");
		ImGui::Text(u8"　Mouse非表示時：Mouseを移動");
		ImGui::Text(u8"その他：ImGuiのWindowでオブジェクトのプロパティを変更");
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CRenderer::GetCommondList());
}