#include "Graphics.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	if (InitWindow(L"Window"))	return 0x01;
	if (InitGraphics())			return 0x02;
	if (InitResources())		return 0x03;

	if (StartLoop())			return 0x04;

	// All necessary release calls and memory deallocations should be made by a static Collector object in the Graphics namespace
	return 0;
}

