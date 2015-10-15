#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <typeinfo>
#include <string>

using namespace std;

//	Function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//	Main.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

//	The window class
	WNDCLASS WindowClass = {};
	WindowClass.style		   = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc	   = (WNDPROC)WndProc; //WNDPROC is pointer to a function taking 4 arguments and returning LRESULT. LRESULT is just a long long.
	WindowClass.hInstance	   = hInstance; // you can also use GetModuleHandle(0);
	WindowClass.hCursor		   = LoadCursor(NULL, IDC_ARROW);
	WindowClass.lpszClassName  = "HandmadeHeroWindowClass";

//	Register the window class
	if (RegisterClass(&WindowClass))
	{
		HWND hWnd = CreateWindowEx(
			0, // Extended window style
			WindowClass.lpszClassName,//Name of previously registered Class
			"Handmade Hero",// Window name
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,//Window style
			CW_USEDEFAULT,//x
			CW_USEDEFAULT,//y
			CW_USEDEFAULT,//Width
			CW_USEDEFAULT,//Height
			0,//Handle to the Parent window ( when having window inside a window )
			0,// Handle to menu
			hInstance, //A handle to the instance of the module to be associated with the window.
			0);

		if (hWnd)
		{
//			Listen for messagess and send them to be handeld
			MSG msg;
			BOOL bRet;
			while ( bRet = GetMessage(&msg, NULL, 0, 0)) // If Message = VM_QUIT, GetMeassage returns 0
			{
				if (bRet == -1)
				{
					MessageBox(NULL, "hWin passed to GetMessage is invalid parameter", "Error 3", NULL);
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			MessageBox(NULL, "CreateWindowEx failed", "Error 2", NULL);
			return -1;
		}

	}
	else
	{
		MessageBox(NULL, "RegidterClass WindowClass failed", "Error 1", NULL);
		return -1;
	}




	return 0;
}

// Main Windows Procedure function. Handles the messages recived from the operating system.
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;

	switch (uMsg)
	{
		case WM_PAINT: // Paint to the window
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(hWnd, &Paint);

			int x = Paint.rcPaint.left;
			int y = Paint.rcPaint.top;
			int height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int width = Paint.rcPaint.right - Paint.rcPaint.left;
			PatBlt(DeviceContext, x, y, width, height, BLACKNESS);

			TextOut(DeviceContext, width/2-50, height/2-50, "Hello world!", _tcslen("Hello world!"));

			EndPaint(hWnd, &Paint);
		}
		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE\n");
			break;
		}
		case WM_DESTROY:
		{
			OutputDebugStringA("WM_DESTROY\n");
			break;
		}
		case WM_CLOSE:
		{
			OutputDebugStringA("WM_CLOSE\n");
			PostQuitMessage(WM_QUIT); // Will exit the while(GetMessage) loop witch will cause the main to finish.
			break;
		}
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
			break;
		}
		default:
		{
//			OutputDebugStringA("default\n");
			Result = DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
		}

	}

	return Result;
}
