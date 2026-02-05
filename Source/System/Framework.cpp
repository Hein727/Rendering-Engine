#include "Framework.h"
#include "Graphics.h"
#include "../SceneManager.h"
#include "../TestScene.h"

framework::framework(HWND hwnd) : hwnd(hwnd)
{

}

bool framework::initialized()
{
	// graphics initialize
	graphics& graph = graphics::getInstance();

	graph.initialize(hwnd);

	// imgui initialize
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// stepup platform/renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(graph.GetDevice(), graph.GetDeviceContext());

	// scene manager initialize(the first scene to load)
	SceneManager::GetInstance().ChangeScene(std::make_unique<TestScene>());

	return true;
}

void framework::update(float delta_time)
{
	auto context = graphics::getInstance().GetDeviceContext();
	// imgui new frame
#ifdef _DEBUG
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif

	// update scene
	SceneManager::GetInstance().Update(delta_time);

	camera_controls::instance().Update(hwnd, delta_time, context);
}

void framework::render(float delta_time)
{
	graphics& graph = graphics::getInstance();
	
	graph.renderingBegin();

	SceneManager::GetInstance().Render();

#ifdef _DEBUG

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#endif

	graph.renderingEnd();
}

bool framework::uninitialized()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return true;
}

framework::~framework()
{

}