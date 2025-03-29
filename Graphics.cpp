#include "Graphics.h"
#include "Utility.h"
#include <math.h>
#include <fstream>
#include <string>
#include <d3d9.h>
#include <d3dx9.h>


// Forward declarations for Functions used only in this file
VOID RenderFrame();

VOID InitRenderStates();

VOID UpdateTextures();
VOID UpdateLighting();
VOID UpdateTransformations();

VOID LoadMeshFromWavefrontObj(LPCWSTR, LPD3DXMESH*);

COORD GetClientSize();
FLOAT GetClientAspectRatio();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

DWORD	PopupErr(LPCWSTR = L"", DWORD = GetLastError(), LPCWSTR = L"");


// Namespace for objects used across this file
namespace Graphics {
	BOOL bWindowCreated = FALSE;
	BOOL bRunLoop		= FALSE;
	HWND hWnd			= NULL;

	LPDIRECT3D9 pd3d			= NULL;
	LPDIRECT3DDEVICE9 pd3ddev	= NULL;

	LPD3DXMESH pmesh			= NULL;

	static struct Collector {
		~Collector() {
			if (pmesh != NULL)		pmesh->Release();

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

	InitRenderStates();

	return S_OK;
}

HRESULT InitResources() {
	using namespace Graphics;

	// Initialize geometry
	LoadMeshFromWavefrontObj(L"data/models/cube.obj", &pmesh);

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

	//UpdateLighting();
	UpdateTextures();
	UpdateTransformations();

	// --- Scene rendering ---
	pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, NULL);
	if (pd3ddev->BeginScene() == D3D_OK) {
		// Draw geometry

		pd3ddev->EndScene();
		pd3ddev->Present(NULL, NULL, hWnd, NULL);
	}
}

VOID InitRenderStates() {
	using namespace Graphics;

	pd3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pd3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);

	//pd3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
	//pd3ddev->SetRenderState(D3DRS_AMBIENT, 0x00202020);
}

VOID UpdateTextures() {
	using namespace Graphics;
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

VOID LoadMeshFromWavefrontObj(LPCWSTR lpFile, LPD3DXMESH* ppmesh) {
	std::vector<D3DXVECTOR3> positions;
	std::vector<D3DXVECTOR3> normals;
	std::vector<D3DXVECTOR2> texcoords;

	struct VERTEX {
		D3DXVECTOR3 position;
		D3DXVECTOR3 normals;
		D3DXVECTOR2 texcoords;
	};

	std::vector<VERTEX> vertices;
	std::vector<DWORD> indices;

	// Read the file
	std::ifstream fileStream(lpFile);
	std::string line = "";
	while (true) {
		line.push_back(fileStream.get());

		int next = fileStream.peek();
		if (next == '\n' || next == EOF) {
			// Process the line ...
			auto tokens = SplitByChar(line, ' ');		// Split line by spaces ' '

			if (!tokens.empty()) {
				auto prefix = tokens[0];
				tokens.erase(tokens.begin());			// Remove prefix from tokens

				if (prefix == "v") {					// If current line starts with a 'v' prefix
					positions.push_back(D3DXVECTOR3());

					if (tokens.size() >= 3) {
						positions.back().x = std::atof(tokens[0].c_str());
						positions.back().y = std::atof(tokens[1].c_str());
						positions.back().z = std::atof(tokens[2].c_str());
					}
				}
				else if (prefix == "vn") {
					normals.push_back(D3DXVECTOR3());

					if (tokens.size() >= 3) {
						normals.back().x = std::atof(tokens[0].c_str());
						normals.back().y = std::atof(tokens[1].c_str());
						normals.back().z = std::atof(tokens[2].c_str());
					}
				}
				else if (prefix == "vt") {
					texcoords.push_back(D3DXVECTOR2());

					if (tokens.size() >= 2) {
						texcoords.back().x = std::atof(tokens[0].c_str());
						texcoords.back().y = std::atof(tokens[1].c_str());
					}
				}
				else if (prefix == "f") {
					vertices.push_back(VERTEX());

					for (auto token : tokens) {
						auto subtokens = SplitByChar(token, '/');
						enum index_type { pos = 1, norm, tex } i = pos;
						for (auto subtoken : subtokens) {
							if (!subtoken.empty()) {
								int index = std::atoi(subtoken.c_str());

								// if i == pos ...
							}

							i++;
						}
					}
				}
			}

			// Break the loop if EOF reached
			if (next == EOF) break;

			line.clear();		// Empty the current line
			fileStream.get();	// At this point the next character is newline, so discard '\n'
		}
	}

	fileStream.close();

	// D3DXCreateMesh ...
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

DWORD PopupErr(LPCWSTR lpMessage, DWORD code, LPCWSTR lpCaption) {
	Popup(std::wstring(lpMessage) + L" | " + NumStr(code) + L" (" + HexStr(code) + L")", lpCaption);

	return code;
}

