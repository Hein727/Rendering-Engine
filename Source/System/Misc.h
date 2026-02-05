#pragma once

#include <windows.h>
#include <crtdbg.h>

#if defined(DEBUG) || defined(_DEBUG)
#define _ASSERT_EXPR_(expr, msg) \
	(void)((!!(expr)) || \
	(1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, "%s", msg)) || \
	(_CrtDbgBreak(), 0))
#else
#define _ASSERT_EXPR_(expr, msg) ((void)0)
#endif

inline LPWSTR trace_back(HRESULT hr)
{
	LPWSTR msg{ 0 };
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msg), 0, NULL);
	return msg;
}

class benchmark
{
	LARGE_INTEGER start, cur, freq;

public:
	benchmark()
	{
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start);
		QueryPerformanceCounter(&cur);
	}
	~benchmark() = default;
	benchmark(const benchmark&) = delete;
	benchmark& operator=(const benchmark&) = delete;
	benchmark(benchmark&&) = delete;
	benchmark& operator=(benchmark&&) = delete;

	void begin()
	{
		QueryPerformanceCounter(&start);
	}
	float end()
	{
		QueryPerformanceCounter(&cur);
		return static_cast<float>(cur.QuadPart - start.QuadPart) / static_cast<float>(freq.QuadPart);
	}
};

