// CharClient.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ChatClient.h"
#include"Http.h"

Http httpProgram;

HWND hDlg;
HWND hWndProc;
HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wc;
	TCHAR *szAppName = _T("Chat");

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = szAppName;
	wc.lpszMenuName = MAKEINTRESOURCE(IDC_CHATCLIENT);

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, _T("This program requires Windows NT!"),
			szAppName, MB_OK);
		return FALSE;
	}

	hWndProc = CreateWindow(szAppName, _T("客户端"),
		WS_OVERLAPPEDWINDOW,
		800, 200,
		900, 600,
		NULL, NULL, hInstance, NULL);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWndProc, Dlg);

	ShowWindow(hWndProc, nCmdShow);
	UpdateWindow(hWndProc);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		httpProgram.GethWnd(hWnd);
		httpProgram.CreateWin();
		httpProgram.jsonDataRead();   //读取文件信息
		break;

	case WM_SIZE:
		httpProgram.ClientSize(lParam);
		httpProgram.ShowWin();
		break;

	case WM_COMMAND:
		httpProgram.CommMsg(wParam);
		break;

	case WM_CLIENT_SOCKET:
		httpProgram.socketMsg(lParam);
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


BOOL CALLBACK Dlg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hName, hPassword, hLocalIP;

	switch (message)
	{
	case WM_INITDIALOG:
		hName = GetDlgItem(hWnd, IDC_EDIT1);
		hPassword = GetDlgItem(hWnd, IDC_EDIT2);
		hLocalIP = GetDlgItem(hWnd, IDC_DLGIP);
		hDlg = FindWindow(NULL, L"登录");
		return TRUE;

	case WM_COMMAND:
		httpProgram.DlgMsg(hWnd, hName, hPassword, hLocalIP, wParam);
		break;

	case WM_CLOSE:
		EndDialog(hWnd, 0);
		DestroyWindow(hWndProc);
		break;
	}

	return FALSE;
}


BOOL CALLBACK Dlg2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hUser, hPassword, hLocalIP;

	switch (message)
	{
	case WM_INITDIALOG:
		hUser = GetDlgItem(hWnd, IDC_DLGEDIT1);
		hPassword = GetDlgItem(hWnd, IDC_DLGEDIT2);
		hLocalIP = GetDlgItem(hWnd, IDC_DLGIP2);
		return TRUE;

	case WM_COMMAND:
		httpProgram.DlgMsg2(hWnd, hDlg, hUser, hPassword,hLocalIP, wParam);
		break;

	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}

	return FALSE;
}
