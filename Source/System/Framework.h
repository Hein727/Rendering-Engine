#pragma once

#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <vector>
#include <memory>
#include <imgui.h>
#include <algorithm>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>	
#include <filesystem>

#include "Timer.h"
#include "Misc.h"
#include "GameContext.h"
#include "SceneManager.h"
#include "AssetManager.h"
#include "../HitCheck/CollisionManager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#define APPLICATION_NAME L"Graduation Project"

class framework
{
public :
	CONST HWND hwnd;

	framework(HWND hwnd);
	~framework();

	int run()
	{
		MSG msg{};

		if (!initialized())
		{
			return 0;
		}

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				tictoc.tick();
				calculate_frame_stats();
				update(tictoc.time_interval());
				render(tictoc.time_interval());
			}
		}
		return uninitialized() ? static_cast<int>(msg.wParam) : 0;
	}

	LRESULT CALLBACK message_handler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
#ifdef _DEBUG
		if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
			return true;	
#endif	

		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_CREATE:
			break;

		case WM_KEYDOWN:
			if (wparam == VK_ESCAPE)
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
			break;

		case WM_ENTERSIZEMOVE:
			tictoc.stop();
			break;

		case WM_EXITSIZEMOVE:
			tictoc.start();
			break;

		case WM_MOUSEWHEEL:
		{
			short delta = GET_WHEEL_DELTA_WPARAM(wparam);
			gameContext.input.cameraControls.setWheel(delta);
			break;
		}

		case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wparam;

			char filePath[MAX_PATH];	

			DragQueryFileA(hDrop, 0, filePath, MAX_PATH);

			std::filesystem::path obj(filePath);	

			if (obj.extension() != ".gltf" || obj.extension() == ".glb")
			{
				DragFinish(hDrop);
				break;
			}

			std::string location = filePath;	

			std::replace(location.begin(), location.end(), '\\', '/');

			sceneManager.HandleInput(location);

			DragFinish(hDrop);
		}
		break;

		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

private :
	bool initialized();
	void update(float delta_time);
	void render(float delta_time);
	bool uninitialized();

private :
	timer tictoc;
	uint32_t frame_per_second = 0;
	float counts_per_second = 0.0f;	
	void calculate_frame_stats()
	{
		if (++frame_per_second, (tictoc.time_stamp() - counts_per_second) >= 1.0f)
		{
			float fps = static_cast<float>(frame_per_second);
			std::wostringstream outs;
			outs.precision(6);
			outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
			SetWindowTextW(hwnd, outs.str().c_str());

			frame_per_second = 0;
			counts_per_second += 1.0f;
		}
	}

	GameContext gameContext;

	SceneManager sceneManager;

	AssetManager assetManager;
};