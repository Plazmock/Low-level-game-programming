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

// bitmap struct
struct win32_offscreen_buffer{
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

// global variables for now
global_variable	bool running = true;
global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimention
{
	int width;
	int height;
};

win32_window_dimention Win32GetWindowDimention(HWND Window)
{
	win32_window_dimention Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.width = ClientRect.right - ClientRect.left;
	Result.height = ClientRect.bottom - ClientRect.top;

	return Result;
}

// Render wierd colored moving boxes on the creen
internal void RenderWierdGradient(win32_offscreen_buffer Buffer, int xOffset, int yOffset)
{
	int width = Buffer.Width;
	int height = Buffer.Height;
	

	uint8 *Row = (uint8 *)Buffer.Memory;
	for (int y = 0; y < Buffer.Height; ++y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int x = 0; x < Buffer.Width; ++x)
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
		Row += Buffer.Pitch;
	}
}


// Resize Divice Independent Bitmap
// Fill the BitmapMemory that Win32UpdateWindow uses to PAINT on the window
internal void Win32ResizeDIBSelect(win32_offscreen_buffer *Buffer, int width, int height)
{
	// TODO: Booletproof this
	// Maybe dont free first, free after, then free is that fails
	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = width;
	Buffer->Height = height;
	Buffer->BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // Negative Height makes the Bitmap a Topdown DIP with origin upper-left corrner
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	int BitmapMemorySize = Buffer->Width*Buffer->Height*Buffer->BytesPerPixel;
	Buffer->Memory = VirtualAlloc(NULL, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	Buffer->Pitch = width*Buffer->BytesPerPixel;


}

// Put the BitmapMemory on the screen
internal void Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer Buffer,
											int x, int y, int width, int height)
{

	// BitBlt is better alternative?
	// TODO: Aspect ratio correction 
	StretchDIBits(DeviceContext,
		/*
		x, y, width, height,
		x, y, width, height,
		*/
		0, 0, WindowWidth, WindowHeight, // destination
		0, 0, Buffer.Width, Buffer.Height, // source
		Buffer.Memory, &Buffer.Info,
		DIB_RGB_COLORS, SRCCOPY);

}


// Main Windows Procedure function. Handles the messages recived from the operating system.
LRESULT CALLBACK WndProc(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;

	switch (uMsg)
	{
	
	case WM_SIZE:
	{
		
	} break;

	case WM_PAINT: // Paint to the window
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		int x = Paint.rcPaint.left;
		int y = Paint.rcPaint.top;
		int width = Paint.rcPaint.right - Paint.rcPaint.left;
		int height = Paint.rcPaint.bottom - Paint.rcPaint.top;

		// Clean up double ClientRect use in code?
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		win32_window_dimention Dimention = Win32GetWindowDimention(Window);
		Win32DisplayBufferInWindow(DeviceContext, Dimention.width, Dimention.height, GlobalBackBuffer, x, y, width, height);
		EndPaint(Window, &Paint);
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
		Result = DefWindowProc(Window, uMsg, wParam, lParam);
	} break;

	}

	return Result;
}

//	Main.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	
//	The window class
	WNDCLASS WindowClass = {};

	Win32ResizeDIBSelect(&GlobalBackBuffer, 1280, 720);

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

			while (running)
			{	
				MSG msg;
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
				RenderWierdGradient(GlobalBackBuffer, xOffset, yOffset);
				HDC DeviceContext = GetDC(Window);

				win32_window_dimention Dimention = Win32GetWindowDimention(Window);
				Win32DisplayBufferInWindow(DeviceContext, Dimention.width, Dimention.height, GlobalBackBuffer, 0, 0, Dimention.width, Dimention.height);
				ReleaseDC(Window, DeviceContext);
				

				++xOffset;
				yOffset++;
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
