#include "stdafx.h"
#include "Http.h"

Http::Http()
{
	_hm._socketClient = NULL;
	memset(&_hm._wsadata, 0, sizeof(WSADATA));	
	memset(&_hm._socketAddr, 0, sizeof(sockaddr_in));

	_wm.hfont = NULL;
	_wm.hInst = NULL;	
	_wm.hWnd = _wm.hEdit
		= _wm.hGroup = _wm.hListBox
		= _wm.hStatic = _wm.hListBox2
		= _wm.hLaunchBtn = _wm.hSendBtn = NULL;
	_wm.cxClient = _wm.cyClient = 0;

	userInfo.clear();
	Name.clear();
}

Http::~Http()
{
	closesocket(_hm._socketClient);
	WSACleanup();
	DeleteObject(_wm.hfont);	

	delete[]_user.UserName;
	delete[]_user.Password;
	delete[]_user.localAddr;

	delete[]_hm._RecvBuf;
	delete[]_hm._SendBuf;
	delete[]_hm._TempBuf;
}

void Http::CreateWin()
{
	_wm.hInst = (HINSTANCE)GetWindowLong(_wm.hWnd, GWL_HINSTANCE);

	_wm.hLaunchBtn = CreateWindow(_T("button"), _T("关闭"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 0, 0, 0,
		_wm.hWnd, (HMENU)CLOSE_BTN, _wm.hInst, NULL);

	_wm.hSendBtn = CreateWindow(_T("button"), _T("发送"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0, 0, 0, 0,
		_wm.hWnd, (HMENU)SEND_BTN, _wm.hInst, NULL);

	_wm.hGroup = CreateWindow(_T("button"), _T("日志"),
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 0, 0, 0, 0,
		_wm.hWnd, (HMENU)GROUP_BOX, _wm.hInst, NULL);

	_wm.hListBox = CreateWindow(_T("listbox"), _T(""),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY |
		ES_LEFT | WS_VSCROLL, 0, 0, 0, 0,
		_wm.hGroup, (HMENU)LIST_BOX, _wm.hInst, NULL);

	_wm.hListBox2 = CreateWindow(_T("listbox"), _T(""),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_LEFT | WS_VSCROLL
		, 0, 0, 0, 0,
		_wm.hWnd, (HMENU)LIST_BOX2, _wm.hInst, NULL);

	_wm.hEdit = CreateWindow(_T("edit"), _T(""),
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE,
		0, 0, 0, 0,
		_wm.hWnd, (HMENU)EDIT_TEXT, _wm.hInst, NULL);	

	_wm.hStatic = CreateWindow(_T("static"), _T("在线用户"),
		WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, _wm.hWnd,
		(HMENU)STATIC_BOX, _wm.hInst, NULL);

	_wm.hfont = CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
		DEFAULT_CHARSET, 0, 0, 0, FIXED_PITCH, _T("新宋体"));

	SendMessage(_wm.hEdit, WM_SETFONT, (WPARAM)_wm.hfont, NULL);
	SendMessage(_wm.hListBox, WM_SETFONT, (WPARAM)_wm.hfont, NULL);
	SendMessage(_wm.hLaunchBtn, WM_SETFONT, (WPARAM)_wm.hfont, NULL);
	SendMessage(_wm.hSendBtn, WM_SETFONT, (WPARAM)_wm.hfont, NULL);
	SendMessage(_wm.hStatic, WM_SETFONT, (WPARAM)_wm.hfont, NULL);
	SendMessage(_wm.hListBox2, WM_SETFONT, (WPARAM)_wm.hfont, NULL);
	SendMessage(_wm.hGroup, WM_SETFONT, (WPARAM)_wm.hfont, NULL);
}

void Http::ClientSize(LPARAM lParam)
{
	_wm.cxClient = LOWORD(lParam);
	_wm.cyClient = HIWORD(lParam);
}

void Http::ShowWin()
{
	MoveWindow(_wm.hGroup, 20, 20, _wm.cxClient - 280, _wm.cyClient - 180, TRUE);
	MoveWindow(_wm.hListBox, 10, 20, _wm.cxClient-300, _wm.cyClient - 200, TRUE);	
	MoveWindow(_wm.hEdit, 30, _wm.cyClient - 140, _wm.cxClient - 300, 70, TRUE);	
	MoveWindow(_wm.hStatic, _wm.cxClient-230, 10, 80, 20, TRUE);
	MoveWindow(_wm.hListBox2, _wm.cxClient-230, 35, 200, _wm.cyClient - 60, TRUE);	
	MoveWindow(_wm.hSendBtn, (_wm.cxClient - 250) / 2 - 80, _wm.cyClient - 50, 80, 30, TRUE);
	MoveWindow(_wm.hLaunchBtn, (_wm.cxClient - 250) / 2 + 40, _wm.cyClient - 50, 80, 30, TRUE);	
}

char *Http::ShowCurrentTime()
{
	SYSTEMTIME st;
	char nHours[10] = { 0 };
	char nMinutes[10] = { 0 };
	char nSeconds[10] = { 0 };
	char Buffer[100] = { 0 };

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

void Http::CommMsg(WPARAM wParam)
{
	switch (LOWORD(wParam))
	{
	case SEND_BTN:
		_hm._SendBuf = new char[BUF_SIZE];
		_hm._TempBuf = new char[BUF_SIZE];

		GetWindowTextA(_wm.hEdit, _hm._SendBuf, BUF_SIZE);

		if (GetWindowTextLengthA(_wm.hEdit) == 0)
			MessageBoxA(_wm.hWnd, "不能发送空消息", "错误", MB_OK);
		else
		{	
			strcpy(_hm._TempBuf, _user.UserName);
			strcat(_hm._TempBuf, " ");
			strcat(_hm._TempBuf, _hm._SendBuf);
			
			send(_hm._socketClient, _hm._TempBuf, strlen(_hm._TempBuf), 0);
		}		
		break;

	case CLOSE_BTN:
		DestroyWindow(_wm.hWnd);
		PostQuitMessage(0);
		
		break;

	case IDM_ABOUT:
		DialogBox(_wm.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), _wm.hWnd, About);
		break;

	case IDM_EXIT:
		DestroyWindow(_wm.hWnd);
		break;
	}
}

void Http::InitSocket()
{
	if (WSAStartup(MAKEWORD(2, 2), &_hm._wsadata) != 0)
		return;

	if (LOBYTE(_hm._wsadata.wVersion) != 2 || HIBYTE(_hm._wsadata.wVersion) != 2)
	{
		WSACleanup();
		return;
	}

	_hm._socketClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == WSAAsyncSelect(_hm._socketClient, _wm.hWnd,
		WM_CLIENT_SOCKET, FD_CONNECT | FD_READ | FD_CLOSE))
	{
		MessageBox(_wm.hWnd, _T("WSAAyncSelect错误!"), _T("客户端"), MB_OK);
		closesocket(_hm._socketClient);
		WSACleanup();
	}

	_hm._socketAddr.sin_family = AF_INET;
	_hm._socketAddr.sin_port = htons(PORT);
	_hm._socketAddr.sin_addr.S_un.S_addr = inet_addr(_user.localAddr);

	if ((connect(_hm._socketClient, (sockaddr*)&_hm._socketAddr,
		sizeof(_hm._socketAddr))) != SOCKET_ERROR)
	{
		SendMessageA(_wm.hListBox, LB_ADDSTRING, 0, (LPARAM)"连接成功!");
	}
}

void Http::jsonDataWrite()      //用户数据写入文件
{
	Users Mem;
	Json::Value Item;
	Json::Value Root;
	Json::Value Array;
	Json::StyledWriter writer;

	for (int i = 0; i < userInfo.size(); i++)
	{
		Item["用户名"] = userInfo[i].UserName;
		Item["密码"] = userInfo[i].Password;
		Item["本机地址"] = userInfo[i].localAddr;
		Array.append(Item);
	}

	Root["用户信息"] = Array;

	ofstream ofs;
	ofs.open("UsersData", ios::out);
	ofs << writer.write(Root);   //写入文件
	ofs.close();
}

void Http::jsonDataRead()       //读取文件用户数据
{
	Json::Reader reader;
	Json::Value Root;
	Users GetInfo;

	ifstream ifs;
	ifs.open("UsersData");

	if (ifs.is_open())    //文件存在
	{
		if (reader.parse(ifs, Root))
		{
			for (int i = 0; i < Root["用户信息"].size(); i++)
			{
				_user.UserName = new char[BUF_SIZE];
				_user.Password = new char[BUF_SIZE];
				_user.localAddr = new char[BUF_SIZE];

				strcpy(_user.UserName, Root["用户信息"][i]["用户名"].asString().c_str());
				strcpy(_user.Password, Root["用户信息"][i]["密码"].asString().c_str());
				strcpy(_user.localAddr, Root["用户信息"][i]["本机地址"].asString().c_str());

				userInfo.push_back(_user);      //文件信息全部保存在vector中
			}
		}
	}

	ifs.close();
}

void Http::socketMsg(LPARAM lParam)
{
	WORD iEvent, iError;
	iEvent = WSAGETSELECTEVENT(lParam);
	iError = WSAGETSELECTERROR(lParam);
	switch (iEvent)
	{
	case FD_CONNECT:
		if (iError)
		{
			SendMessageA(_wm.hListBox, LB_ADDSTRING, 0, (LPARAM)"连接失败!");
			return;
		}
		else
		{
			SendMessageA(_wm.hListBox, LB_ADDSTRING, 0, (LPARAM)"连接成功!");
			send(_hm._socketClient, _user.UserName,
				strlen(_user.UserName), 0);
		}
		break;

	case FD_READ:
		if (iError)
		{
			MessageBox(_wm.hWnd, _T("接收失败!"), _T("客户端"), MB_OK);
			return;
		}

		_hm._RecvBuf = new char[BUF_SIZE]{0};
		_hm._TempBuf = new char[BUF_SIZE]{0};

		recv(_hm._socketClient, _hm._RecvBuf, BUF_SIZE, 0);	

		if (memchr(_hm._RecvBuf, ' ', strlen(_hm._RecvBuf)) == NULL)   //接收到的是用户名
		{
			if (!IfHasName(_hm._RecvBuf))
				Name.push_back(_hm._RecvBuf);
			
			SendMessageA(_wm.hListBox2, LB_RESETCONTENT, 0, 0);
			for (int i = 0; i < Name.size(); i++)
				SendMessageA(_wm.hListBox2, LB_ADDSTRING, 0, (LPARAM)Name[i]);
		}
		else  //消息内容
		{
			strcpy(_hm._TempBuf, strtok(_hm._RecvBuf, " "));
			strcat(_hm._TempBuf, " ");
			strcat(_hm._TempBuf, ShowCurrentTime());
			SendMessageA(_wm.hListBox, LB_ADDSTRING, 0, 
				(LPARAM)_hm._TempBuf);
			SendMessageA(_wm.hListBox, LB_ADDSTRING, 0,
				(LPARAM)strtok(NULL, " "));

		}

		if (strstr(_hm._RecvBuf, "离开聊天室...") != NULL)
		{
			char* Buffer;
			Buffer = new char[50];
			strcpy(Buffer, strtok(_hm._RecvBuf, "离开聊天室..."));
			
			for (int i = 0; i < Name.size(); i++)
			{
				if (strcmp(Name[i], Buffer) == 0)
				{
					Name.erase(Name.begin() + i);
				}
			}

			SendMessageA(_wm.hListBox2, LB_RESETCONTENT, 0, 0);
			for (int i = 0; i < Name.size(); i++)
				SendMessageA(_wm.hListBox2, LB_ADDSTRING, 0,
				(LPARAM)Name[i]);
		}

		break;

	
	}
}

bool Http::IfHasName(char * pName)
{
	for (int i = 0; i < Name.size(); i++)
	{
		if (strcmp(Name[i], pName) == 0)
			return true;
	}
	return false;
}

bool Http::IfhasUser(char * pUserName, char *pPassWord)   //判断用户是否存在
{
	if (userInfo.size() > 0)
	{
		for (int i = 0; i < userInfo.size(); i++)
		{
			if (strcmp(pUserName, userInfo[i].UserName) == 0 &&    //用户存在
				strcmp(pPassWord, userInfo[i].Password) == 0)							
				return true;			
		}
	}	
	return false;
}

void Http::DlgMsg(HWND hWnd,HWND hName, HWND hPassword,HWND hLocalIP,WPARAM wParam)
{
	char User[50] = { 0 };
	char Password[50] = { 0 };
	char IPstr[50] = { 0 };

	switch (LOWORD(wParam))
	{
	case IDC_BUTTON1:
		_user.UserName = new char[BUF_SIZE];
		_user.Password = new char[BUF_SIZE];
		_user.localAddr = new char[BUF_SIZE];

		if (GetWindowTextLengthA(hName) != 0
			&& GetWindowTextLengthA(hPassword) != 0
			&& GetWindowTextLengthA(hLocalIP) != 0)
		{
			GetWindowTextA(hName, User, 50);
			GetWindowTextA(hPassword, Password, 50);
			GetWindowTextA(hLocalIP, IPstr, 50);

			if (IfhasUser(User, Password))
			{
				strcpy(_user.UserName, User);
				strcpy(_user.Password, Password);
				strcpy(_user.localAddr, IPstr);

				InitSocket();
				EndDialog(hWnd, 0);
				ShowWindow(_wm.hWnd, SW_NORMAL);
				UpdateWindow(_wm.hWnd);
			}
			else
				MessageBox(_wm.hWnd, _T("用户名或密码或IP地址错误"), _T("错误"),
				MB_OK | MB_ICONERROR);
		}
		else
			MessageBox(_wm.hWnd, _T("用户名或密码不能为空"),
			_T("错误"), MB_OK | MB_ICONERROR);
		break;

	case IDC_BUTTON2:
		DialogBox(_wm.hInst, MAKEINTRESOURCE(IDD_DIALOG2), _wm.hWnd, Dlg2);
		break;
	}
}


void Http::DlgMsg2(HWND hWnd,HWND hDlg, HWND hName, HWND hPassword,HWND hLocalIP, WPARAM wParam)
{
	char User[50] = { 0 };
	char Password[50] = { 0 };
	char IPstr[50] = { 0 };

	switch (LOWORD(wParam))
	{
	case IDOK:
		_user.UserName = new char[BUF_SIZE];
		_user.Password = new char[BUF_SIZE];
		_user.localAddr = new char[BUF_SIZE];

		if (GetWindowTextLengthA(hName) != 0
			&& GetWindowTextLengthA(hPassword) != 0
			&& GetWindowTextLengthA(hLocalIP) != 0)
		{
			GetWindowTextA(hName, User, 50);
			GetWindowTextA(hPassword, Password, 50);
			GetWindowTextA(hLocalIP, IPstr, 50);

			if (!IfhasUser(User, Password))
			{
				strcpy(_user.UserName, User);
				strcpy(_user.Password, Password);
				strcpy(_user.localAddr, IPstr);

				userInfo.push_back(_user);
				jsonDataWrite();

				InitSocket();
				EndDialog(hWnd, 0);
				EndDialog(hDlg, 0);
				ShowWindow(_wm.hWnd, SW_NORMAL);
				UpdateWindow(_wm.hWnd);
			}
			else
				MessageBox(_wm.hWnd, _T("用户已经存在，请登录"), _T("提示"),
				MB_OK | MB_ICONERROR);
		}
		else
			MessageBox(_wm.hWnd, _T("用户名或密码不能为空"),
			_T("错误"), MB_OK | MB_ICONERROR);
		break;

	case IDCANCEL:
		EndDialog(hWnd, 0);
		break;
	}  
}







