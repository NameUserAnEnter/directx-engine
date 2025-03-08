#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Windows.h>

// TODO: consider putting the functions into a namespace to avoid overlapping function names with external functions

HRESULT InitWindow(LPCWSTR, int = 640, int = 480, int = 0, int = 0);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

VOID	Popup(LPCWSTR = L"", LPCWSTR = L"");
HRESULT PopupErr(LPCWSTR = L"", HRESULT = HRESULT_FROM_WIN32(GetLastError()), LPCWSTR = L"");

LPWSTR NumStr(unsigned long long);

#endif

