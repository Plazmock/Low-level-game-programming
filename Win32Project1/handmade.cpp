#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <typeinfo>
#include <string>
#include <stdint.h>

using namespace std;

typedef UINT8 uint8;
typedef UINT16 uint16;
typedef UINT32 uint32;
typedef UINT64 uint64;

#define internal static
#define local_persist static
#define global_variable static

// global variables
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;
global_variable bool running = true;

//	Function declarations
//	The function that handles messages to the window
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//	Resize Divice Independent Bitmap
internal void Win32ResizeDIBSelect(int width, int heigth);
internal void Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int x, int y, int width, int height);
internal void RenderWierdGradient(int xOffset, int yOffset);

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
		HWND Window = CreateWindowEx( // HWND is Pointer to int
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

		if (Window)
		{
//			Listen for messagess and send them to be handeld 
			int xOffset = 0;
			int yOffset = 0;
			MSG msg;
			while (running)
			{	
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) // If Message = VM_QUIT, GetMeassage returns 0
				{
					if (msg.message == WM_QUIT)
					{
						running = false;
					}
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				// Renter and sipaly WierdGradient
				RenderWierdGradient(xOffset, yOffset);
				HDC DeviceContext = GetDC(Window);
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
				ReleaseDC(Window, DeviceContext);
				

				++xOffset;
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
			int width = Paint.rcPaint.right - Paint.rcPaint.left;
			int height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			// Clean up double ClientRect use in code?
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			Win32UpdateWindow(DeviceContext, &ClientRect, x, y, width, height);
			EndPaint(hWnd, &Paint);
		} break;

		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(hWnd, &ClientRect);
			int width = ClientRect.right - ClientRect.left;
			int height = ClientRect.bottom - ClientRect.top;
			Win32ResizeDIBSelect(width, height);
			

		} break;

		case WM_DESTROY:
		{
			PostQuitMessage(WM_QUIT); // Will end the while(GetMessage) loop witch will cause the main to finish.
		} break;

		case WM_CLOSE:
		{
			PostQuitMessage(WM_QUIT); // Will end the while(GetMessage) loop witch will cause the main to finish.
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		default:
		{
			Result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		} break;
		
	}

	return Result;
}
// Resize Divice Independent Bitmap
// Fill the BitmapMemory that Win32UpdateWindow uses to PAINT on the window
internal void Win32ResizeDIBSelect(int width, int height)
{
	// TODO: Booletproof this
	// Maybe dont free first, free after, then free is that fails
	if (BitmapMemory)
	{
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}

	BitmapWidth = width;
	BitmapHeight = height;
	
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight; // Negative Height makes the Bitmap a Topdown DIP with origin upper-left corrner
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	
	int BitmapMemorySize= BitmapWidth*BitmapHeight*BytesPerPixel;
	BitmapMemory = VirtualAlloc(NULL, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	// TODO: Probably clear this to black

}

// Put the BitmapMemory on the screen
internal void Win32UpdateWindow(HDC DeviceContext,RECT *ClientRect, int x, int y, int width, int height)
{
	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;
	// BitBlt is better alternative?
	StretchDIBits(DeviceContext,
					/*
					x, y, width, height,
					x, y, width, height,
					*/
					0, 0, BitmapWidth, BitmapHeight,
					0, 0, WindowWidth, WindowHeight,
					BitmapMemory, &BitmapInfo,
					DIB_RGB_COLORS, SRCCOPY);

}

internal void RenderWierdGradient(int xOffset, int yOffset)
{
	int width = BitmapWidth;
	int height = BitmapHeight;

	int Pitch = width*BytesPerPixel;
	uint8 *Row = (uint8 *)BitmapMemory;
	for (int y = 0; y < BitmapHeight; ++y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int x = 0; x < BitmapWidth; ++x)
		{
			/*
			Pixel in Memory:	BB GG RR xx
			Pixel in Register:  xx RR GG BB
			0xxxRRGGBB
			*/
			uint8 Blue = x + xOffset;
			uint8 Green = y + yOffset;
			
			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Pitch;
	}
}