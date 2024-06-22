#include <iostream>
#include <conio.h>

#include <Windows.h>
#include <atlimage.h>

using namespace std;

#define ESC			27

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;

	static CImage img;

	

	RECT rect;
	GetWindowRect(hwnd, &rect);

	switch (msg) {
	case WM_CREATE:
		img.Load(_T("MainLogo.bmp"));
		if (img.IsNull()) {
			cerr << "Failed to load image" << endl;
			return -1;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		img.Draw(hdc,0,0,img.GetWidth(),img.GetHeight());
		EndPaint(hwnd, &ps);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

bool MakeWindow(const wchar_t* title, HINSTANCE hInst, HWND hwnd) {
	WNDCLASS wc = {};

	//memset(&wc, 0, sizeof(WNDCLASS));

	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = title;

	auto result = RegisterClass(&wc);
	if(result == 0) {
		cerr << "Failed to register class" << endl;
		return false;
	}

	hwnd = CreateWindowW(title, title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInst, NULL);

	if (hwnd == NULL) return false;

	

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	return true;
}

DWORD WINAPI WindowThread(void* args) {
	HWND hwnd = NULL;
	HINSTANCE hInst = NULL;

	if (!MakeWindow(L"TEST_IMAGE", hInst, hwnd)) {
		cerr << "Failed to MakeWindow" << endl;
		return -1;
	}

	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

int main(int argc, char* argv[], char* env[]) {
	

	HANDLE windowThreadHandle = NULL;

	windowThreadHandle = CreateThread(NULL, 0, WindowThread, NULL, NULL, NULL);
	if (windowThreadHandle == NULL) {
		cerr << "Failed to create thread" << endl;
		return -1;
	}
	
	while (1) {
		if (_kbhit()) {
			if (_getch() == ESC) {
			}
		}
	}

	return 0;
}