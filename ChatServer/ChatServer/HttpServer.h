#pragma once
#include<windows.h>
#include<winsock2.h>
#include<vector>
#include<string.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define LIST_BOX 200
#define EDIT_CTL 203
#define MSG_BOX  204
#define STATIC_BOX 205
#define LIST_BOX2 206

#define WM_SERVER_SOCKET 5001
#define PORT 6000
#define BUF_SIZE 1024
#define MAX_NUMBER 20

typedef struct HttpMem
{	
	int AddrLen;
	WSADATA _wsadata;
	SOCKET _sockClient, _sockSrv;	
	sockaddr_in _clientAddr, _RemoteAddr;	
	char *_RecvBuf, *_TempBuf;
};

typedef struct WndMem
{
	HFONT _hfont;
	HINSTANCE _hInst;		
	HWND _hWnd, _hListBox,_hGroup,
		_hStatic, _hListBox2;
	int _cxClient, _cyClient;
};

typedef struct UserInfo
{
	char *User_Name;
	SOCKET nSockets;
};

class HttpServer
{
private:
	HttpMem _hm;
	WndMem _wm;
	UserInfo _users;   
	vector<UserInfo>_userVec;

public:
	HttpServer();
	~HttpServer();

	void GethWnd(HWND hWnd)
	{ _wm._hWnd = hWnd; }  //获取主窗口句柄
	void ClientSize(LPARAM lParam);  
	void createwindow();
	void showwindow();	
	void InitSocket();	  //初始化socket

public:	
	char * ShowCurrentTime();
	void socketMsg(LPARAM lParam,WPARAM wParam);   //自定义消息
};
