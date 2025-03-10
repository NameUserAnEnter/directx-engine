#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Windows.h>

// TODO:
// - consider implementing a memory management class with a single static object for allocation and deallocation

HRESULT InitWindow(LPCWSTR, int = 640, int = 480, int = 0, int = 0);
HRESULT InitGraphics();

HRESULT StartLoop();

#endif

