#include "mwin.h"
#include "game.h"
#include "windows.h"
HINSTANCE hInst;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;  
    HDC hdc;

	switch (message)
	{
	case WM_KEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_MOUSEACTIVATE:
	case WM_ACTIVATE:
	case WM_MOUSEMOVE:
		StopMovie();
		break;
	case WM_PAINT:  
        hdc = BeginPaint( hWnd, &ps );  
        EndPaint( hWnd, &ps );  
        break;
    case WM_DESTROY:  
        PostQuitMessage( 0 );  
        break;  
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC) WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = (HBRUSH) GetStockObject( WHITE_BRUSH );
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "MWIN";
	wcex.hIconSm = LoadIcon( NULL, IDI_WINLOGO );
	return RegisterClassEx( &wcex );
}

DWORD WINAPI startMWindow(PVOID)
{
	hInst = GetModuleHandle(NULL);
	MyRegisterClass(hInst);

	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	hWnd = CreateWindowEx(WS_EX_TOPMOST, "MWIN", "", WS_DISABLED | WS_POPUP,
		(width - M_WIDTH) / 2, (height - M_HEIGHT) / 2, M_WIDTH, M_HEIGHT, NULL, NULL, hInst, NULL);
	//CreateThread(NULL, 0, ThreadFun, NULL, 0, NULL);

	ygo::mainGame->cmovies = new CMovies();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//TranslateMessage(&msg);
		//DispatchMessage(&msg);
		WndProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}
	return 0;
}



void InitMWindow()
{
	CreateThread(NULL, 0, startMWindow, NULL, 0, NULL);
}

void PlayMovie(char* path)
{
	ygo::mainGame->AddChatMsg(L"按键盘任意键跳过动画", 8);
	//SetForegroundWindow(hWnd);
	ShowWindow(hWnd, SW_SHOWNOACTIVATE);
	//ShowWindow(hWnd, SW_SHOW);
	ygo::mainGame->cmovies->OnStartMovies(path);
}

void StopMovie()
{
	if(IsWindowVisible(hWnd)) {
		ShowWindow(hWnd, SW_HIDE);
		ygo::mainGame->cmovies->OnStopMovies();
	}
}

void HideMWindow()
{
	ShowWindow(hWnd, SW_HIDE);
}

void CloseMWindow()
{
	CloseWindow(hWnd);
	DestroyWindow(hWnd);
}
