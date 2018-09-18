#include "stdafx.h"
#include "HttpServer.h"
#include"Resource.h"

HttpServer::HttpServer()
{
	_hm.AddrLen = 0;
	_hm._sockClient = _hm._sockSrv = NULL;
	memset(&_hm._wsadata, 0, sizeof(WSADATA));
	memset(&_hm._clientAddr, 0, sizeof(sockaddr_in));
	memset(&_hm._RemoteAddr, 0, sizeof(sockaddr_in));

	_wm._hfont = NULL;
	_wm._hInst = NULL;
	_wm._cxClient = _wm._cyClient = 0;
	_wm._hWnd = _wm._hListBox = _wm._hGroup
		= _wm._hListBox2 = _wm._hStatic = NULL;	
	
	_users.nSockets = NULL;
	_userVec.clear();
}

HttpServer::~HttpServer()
{
	WSACleanup();
	closesocket(_hm._sockClient);
	closesocket(_hm._sockSrv);	
	
	for (int i = 0; i < _userVec.size(); i++)
	{
		delete[]_userVec[i].User_Name;
		closesocket(_userVec[i].nSockets);
	}
	
	closesocket(_users.nSockets);
	delete[]_users.User_Name;

	DeleteObject(_wm._hfont);
	
	delete[]_hm._RecvBuf;
	delete[]_hm._TempBuf;
}

void HttpServer::createwindow()
{
	_wm._hInst = (HINSTANCE)GetWindowLong(_wm._hWnd, GWL_HINSTANCE);	
	_wm._hGroup = CreateWindow(_T("button"), _T("日志"),
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX
		, 0, 0, 0, 0,
		_wm._hWnd, (HMENU)MSG_BOX, _wm._hInst, NULL);

	_wm._hListBox = CreateWindow(_T("listbox"), _T(""),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY|WS_VSCROLL,
		0, 0, 0, 0, _wm._hGroup, (HMENU)LIST_BOX, _wm._hInst, NULL);

	_wm._hListBox2 = CreateWindow(_T("listbox"), _T(""),
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_READONLY|ES_LEFT,
		0, 0, 0, 0, _wm._hWnd, (HMENU)LIST_BOX2, _wm._hInst, NULL);

	_wm._hStatic = CreateWindow(_T("static"), _T("在线用户"),
		WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, _wm._hWnd, 
		(HMENU)STATIC_BOX, _wm._hInst, NULL);

	_wm._hfont = CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
		DEFAULT_CHARSET, 0, 0, 0, FIXED_PITCH, _T("新宋体"));

	SendMessage(_wm._hListBox, WM_SETFONT, (WPARAM)_wm._hfont, NULL);
	SendMessage(_wm._hListBox2, WM_SETFONT, (WPARAM)_wm._hfont, NULL);
	SendMessage(_wm._hGroup, WM_SETFONT, (WPARAM)_wm._hfont, NULL);	
	SendMessage(_wm._hStatic, WM_SETFONT, (WPARAM)_wm._hfont, NULL);
}

void HttpServer::ClientSize(LPARAM lParam)
{
	_wm._cxClient = LOWORD(lParam);
	_wm._cyClient = HIWORD(lParam);
}

void HttpServer::showwindow()
{
	MoveWindow(_wm._hStatic, _wm._cxClient - 240, 30, 80, 20, TRUE);
	MoveWindow(_wm._hGroup, 20, 20, _wm._cxClient-300, _wm._cyClient - 50, TRUE);
	MoveWindow(_wm._hListBox, 10, 30, _wm._cxClient - 320, _wm._cyClient - 90, TRUE);
	MoveWindow(_wm._hListBox2, _wm._cxClient - 240, 50, 200, _wm._cyClient - 75, TRUE);
}

void HttpServer::InitSocket()    //初始化socket
{	
	if (WSAStartup(MAKEWORD(2, 2), &_hm._wsadata) != 0)
		return;

	if (LOBYTE(_hm._wsadata.wVersion) != 2 || HIBYTE(_hm._wsadata.wVersion) != 2)
	{
		WSACleanup();
		return;
	}

	_hm._sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == WSAAsyncSelect(_hm._sockSrv, _wm._hWnd,
		WM_SERVER_SOCKET, FD_ACCEPT | FD_READ))
	{
		MessageBox(_wm._hWnd, _T("自定义消息错误!"), _T("服务器"), MB_OK);
		closesocket(_hm._sockSrv);
		WSACleanup();
		return;
	}

	//本机地址
	_hm._clientAddr.sin_family = AF_INET;
	_hm._clientAddr.sin_port = htons(PORT);
	_hm._clientAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(_hm._sockSrv, (sockaddr*)&_hm._clientAddr, sizeof(sockaddr));	
	if (listen(_hm._sockSrv, MAX_NUMBER) == SOCKET_ERROR)
	{
		SendMessageA(_wm._hListBox, LB_ADDSTRING, 0, (LPARAM)"监听失败!");
		return;
	}
	else
		SendMessageA(_wm._hListBox, LB_ADDSTRING, 0, (LPARAM)"监听成功!");	
}

void HttpServer::socketMsg(LPARAM lParam,WPARAM wParam)
{
	WORD iEvent, iError;
	iEvent = WSAGETSELECTEVENT(lParam);
	iError = WSAGETSELECTERROR(lParam);
	switch (iEvent)
	{
	case FD_ACCEPT:
		if (iError)
		{
			MessageBox(_wm._hWnd, _T("接受失败"), _T("错误"), MB_OK);
			return;
		}

		_hm.AddrLen = sizeof(sockaddr_in);
		_hm._sockClient = accept(_hm._sockSrv, (sockaddr FAR *)&_hm._RemoteAddr, &_hm.AddrLen);
		if (_hm._sockClient == NULL)
		{
			MessageBox(_wm._hWnd, _T("接收失败"), _T("错误"), MB_OK | MB_ICONERROR);
			return;
		}		
				
		if (_userVec.size() <= MAX_NUMBER)
		{
			_users.nSockets = _hm._sockClient;
			if (WSAAsyncSelect(_hm._sockClient, _wm._hWnd, WM_SERVER_SOCKET,
				FD_ACCEPT | FD_READ | FD_CLOSE) == SOCKET_ERROR)
			{
				MessageBox(_wm._hWnd, _T("异步模式创建失败"), _T("错误"),
					MB_OK | MB_ICONERROR);
			}
		}
		break;

	case FD_READ:
		if (iError)
		{
			MessageBox(_wm._hWnd, _T("接收失败"), _T("错误"), MB_OK);
			return;
		}
					
		_hm._RecvBuf = new char[BUF_SIZE]{0};
		_hm._TempBuf = new char[BUF_SIZE]{0};
				
		recv(_hm._sockClient, _hm._RecvBuf, BUF_SIZE, 0);

		if (memchr(_hm._RecvBuf, ' ', strlen(_hm._RecvBuf)) == NULL)   //接收到的是用户名
		{
			_users.User_Name = new char[BUF_SIZE]{0};
			strcpy(_users.User_Name, _hm._RecvBuf);
			_userVec.push_back(_users);

			SendMessageA(_wm._hListBox2, LB_ADDSTRING, 0, (LPARAM)_hm._RecvBuf);
				
			for (int x = 0; x < _userVec.size(); x++)
			{
				for (int y = 0; y < _userVec.size(); y++)
				{
					send(_userVec[x].nSockets, _userVec[y].User_Name, BUF_SIZE, 0);
				}
			}
		}
		else        //接受的是发送的消息
		{		
			for (int x = 0; x < _userVec.size(); x++)
				send(_userVec[x].nSockets, _hm._RecvBuf, BUF_SIZE, 0);
			
			for (int i = 0; i < _userVec.size(); i++)    //呈现消息
			{
				if (_userVec[i].nSockets == wParam)
				{
					strcpy(_hm._TempBuf, _userVec[i].User_Name);
					strcat(_hm._TempBuf, " ");
					strcat(_hm._TempBuf, ShowCurrentTime());
					strtok(_hm._RecvBuf, " ");
					SendMessageA(_wm._hListBox, LB_ADDSTRING, 0, (LPARAM)_hm._TempBuf);
					SendMessageA(_wm._hListBox, LB_ADDSTRING, 0, (LPARAM)strtok(NULL, " "));
				}
			}
		}			
		break;

	case FD_CLOSE:		
		for (int i = 0; i < _userVec.size(); i++)   //删除离开的人
		{
			if (_userVec[i].nSockets == wParam)
			{
				strcpy(_hm._TempBuf, _userVec[i].User_Name);
				_userVec.erase(_userVec.begin() + i);
			}
		}

		SendMessageA(_wm._hListBox2, LB_RESETCONTENT, 0, 0);  //刷新在线用户
		for (int i = 0; i < _userVec.size(); i++)
			SendMessageA(_wm._hListBox2, LB_ADDSTRING, 0,
			(LPARAM)_userVec[i].User_Name);

		strcat(_hm._TempBuf, "离开聊天室...");
		for (int x = 0; x < _userVec.size(); x++)
		{
			send(_userVec[x].nSockets, _hm._TempBuf, BUF_SIZE, 0);			
		}

		SendMessageA(_wm._hListBox, LB_ADDSTRING, 0, (LPARAM)_hm._TempBuf);
		break;
	}	
}

char *HttpServer::ShowCurrentTime()
{
	SYSTEMTIME st;
	char nHours[10] = { 0 };
	char nMinutes[10] = { 0 };
	char nSeconds[10] = { 0 };
	char Buffer[50] = { 0 };

	GetLocalTime(&st);
	sprintf(nHours, "%d", st.wHour);
	sprintf(nMinutes, "%d", st.wMinute);
	sprintf(nSeconds, "%d", st.wSecond);

	strcpy(Buffer, nHours);
	strcat(Buffer, ":");
	strcat(Buffer, nMinutes);
	strcat(Buffer, ":");
	strcat(Buffer, nSeconds);

	return Buffer;
}













