#include<windows.h>
#include<d3dx9.h>
#include<dinput.h>

#define SAFE_RELEASE(p){if(p){(p)->Relese();(p)=NULL;}}

LPDIRECT3D9 pD3d;
LPDIRECT3DDEVICE9 pD3Device;
LPDIRECT3DDEVICE9 pDevice;
LPDIRECT3DTEXTURE9 pTexture;
LPDIRECTINPUT8 pDinput = NULL;
LPDIRECTINPUTDEVICE8 pKeyDevice = NULL;
LPD3DXFONT m_pFont;
FLOAT fPosX = 270, fPosY = 180;
D3DPRESENT_PARAMETERS D3dPresentParameters;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM);
HRESULT InitD3d(HWND);
HRESULT InitDinput(HWND);
VOID AppProcess();
VOID FreeDx();

struct CUSTOMVERTEX {
	float x, y, z; // 頂点座標
	float rhw; // 除算数
	DWORD Color; // 頂点の色
	float tu, tv; //　テクスチャ座標
};

CUSTOMVERTEX v[4] =
{
{10, 10, 0.0f, 1.0f, 0xffffffff, 0.0f, 0.0f},//
{200, 10, 0.0f, 1.0f, 0xffffffff, 1.0f, 0.0f},//	ポリゴン頂点の座標と色
{200, 200, 0.0f, 1.0f, 0xffffffff, 1.0f, 1.0f},//
{10, 200 , 0.0f, 1.0f, 0xffffffff,0.0f,1.0f },//
};


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


#define FVF_CUSTOM ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, INT iCmdShow)
{
	HWND hwnd;
	MSG msg;
	DWORD SyncCurr;
	DWORD SyncPrev = timeGetTime();

	static char szAppName[] = "Chapter";
	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(wndclass);//
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//ウィンドウサイズ、タイトル
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindow(szAppName, szAppName, WS_OVERLAPPEDWINDOW,//
		0, 0, 640, 480, NULL, NULL, hInst, NULL);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	ZeroMemory(&D3dPresentParameters, sizeof(D3dPresentParameters));

	D3dPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	D3dPresentParameters.BackBufferCount = 1;
	D3dPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3dPresentParameters.Windowed = TRUE;

	pD3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (FAILED(pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
		D3DCREATE_MIXED_VERTEXPROCESSING,
		&D3dPresentParameters, &pD3Device)))
	{
		return E_FAIL;
	}

	pD3Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pD3Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	D3DXCreateTextureFromFile(
		pD3Device, "pk.png", &pTexture
	);

	if (FAILED(InitDinput(hwnd)))
	{
		return 0;
	}

	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0u, 0u, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			SyncCurr = timeGetTime();
			//1秒間に60回この中に入る
			if (SyncCurr - SyncPrev >= 1000 / 60)
			{
				pD3Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0, 0);
				pD3Device->BeginScene();
				pD3Device->SetTexture(0, pTexture);
				pD3Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(CUSTOMVERTEX));
				pD3Device->EndScene();
				AppProcess();
				//pD3Device->Present(0, 0, 0, 0);

				SyncPrev = SyncCurr;//ゲームの処理
			}
		}
	}

	pD3Device->Release();
	pD3Device = nullptr;
	pD3d->Release();
	pD3d = nullptr;

	return(INT)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch ((CHAR)wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}
HRESULT InitDinput(HWND hWnd) {
	HRESULT hr;
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL),
		DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID * *)& pDinput, NULL)))
	{
		return hr;
	}
	if (FAILED(hr = pDinput->CreateDevice(GUID_SysKeyboard,
		&pKeyDevice, NULL)))
	{
		return hr;
	}
	if (FAILED(hr = pKeyDevice->SetDataFormat(&c_dfDIKeyboard)))
	{
		return hr;
	}
	if (FAILED(hr = pKeyDevice->SetCooperativeLevel(
		hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
	{
		return hr;
	}
	pKeyDevice->Acquire();
	return S_OK;
}
VOID AppProcess()
{
	

	HRESULT hr = pKeyDevice->Acquire();

	if((hr == DI_OK) || (hr == S_FALSE)) {
		BYTE diks[256];
		pKeyDevice->GetDeviceState(sizeof(diks),&diks);

		if (diks[DIK_LEFT] & 0x80)
		{
			for (int i = 0; i < 4; i++)
			{
				v[i].x -= 4;
			}
		}
		if (diks[DIK_RIGHT] & 0x80)
		{
			for (int i = 0; i < 4; i++)
			{
				v[i].x += 4;
			}
		}
		if (diks[DIK_UP] & 0x80)
		{
			for (int i = 0; i < 4; i++)
			{
				v[i].y -= 4;
			}
		}
		if (diks[DIK_DOWN] & 0x80)
		{
			for (int i = 0; i < 4; i++)
			{
				v[i].y += 4;
			}
		}
		pD3Device->Present(NULL, NULL, NULL, NULL);
	}
}