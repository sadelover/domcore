// TCPSocket.h: interface for the CTCPSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(TCPSOCKET)
#define TCPSOCKET

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "winsock2.h"
#include <string>

using namespace std;

//在这里支持多种平台
#ifdef UNDER_CE
# pragma comment(lib,"winsock.lib")
#else
# pragma comment(lib,"ws2_32.lib")
#endif

#define SD_RECEIVE 0x00
#define SD_SEND 0x01
#define SD_BOTH 0x02

#define MAX_CONNECTION 10	//最大的连接数
//Socket类型
enum  _declspec(dllexport) TCP_SOCKET_TYPE
{
	TCP_SOCKET_SERVER=0,
	TCP_SOCKET_CLIENT
};

enum _declspec(dllexport) TCP_CONNECT_STATE
{
	TCP_STATE_CONNECT=0,  //连接
	TCP_STATE_BAD_DISCONNECT,	//出错断开
	TCP_STATE_GOOD_DISCONNECT		//正常断开
};

enum _declspec(dllexport) TCP_DATA_TYPE
{
	TCP_DATA_SEND = 0,
	TCP_DATA_RECEIVE
};

typedef LRESULT (*LPStatusProc)(TCP_CONNECT_STATE type,char *data,int nNo,DWORD userdata); //服务器状态的回调函数
typedef LRESULT (*LPDataArriveProc)(char *data,int length,DWORD userdata); //数据到达的回调函数
typedef LRESULT (*ServerDataArriveProc)(int nNO,char *ip,char *data,int length,DWORD userdata); //数据到达的回调函数

//Socket超时参数结构体
struct TimeOutParameter
{
	int EndTime;
	SOCKET s;
	int nNo;
	BOOL bFinished;
	BOOL bExit;
	BOOL* pbConnected;
	HANDLE* phDataThread;
	int* pnConnections;
};

class __declspec(dllexport) CTCPSocket  
{
public:
	CTCPSocket(int nType=TCP_SOCKET_CLIENT);
	virtual ~CTCPSocket();

	//变量
	int error; //错误类型

	//函数
	int GetError(); //取得错误
	SOCKET GetSocket(); //取得套接字
	int GetType(); //取得类型
	void SetType(int nType);
	BOOL IsConnected(SOCKET s); //判断一个socket是否连接

	BOOL CreateServer(int nPort,int backlog=5); //建立服务器
	BOOL StartServer(LPStatusProc proc1=NULL,ServerDataArriveProc proc2=NULL,DWORD userdata=NULL); //开始服务
	BOOL StopServer(); //停止服务
	SOCKET Listen(char* ClientIP=NULL); //监听单个IP的连接
	int ReceiveServer(int nNo,char* data, int length,int timeout); //接收指定字节的数据
	int SendServer(int nNo,char* data, int length); //发送指定字节的数据
	void Disconnect(int nNo);

	BOOL Connect(CString pstrHost, int nPort); //连接一个IP
	BOOL Connect(string pstrHost, int nPort); //连接一个IP
	BOOL StartReceiving(LPStatusProc proc1=NULL,LPDataArriveProc proc2=NULL,DWORD userdata=NULL); //开始自动接收
	BOOL StopReceiving(); //停止自动接收	
	int ReceiveClient(char* data, int length,int timeout); //接收指定字节的数据
	int SendClient(char* data, int length); //发送指定字节的数据

	void Disconnect(); //关闭	

	std::string   ConvertWideCharToMultiByte(const std::wstring& wstrSrc);

protected:
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length) =0;
//protected:
public:
	//变量
	int m_nType; //类型
	SOCKET m_sSocket; //套接字
	BOOL m_bAvailable; //能否使用
	BOOL m_bCreated; //是否建立,就是CreateServer和Connect之后的状态
	BOOL m_bAuto; //是否自动收发,就是StartServer和StartReceiving之后的状态
	DWORD m_dwUserData; //用户数据

	int m_nPort; //服务器端口
	HANDLE m_hServerThread; //服务器监听连接的线程
	HANDLE m_hServerDataThread[MAX_CONNECTION]; //服务器数据收发的线程
	SOCKET m_sServer[MAX_CONNECTION]; //每个客户的连接,初期为了方便给连接回送才这样设置
	char m_cIp[MAX_CONNECTION][16]; //每个连接的IP
	BOOL m_bConnected[MAX_CONNECTION]; //每个能够使用的连接的状态
	int m_nConnections; //连接总数
	int m_nCurrent; //当前正要建立的连接
	ServerDataArriveProc m_lpServerDataArriveProc; //服务器数据收发回调
	LPStatusProc m_lpServerStatusProc; //服务器状态回复回调

	HANDLE m_hClientThread; //客户端数据收发的线程
	LPDataArriveProc m_lpClientDataArriveProc; //客户端数据收发回调
	LPStatusProc m_lpClientStatusProc; //客户端状态回复回调

	//函数
	BOOL Initwinsock();
	BOOL NewConnect(int nNo);
	static DWORD WINAPI ServerThread(LPVOID lpParmameter); //服务器监听线程
	static DWORD WINAPI DataThread(LPVOID lpParameter); //数据收发线程

	static DWORD WINAPI ClientThread(LPVOID lpParameter); //客户端接收线程

	static DWORD WINAPI TimeOutControl(LPVOID lpParameter); //超时控制线程

};

#endif
