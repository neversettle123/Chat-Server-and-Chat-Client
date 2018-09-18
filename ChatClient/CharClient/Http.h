#pragma once
#include<windows.h>
#include<winsock2.h>
#include<vector>
#include"json.h"
#include<fstream>
#include<string.h>
#include"resource.h"
#include"ChatClient.h"
#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define LIST_BOX 200
#define EDIT_TEXT 201
#define CLOSE_BTN 202
#define SEND_BTN 203
#define LIST_BOX2 204
#define STATIC_BOX 205
#define GROUP_BOX 206
#define WM_CLIENT_SOCKET 5000

#define PORT 6000
#define BUF_SIZE 1024

typedef struct HttpMember
{
	WSADATA _wsadata;
	SOCKET _socketClient;
	sockaddr_in _socketAddr;

	char * _SendBuf, *_RecvBuf;
	char *_TempBuf;
};

typedef struct WndMember
{
	HFONT hfont;
	HINSTANCE hInst;	
	HWND hWnd, hEdit,
		hGroup, hListBox,
		hStatic, hListBox2,
		hLaunchBtn, hSendBtn;	
	int cxClient, cyClient;	
};

typedef struct Users
{	
	char *UserName, *Password, *localAddr;
};

class Http
{
private:
	HttpMember _hm;
	WndMember _wm;
	Users _user;    //��¼�û���Ϣ
	vector<Users>userInfo;   //���ڱ���ע���û���Ϣ
	vector<char*>Name;  //���������û���

public:
	Http();
	~Http();	
	void CreateWin();
	void ShowWin();
	void CommMsg(WPARAM wParam);      //������Ϣ
	void ClientSize(LPARAM lParam);
	void GethWnd(HWND hWnd){ _wm.hWnd = hWnd; }     //��ȡ�����ھ��

public:	
	void InitSocket();     //��ʼ��socket
	void jsonDataRead();            //���浽json�ļ�
	void jsonDataWrite();           //��ȡjson�ļ�

	char * ShowCurrentTime();                //����ʱ��
	void socketMsg(LPARAM lParam);   //�Զ�����Ϣ
	bool IfhasUser(char * pUserName, char *pPassWord);      //�û��Ƿ����
	bool IfHasName(char* pName);

public:
	void DlgMsg(HWND hWnd, HWND hName, HWND hPassword,HWND hLocalIP, WPARAM wParam);  //��¼�Ի���
	void DlgMsg2(HWND hWnd, HWND hDlg, HWND hName, HWND hPassword,HWND hLocalIP, WPARAM wParam);  //ע��Ի���
};




