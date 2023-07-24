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

//������֧�ֶ���ƽ̨
#ifdef UNDER_CE
# pragma comment(lib,"winsock.lib")
#else
# pragma comment(lib,"ws2_32.lib")
#endif

#define SD_RECEIVE 0x00
#define SD_SEND 0x01
#define SD_BOTH 0x02

#define MAX_CONNECTION 10	//����������
//Socket����
enum  _declspec(dllexport) TCP_SOCKET_TYPE
{
	TCP_SOCKET_SERVER=0,
	TCP_SOCKET_CLIENT
};

enum _declspec(dllexport) TCP_CONNECT_STATE
{
	TCP_STATE_CONNECT=0,  //����
	TCP_STATE_BAD_DISCONNECT,	//����Ͽ�
	TCP_STATE_GOOD_DISCONNECT		//�����Ͽ�
};

enum _declspec(dllexport) TCP_DATA_TYPE
{
	TCP_DATA_SEND = 0,
	TCP_DATA_RECEIVE
};

typedef LRESULT (*LPStatusProc)(TCP_CONNECT_STATE type,char *data,int nNo,DWORD userdata); //������״̬�Ļص�����
typedef LRESULT (*LPDataArriveProc)(char *data,int length,DWORD userdata); //���ݵ���Ļص�����
typedef LRESULT (*ServerDataArriveProc)(int nNO,char *ip,char *data,int length,DWORD userdata); //���ݵ���Ļص�����

//Socket��ʱ�����ṹ��
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

	//����
	int error; //��������

	//����
	int GetError(); //ȡ�ô���
	SOCKET GetSocket(); //ȡ���׽���
	int GetType(); //ȡ������
	void SetType(int nType);
	BOOL IsConnected(SOCKET s); //�ж�һ��socket�Ƿ�����

	BOOL CreateServer(int nPort,int backlog=5); //����������
	BOOL StartServer(LPStatusProc proc1=NULL,ServerDataArriveProc proc2=NULL,DWORD userdata=NULL); //��ʼ����
	BOOL StopServer(); //ֹͣ����
	SOCKET Listen(char* ClientIP=NULL); //��������IP������
	int ReceiveServer(int nNo,char* data, int length,int timeout); //����ָ���ֽڵ�����
	int SendServer(int nNo,char* data, int length); //����ָ���ֽڵ�����
	void Disconnect(int nNo);

	BOOL Connect(CString pstrHost, int nPort); //����һ��IP
	BOOL Connect(string pstrHost, int nPort); //����һ��IP
	BOOL StartReceiving(LPStatusProc proc1=NULL,LPDataArriveProc proc2=NULL,DWORD userdata=NULL); //��ʼ�Զ�����
	BOOL StopReceiving(); //ֹͣ�Զ�����	
	int ReceiveClient(char* data, int length,int timeout); //����ָ���ֽڵ�����
	int SendClient(char* data, int length); //����ָ���ֽڵ�����

	void Disconnect(); //�ر�	

	std::string   ConvertWideCharToMultiByte(const std::wstring& wstrSrc);

protected:
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length) =0;
//protected:
public:
	//����
	int m_nType; //����
	SOCKET m_sSocket; //�׽���
	BOOL m_bAvailable; //�ܷ�ʹ��
	BOOL m_bCreated; //�Ƿ���,����CreateServer��Connect֮���״̬
	BOOL m_bAuto; //�Ƿ��Զ��շ�,����StartServer��StartReceiving֮���״̬
	DWORD m_dwUserData; //�û�����

	int m_nPort; //�������˿�
	HANDLE m_hServerThread; //�������������ӵ��߳�
	HANDLE m_hServerDataThread[MAX_CONNECTION]; //�����������շ����߳�
	SOCKET m_sServer[MAX_CONNECTION]; //ÿ���ͻ�������,����Ϊ�˷�������ӻ��Ͳ���������
	char m_cIp[MAX_CONNECTION][16]; //ÿ�����ӵ�IP
	BOOL m_bConnected[MAX_CONNECTION]; //ÿ���ܹ�ʹ�õ����ӵ�״̬
	int m_nConnections; //��������
	int m_nCurrent; //��ǰ��Ҫ����������
	ServerDataArriveProc m_lpServerDataArriveProc; //�����������շ��ص�
	LPStatusProc m_lpServerStatusProc; //������״̬�ظ��ص�

	HANDLE m_hClientThread; //�ͻ��������շ����߳�
	LPDataArriveProc m_lpClientDataArriveProc; //�ͻ��������շ��ص�
	LPStatusProc m_lpClientStatusProc; //�ͻ���״̬�ظ��ص�

	//����
	BOOL Initwinsock();
	BOOL NewConnect(int nNo);
	static DWORD WINAPI ServerThread(LPVOID lpParmameter); //�����������߳�
	static DWORD WINAPI DataThread(LPVOID lpParameter); //�����շ��߳�

	static DWORD WINAPI ClientThread(LPVOID lpParameter); //�ͻ��˽����߳�

	static DWORD WINAPI TimeOutControl(LPVOID lpParameter); //��ʱ�����߳�

};

#endif
