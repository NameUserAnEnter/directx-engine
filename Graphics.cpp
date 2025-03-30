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

HRESULT LoadMeshFromWavefrontObj(LPCWSTR, LPD3DXMESH*);

COORD GetClientSize();
FLOAT GetClientAspectRatio();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

DWORD	PopupErr(LPCWSTR = L"", DWORD = GetLastError());
DWORD	PopupErr(std::wstring = L"", DWORD = GetLastError());


// Namespace for objects used across this file
namespace Graphics {
	BOOL bWindowCreated = FALSE;
	BOOL bRunLoop		= FALSE;
	HWND hWnd			= NULL;

	LPDIRECT3D9 pd3d			= NULL;
	LPDIRECT3DDEVICE9 pd3ddev	= NULL;

	LPD3DXMESH pmesh			= NULL;

	LPDIRECT3DTEXTURE9 ptexture	= NULL;

	static struct Collector {
		~Collector() {
			if (ptexture != NULL)	ptexture->Release();

			if (pmesh != NULL)		pmesh->Release();

			if (pd3ddev != NULL)	pd3ddev->Release();
			if (pd3d != NULL)		pd3d->Release();
		}
	} _collector;

	struct VERTEX {
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texcoords;
	};

	const DWORD VERTEX_FORMAT = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
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

	HRESULT hr;
	
	std::wstring szTextureFile = L"data/textures/poison.bmp";
	std::wstring szMeshFile = L"data/models/cube.obj";

	// Initialize geometry
	hr = LoadMeshFromWavefrontObj(szMeshFile.c_str(), &pmesh);
	if (hr != S_OK) return 0x01;

	// Load textures
	hr = D3DXCreateTextureFromFile(pd3ddev, szTextureFile.c_str(), &ptexture);
	if (hr != S_OK) return PopupErr(L"Cannot load texture file \"" + szTextureFile + L"\"", 0x02);

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
	UpdateTextures();
	UpdateTransformations();

	// --- Scene rendering ---
	pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, NULL);
	if (pd3ddev->BeginScene() == D3D_OK) {
		// Draw geometry
		LPDIRECT3DVERTEXBUFFER9 pvbuffer = NULL;
		LPDIRECT3DINDEXBUFFER9 pibuffer = NULL;

		pmesh->GetIndexBuffer(&pibuffer);
		pd3ddev->SetIndices(pibuffer);

		pmesh->GetVertexBuffer(&pvbuffer);
		pd3ddev->SetStreamSource(0, pvbuffer, 0, sizeof(VERTEX));

		pd3ddev->SetFVF(VERTEX_FORMAT);

		pd3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, pmesh->GetNumVertices(), 0, pmesh->GetNumFaces());

		pd3ddev->EndScene();
		pd3ddev->Present(NULL, NULL, hWnd, NULL);
	}
}

VOID InitRenderStates() {
	using namespace Graphics;

	pd3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	pd3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
	pd3ddev->SetRenderState(D3DRS_AMBIENT, 0x00202020);
}

VOID UpdateTextures() {
	using namespace Graphics;

	pd3ddev->SetTexture(0, ptexture);

	pd3ddev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pd3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
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

HRESULT LoadMeshFromWavefrontObj(LPCWSTR lpFile, LPD3DXMESH* ppmesh) {
	using namespace Graphics;

	std::vector<D3DXVECTOR3> positions;
	std::vector<D3DXVECTOR3> normals;
	std::vector<D3DXVECTOR2> texcoords;

	std::vector<VERTEX> vertices;
	std::vector<DWORD> indices;

	// Read the file
	std::ifstream fileStream(lpFile);
	
	// If cannot open file ...
	if (fileStream.fail()) return PopupErr(L"Cannot open mesh file \"" + std::wstring(lpFile) + L"\".", 0x01);

	// Consider using std::getline() with EOF checking instead of get() and peek()
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
					// Loop through face vertices
					for (auto token : tokens) {
						VERTEX current_vertex;
						
						enum index_types { pos, tex, norm }; int index_type = index_types::pos;
						// Note the order of index types has to be same as in .obj face data; "f p/t/n ..."

						auto subtokens = SplitByChar(token, '/');
						for (auto subtoken : subtokens) {

							if (!subtoken.empty()) {
								int index = std::atoi(subtoken.c_str()) - 1;

								if (index_type == index_types::pos  && index < positions.size()) current_vertex.position  = positions[index];
								if (index_type == index_types::tex  && index < texcoords.size()) current_vertex.texcoords = texcoords[index];
								if (index_type == index_types::norm && index < normals.size())   current_vertex.normal    = normals[index];
							}

							index_type++;
						}

						// Check if current vertex is a duplicate of any of the vertices already stored in vertex buffer
						// If that's the case add index of the original vertex to the index buffer
						// Otherwise add current vertex to the vertex buffer and store it's vertex buffer index in the index buffer
						bool found = false;
						for (int i = 0; i < vertices.size(); i++) {
							if (vertices[i].position == current_vertex.position &&
								vertices[i].texcoords == current_vertex.texcoords &&
								vertices[i].normal == current_vertex.normal) {

								indices.push_back(i);

								found = true;
								break;
							}
						}

						if (!found) {
							vertices.push_back(current_vertex);
							indices.push_back(vertices.size() - 1);
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

	// Clear resources that are no longer needed
	positions.clear();
	normals.clear();
	texcoords.clear();

	HRESULT hr;
	VERTEX* pvbuffer = NULL;
	DWORD* pibuffer = NULL;
	
	// Create mesh object
	hr = D3DXCreateMeshFVF(indices.size() / 3, vertices.size(), D3DXMESH_MANAGED | D3DXMESH_32BIT, VERTEX_FORMAT, pd3ddev, ppmesh);
	if (hr == E_OUTOFMEMORY)	return PopupErr(L"Cannot create mesh object, out of memory.",	0x02);
	else if (hr != D3D_OK)		return PopupErr(L"Cannot create mesh object.",					0x03);

	// Initialize vertex buffer with local data
	hr = (*ppmesh)->LockVertexBuffer(NULL, (VOID**) &pvbuffer);
	if (hr != D3D_OK)			return PopupErr(L"Cannot lock vertex buffer.", 0x04);

	for (int i = 0; i < vertices.size(); i++) pvbuffer[i] = vertices[i];

	hr = (*ppmesh)->UnlockVertexBuffer();
	if (hr != D3D_OK)			return PopupErr(L"Cannot unlock vertex buffer.", 0x05);
	
	// Initialize index buffer with local data
	hr = (*ppmesh)->LockIndexBuffer(NULL, (VOID**) &pibuffer);
	if (hr != D3D_OK)			return PopupErr(L"Cannot lock index buffer.", 0x06);

	for (int i = 0; i < indices.size(); i++) pibuffer[i] = indices[i];

	hr = (*ppmesh)->UnlockIndexBuffer();
	if (hr != D3D_OK)			return PopupErr(L"Cannot unlock index buffer.",	0x07);

	// Done
	return S_OK;
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

DWORD PopupErr(LPCWSTR lpMessage, DWORD code) {
	Popup(std::wstring(lpMessage) + L" | " + NumStr(code) + L" (" + HexStr(code) + L")", L"Error");

	return code;
}

DWORD PopupErr(std::wstring str, DWORD code) {
	return PopupErr(str.c_str(), code);
}

