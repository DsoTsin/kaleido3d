#include "UICanvas.h"

namespace k3d {
	//win32 StretchDIBits copy image to hDC
	// dc = GetDC(hwnd);
	 //BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(fSurfaceMemory.get());
	 //ZeroMemory(bmpInfo, sizeof(BITMAPINFO));
	 //bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	 //bmpInfo->bmiHeader.biWidth = w;
	 //bmpInfo->bmiHeader.biHeight = -h; // negative means top-down bitmap. Skia draws top-down.
	 //bmpInfo->bmiHeader.biPlanes = 1;
	 //bmpInfo->bmiHeader.biBitCount = 32;
	 //bmpInfo->bmiHeader.biCompression = BI_RGB;
	// void* pixels = bmpInfo->bmiColors;
	// StretchDIBits(dc, 0, 0, fWidth, fHeight, 0, 0,
	//				fWidth, fHeight, bmpInfo->bmiColors, bmpInfo,
    //              DIB_RGB_COLORS, SRCCOPY);
	// SetDIBitsToDevice + UpdateLayeredWindow translucent window
	// ReleaseDC(dc);

	// 
}