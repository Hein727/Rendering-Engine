#include <time.h>
#include "Framework.h"
#include "../RenderingComponents/Graphics.h"


// Window procedure for handling messages
LRESULT CALLBACK window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	framework* p{ reinterpret_cast<framework*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) };
	return p ? p->message_handler(hwnd, msg, wparam, lparam) : DefWindowProc(hwnd, msg, wparam, lparam);

}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev_instance, _In_ LPSTR cmd_line, _In_ int cmd_show)
{
	srand(static_cast<unsigned int>(time(nullptr)));


	// Enable memory leak detection in debug builds
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtBreakAlloc(3096); // Set a breakpoint on a specific allocation number for debugging
#endif

	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = window_procedure;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);	
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = APPLICATION_NAME;
	wcex.hIconSm = 0;
	RegisterClassExW(&wcex);

	RECT rc{ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hwnd = CreateWindowExW(0, APPLICATION_NAME, L"", WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, instance, nullptr);
	ShowWindow(hwnd, cmd_show);

#ifdef _DEBUG
	DragAcceptFiles(hwnd, TRUE);
#endif

	framework framework(hwnd);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&framework));
	return framework.run();	
}
