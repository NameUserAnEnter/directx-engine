#include "Graphics.h"
#include <math.h>

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

	//return PopupErr(L"Test", 255);
	return PopupErr(L"Test", 2147483647);
	return PopupErr(L"Test", 2147483648);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return S_OK;
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

HRESULT PopupErr(LPCWSTR lpMessage, HRESULT hr, LPCWSTR lpCaption) {
	wchar_t* szCode = HexStr(hr);
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
	return hr;
}

LPWSTR NumStr(unsigned long long num, unsigned int base) {
	if (num == 0) {
		wchar_t* buf = (wchar_t*) calloc(2, sizeof(wchar_t));
		buf[0] = L'0';
		buf[1] = L'\0';

		return buf;
	}

	int digits = 0;
	while (pow(base, digits ) <= num) digits++;

	int buf_size = digits + 1;		// add one for '\0'

	wchar_t* buf = (wchar_t*) calloc(buf_size, sizeof(wchar_t));
	buf[buf_size - 1] = '\0';

	for (int i = 0; i < digits; i++) {
		int digit = (num - (num / (unsigned long long) pow(base, i + 1)) * (unsigned long long) pow(base, i + 1)) / (unsigned long long) pow(base, i);

		if (digit >= 10) buf[digits - 1 - i] = (wchar_t) L'A' - (wchar_t) 10 + wchar_t(digit);
		else buf[digits - 1 - i] = (wchar_t) L'0' + (wchar_t) digit;
	}

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

