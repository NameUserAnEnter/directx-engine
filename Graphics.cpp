#include "Graphics.h"
#include <math.h>

namespace Graphics {
	BOOL bWindowCreated = FALSE;
	BOOL bRunLoop		= FALSE;
	HWND hWnd			= NULL;
};

VOID RenderFrame();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

VOID	Popup(LPCWSTR = L"", LPCWSTR = L"");
DWORD	PopupErr(LPCWSTR = L"", DWORD = GetLastError(), LPCWSTR = L"");

LPWSTR NumStr(unsigned long long, unsigned int = 10);
LPWSTR HexStr(unsigned long long);

HRESULT InitWindow(LPCWSTR lpWindowTitle, int nClientWidth, int nClientHeight, int x, int y) {
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));

	WNDCLASS wc = { };
	wc.hInstance = (HINSTANCE) GetModuleHandle(NULL);
	wc.lpfnWndProc = WndProc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpszClassName = L"WindowClass";
	wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

	RegisterClass(&wc);

	DWORD dwStyle = WS_OVERLAPPEDWINDOW;

	RECT rc;
	SetRect(&rc, 0, 0, nClientWidth, nClientHeight);
	AdjustWindowRect(&rc, dwStyle, false);

	int nWindowWidth = rc.right - rc.left;
	int nWindowHeight = rc.bottom - rc.top;

	HWND hWnd = CreateWindowW(wc.lpszClassName, lpWindowTitle, dwStyle, x, y, nWindowWidth, nWindowHeight, NULL, NULL, wc.hInstance, NULL);
	if (hWnd == NULL) {
		return PopupErr(L"CreateWindow");
	}

	Graphics::hWnd = hWnd;
	Graphics::bWindowCreated = TRUE;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return S_OK;
}

HRESULT InitGraphics() {
	return S_OK;
}

HRESULT StartLoop() {
	if (!Graphics::bWindowCreated) return 0x01;

	MSG msg = { };

	Graphics::bRunLoop = TRUE;

	while (Graphics::bRunLoop) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		RenderFrame();
	}

	return S_OK;
}

VOID RenderFrame() {
	//
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_MOUSEWHEEL:
			break;
		case WM_KEYUP:
			break;
		case WM_KEYDOWN:
			break;

		case WM_DESTROY:
			Graphics::bRunLoop = FALSE;
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

VOID Popup(LPCWSTR lpMessage, LPCWSTR lpCaption) {
	MessageBoxW(NULL, lpMessage, lpCaption, MB_OK);
}

DWORD PopupErr(LPCWSTR lpMessage, DWORD code, LPCWSTR lpCaption) {
	wchar_t* szCode = HexStr(code);
	const wchar_t szSep[] = L": ";

	int buf_size = wcslen(lpMessage) + wcslen(szSep) + wcslen(szCode) + 1;
	wchar_t* buf = (wchar_t*) calloc(buf_size, sizeof(wchar_t));
	buf[0] = L'\0';
	
	wcscat_s(buf, buf_size, lpMessage);
	wcscat_s(buf, buf_size, szSep);
	wcscat_s(buf, buf_size, szCode);
	Popup(buf, lpCaption);

	free(szCode);
	free(buf);
	return code;
}

LPWSTR NumStr(unsigned long long num, unsigned int base) {
	if (base == 0) return nullptr;

	int digits = 0;
	unsigned long long remaining = num;

	do {
		digits++;

		remaining /= base;
	} while(remaining > 0);

	int buf_size = digits + 1;		// add one for '\0'

	wchar_t* buf = (wchar_t*) calloc(buf_size, sizeof(wchar_t));
	buf[buf_size - 1] = '\0';

	for (int i = 0; i < digits; i++) {
		int digit = (int) ((num / (unsigned long long) pow(base, i)) % base);

		wchar_t starting_point = L'0';
		if (digit >= 10) starting_point = L'A' - (wchar_t) 10;
		
		buf[digits - 1 - i] = starting_point + (wchar_t) digit;
	}

	// memory pointed to by the returned pointer has to be freed when no longer needed
	return buf;
}

LPWSTR HexStr(unsigned long long num) {
	const wchar_t prefix[] = L"0x";
	wchar_t* num_part = NumStr(num, 16);

	int buf_size = wcslen(prefix) + wcslen(num_part) + 1;
	wchar_t* buf = (wchar_t*) calloc(buf_size, sizeof(wchar_t));
	buf[0] = L'\0';

	wcscat_s(buf, buf_size, prefix);
	wcscat_s(buf, buf_size, num_part);

	return buf;
}

