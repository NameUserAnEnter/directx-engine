#include "Graphics.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	if (InitWindow(L"Window"))	return 0x01;
	if (InitGraphics())			return 0x02;

	if (StartLoop())			return 0x03;

	return 0;
}

