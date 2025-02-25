#pragma once
#include <windows.h>

class DisplayWin32
{
public:
	int ClientWidth;
	int ClientHeight;
	HWND hWnd;
	DisplayWin32();
	DisplayWin32(int width, int height);
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
};

