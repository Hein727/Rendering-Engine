#include "Framework.h"
#include "../TestScene.h"
#include "../BattleScene.h"
#include "../Imgui/src/ImGuizmo.h"

framework::framework(HWND hwnd) : hwnd(hwnd)
{

}

bool framework::initialized()
{
	// graphics initialize
	graphics& graph = gameContext.graphics;

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
	//sceneManager.ChangeScene(std::make_unique<TestScene>(gameContext, sceneManager, assetManager));
	sceneManager.ChangeScene(std::make_unique<BattleScene>(gameContext, sceneManager, assetManager));

	return true;
}

void framework::update(float delta_time)
{
	auto context = gameContext.graphics.GetDeviceContext();


	// imgui new frame
#ifdef _DEBUG

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
#endif

	// update scene
	sceneManager.Update(delta_time);

	gameContext.input.cameraControls.Update(hwnd, delta_time, context);

#if _DEBUG

	gameContext.input.mouseControl.Update(delta_time);

	if (gameContext.input.mouseControl.GetMouseLeftClick())
	{
		gameContext.input.mouseControl.UpdateMouseRay(gameContext.graphics);
	}

#endif
}

void framework::render(float delta_time)
{
	auto context = gameContext.graphics.GetDeviceContext();
	ID3D11RenderTargetView* null_rtv[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, null_rtv, nullptr);
	ID3D11ShaderResourceView* null_srv[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	context->VSGetShaderResources(0, _countof(null_srv), null_srv);
	context->PSSetShaderResources(0, _countof(null_srv), null_srv);

	graphics& graph = gameContext.graphics;
	
	graph.renderingBegin(gameContext.input.cameraControls);

	sceneManager.Render(delta_time);

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

	gameContext.graphics.uninitialize();

	return true;
}

framework::~framework()
{
}