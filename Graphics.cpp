#include "Graphics.h"
#include <math.h>
#include <d3d9.h>
#include <d3dx9.h>


#define D3DFVF_CUSTOM		D3DFVF_XYZ | D3DFVF_NORMAL


struct CUSTOMVERTEX {
	D3DXVECTOR3 position, normal;
};


// Forward declarations for Functions used only in this file
VOID RenderFrame();
VOID UpdateLighting();
VOID UpdateTransformations();

COORD GetClientSize();
FLOAT GetClientAspectRatio();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

VOID	Popup(LPCWSTR = L"", LPCWSTR = L"");
DWORD	PopupErr(LPCWSTR = L"", DWORD = GetLastError(), LPCWSTR = L"");

LPWSTR NumStr(unsigned long long, unsigned int = 10);
LPWSTR HexStr(unsigned long long);


// Namespace for objects used across this file
namespace Graphics {
	BOOL bWindowCreated = FALSE;
	BOOL bRunLoop		= FALSE;
	HWND hWnd			= NULL;

	LPDIRECT3D9 pd3d			= NULL;
	LPDIRECT3DDEVICE9 pd3ddev	= NULL;

	LPDIRECT3DVERTEXBUFFER9 pvbuffer = NULL;

	static struct Collector {
		~Collector() {
			if (pvbuffer != NULL)	pvbuffer->Release();

			if (pd3ddev != NULL)	pd3ddev->Release();
			if (pd3d != NULL)		pd3d->Release();
		}
	} _collector;
};


// Function definitions
HRESULT InitWindow(LPCWSTR lpWindowTitle, int nClientWidth, int nClientHeight, int x, int y) {
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));

	WNDCLASS wc = { };
	wc.hInstance = (HINSTANCE) GetModuleHandle(NULL);
	wc.lpfnWndProc = WndProc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpszClassName = L"WindowClass";
	wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

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
	using namespace Graphics;

	// Create D3D object
	pd3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (pd3d == NULL) return PopupErr(L"Cannot create D3D object.", 0x01);

	// Initialize D3D present parameters
	D3DPRESENT_PARAMETERS d3dpp = { };
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	// Create D3D device
	HRESULT hr = pd3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pd3ddev);
	if (hr != D3D_OK) return PopupErr(L"Cannot create D3D device.", 0x02);

	// Set rendering states
	pd3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	// temporary until fully 3d models are used
	//pd3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);		// temporarily disable lighting until vertex normalls are added to the FVF

	// to do: disable culling and try to correct cylinder artifacts with z/stencil buffer used as in the reference?

	pd3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
	pd3ddev->SetRenderState(D3DRS_AMBIENT, 0x00202020);

	return S_OK;
}

HRESULT InitResources() {
	using namespace Graphics;

	// Create vertex buffer
	HRESULT hr = pd3ddev->CreateVertexBuffer(6 * sizeof(CUSTOMVERTEX), NULL, D3DFVF_CUSTOM, D3DPOOL_MANAGED, &pvbuffer, NULL);
	if (hr != D3D_OK) return PopupErr(L"Cannot create vertex buffer.", 0x01);

	CUSTOMVERTEX* vertices = NULL;

	hr = pvbuffer->Lock(0, 0, (VOID**) &vertices, NULL);
	if (hr != D3D_OK) return PopupErr(L"Cannot lock vertex buffer.", 0x02);

	vertices[0].position = { -1.0,  1.0, -1.0 };
	vertices[1].position = {  1.0,  1.0, -1.0 };
	vertices[2].position = {  1.0, -1.0, -1.0 };

	vertices[3].position = { -1.0,  1.0, -1.0 };
	vertices[4].position = {  1.0, -1.0, -1.0 };
	vertices[5].position = { -1.0, -1.0, -1.0 };

	for (int i = 0; i < 6; i++) vertices[i].normal = { 0, 0, -1 };

	hr = pvbuffer->Unlock();
	if (hr != D3D_OK) return PopupErr(L"Cannot unlock vertex buffer.", 0x03);

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


// Function definitions for local functions
VOID RenderFrame() {
	using namespace Graphics;

	UpdateLighting();
	UpdateTransformations();

	// --- Scene rendering ---
	pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, NULL);
	if (pd3ddev->BeginScene() == D3D_OK) {

		pd3ddev->SetStreamSource(0, pvbuffer, 0, sizeof(CUSTOMVERTEX));
		pd3ddev->SetFVF(D3DFVF_CUSTOM);

		pd3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);		// adjust to the cylinder shape

		pd3ddev->EndScene();
		pd3ddev->Present(NULL, NULL, hWnd, NULL);
	}
}

VOID UpdateLighting() {
	using namespace Graphics;

	// Set material
	D3DMATERIAL9 material = { };
	material.Diffuse = { 1.0f, 1.0f, 0.0f, 1.0f };
	material.Ambient = { 1.0f, 1.0f, 0.0f, 1.0f };
	
	pd3ddev->SetMaterial(&material);

	// Set light
	D3DLIGHT9 light = { };
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse = { 1.0f, 1.0f, 1.0f, 0.0f };
	light.Direction = { 0, 0, 1 };

	pd3ddev->SetLight(0, &light);
	pd3ddev->LightEnable(0, TRUE);
}

VOID UpdateTransformations() {
	using namespace Graphics;

	// World transformation
	D3DXMATRIX matWorld, matView, matProj;

	static float angle = 0.0f;
	angle += D3DX_PI / 4;

	D3DXMATRIX matRotation, matTranslation;
	D3DXMatrixRotationYawPitchRoll(&matRotation, angle / 50.f, 0, 0);
	D3DXMatrixTranslation(&matTranslation, 0, 0, 0);

	D3DXMatrixMultiply(&matWorld, &matTranslation, &matRotation);
	pd3ddev->SetTransform(D3DTS_WORLD, &matWorld);

	// View transformation
	D3DXVECTOR3 eye(0, 1, -5);
	D3DXVECTOR3 target(0, 0, 0);
	D3DXVECTOR3 up(0, 1, 0);

	D3DXMatrixLookAtLH(&matView, &eye, &target, &up);
	pd3ddev->SetTransform(D3DTS_VIEW, &matView);

	// Projection transformation
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, GetClientAspectRatio(), 1.0f, 100.0f);
	pd3ddev->SetTransform(D3DTS_PROJECTION, &matProj);
}

COORD GetClientSize() {
	RECT rc = { };
	GetClientRect(Graphics::hWnd, &rc);

	return { (SHORT) (rc.right - rc.left), (SHORT) (rc.bottom - rc.top) };
}

FLOAT GetClientAspectRatio() {
	COORD client = GetClientSize();
	if (client.Y == 0) return 0.0f;

	return (FLOAT) client.X / (FLOAT) client.Y;
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
	const wchar_t szSep[] = L" | ";

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

