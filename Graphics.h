#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Windows.h>

HRESULT InitWindow(LPCWSTR, int = 640, int = 480, int = 0, int = 0);
HRESULT InitGraphics();
HRESULT InitResources();

HRESULT StartLoop();

#endif

