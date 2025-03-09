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

	auto Test = [](unsigned long long num) {
		LPWSTR p1 = HexStr(num);
		LPWSTR p2 = NumStr(num);
		LPCWSTR sep = L"\n";

		unsigned int buf_size = wcslen(p1) + wcslen(sep) + wcslen(p2) + 1;
		wchar_t* buf = (wchar_t*) calloc(buf_size, sizeof(wchar_t));
		buf[0] = L'\0';

		wcscat_s(buf, buf_size, p1);
		wcscat_s(buf, buf_size, sep);
		wcscat_s(buf, buf_size, p2);

		Popup(buf);

		free(buf);
		free(p1);
		free(p2);
	};

  //Test(0x0);
  //Test(0xFF);
  //Test(0xFFFFFFFF);
  //Test(0xFFFFFFFFFFFFFFF);
  //Test(0x1000000000000000);
  //Test(0x1FFFFFFFFFFFFFFF);

  //Test(0x7FFFFFFFFFFFFFFF);
  //Test(0x8000000000000000);
  //Test(0x800000000000000F);
  //Test(0xEFFFFFFFFFFFFFFF);
  //Test(0xF000000000000000);
  //Test(0xF0FFFFFFFFFFFFFF);
  //Test(0xFFFFFFFFFFFFFBFF);
  //Test(0xFFFFFFFFFFFFFC00);
  //Test(0xFFFFFFFFFFFFFF0F);

  //Test(0xFFFFFFFFFFFFFBFF);
	Test(0xFFFFFFFFFFFFFBFF);
	Test(0xFFFFFFFFFFFFFC00);
	return 0x01;

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
		buf[0] = L'\0';

		wcscat_s(buf, 2, L"0");

		return buf;
	}
	
	// 5678 /    1 = 5678  -> 5678  % 10 = 8     -> 8
	// 5678 /   10 = 567.8 -> 567.8 % 10 = 7.8   -> 7
	// 5678 /  100 = 56.78 -> 56.78 % 10 = 6.78  -> 6
	// 5678 / 1000 = 5.678 -> 5.678 % 10 = 5.678 -> 5

	int digits = 0;
	while (pow(base, digits ) <= num) digits++;

	int buf_size = digits + 1;		// add one for '\0'

	wchar_t* buf = (wchar_t*) calloc(buf_size, sizeof(wchar_t));
	buf[buf_size - 1] = '\0';

	for (int i = 0; i < digits; i++) {
		//int digit = (int) ((num / (unsigned long long) pow(base, i)) % base);
		int digit = (int) ((num / (unsigned long long) pow(base, i)) % base);

		if (digit >= 10) buf[digits - 1 - i] = (wchar_t) L'A' - (wchar_t) 10 + (wchar_t) digit;
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

unsigned long long power(unsigned long long base, unsigned short exponent) {
	unsigned long long r = 1;
	for (int i = 0; i < exponent; i++) r *= base;

	return r;
}

