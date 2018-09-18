// ChatServer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ChatServer.h"
#include"HttpServer.h"

HttpServer httpServer;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
                   PSTR lpCmdLine,int nCmdShow)
{
	MSG msg;
	HWND hWnd;
	WNDCLASS wc;
	TCHAR *szAppName = _T("Server");

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = szAppName;
	wc.lpszMenuName = MAKEINTRESOURCE(IDC_CHATSERVER);

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, _T("This program requires Windows NT!"), szAppName, MB_OK);
		return FALSE;
	}

	hWnd = CreateWindow(szAppName, _T("服务器"),
		WS_OVERLAPPEDWINDOW,
		800, 200,
		900, 600,
		NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{	
		TranslateMessage(&msg);
		DispatchMessage(&msg);		
	}

	return (int) msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInst;
	switch (message)
	{
	case WM_CREATE:
		hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		httpServer.GethWnd(hWnd);
		httpServer.createwindow();			
		httpServer.InitSocket();				
		break;

	case WM_SIZE:
		httpServer.ClientSize(lParam);
		httpServer.showwindow();
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		}
		break;

	case WM_SERVER_SOCKET:
		httpServer.socketMsg(lParam,wParam);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;	
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


BOOL CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
