// TCPServer_CE.h: interface for the CTCPServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPSERVER_CE_H__711FE909_4A87_4123_95F8_45160691659D__INCLUDED_)
#define AFX_TCPSERVER_CE_H__711FE909_4A87_4123_95F8_45160691659D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock.h>
#include <map>
#include <string>
#include "../Tools/CustomTools/CustomTools.h"
using namespace std;

class CTCPCustom;
class CTCPServer;

enum E_RESTART_TYPE
{
	E_RESTART_NULL = 0,
	E_RESTART_CORE,				//����Core
	E_RESTART_LOGIC,
	E_RESTART_UPDATE,
	E_RESTART_MASTER,
	E_CLOSE_CORE,				//�ر�Core
	E_CLOSE_LOGIC,
	E_CLOSE_UPDATE,
	E_CLOSE_MASTER,
	E_RESTART_DTUENGINE,
	E_CLOSE_DTUENGINE,
};

enum E_RESTART_REASON
{
	E_REASON_NULL = 0,
	E_REASON_UNFEED_CORE,			//Coreû��ι��
	E_REASON_UNFEED_LOGIC,			//Logicû��ι��
	E_REASON_UNFEED_UPDATE,			//Updateû��ι��
	E_REASON_SUBMIT_CORE,			//Core�����ύ
	E_REASON_SUBMIT_LOGIC,
	E_REASON_SUBMIT_UPDATE,
	E_REASON_UNEXIST_MASTER,		//master�жϲ�����
	E_REASON_FEED,					//����ι��
	E_REASON_THIRD,					//���������ݰ�
	E_REASON_UNFEED_DTUENGINE,		//DTUEngineû��ι��
	E_REASON_SUBMIT_DTUENGINE,			//
};


//����ͻ������ӽ����¼� 
typedef void (CALLBACK* ONCLIENTCONNECT)(void* pOwner,CTCPCustom*); 
//����ͻ���SOCKET�ر��¼� 
typedef void (CALLBACK* ONCLIENTCLOSE)(void* pOwner,CTCPCustom*); 
//����ͻ��˵������ݽ����¼� 
typedef void (CALLBACK* ONCLIENTREAD)(void* pOwner,CTCPCustom*,const char * buf,DWORD dwBufLen ); 
//����ͻ���Socket�����¼� 
typedef void (CALLBACK* ONCLIENTERROR)(void* pOwner,CTCPCustom*,int nErrorCode); 
//�����������Socket�����¼� 
typedef void (CALLBACK* ONSERVERERROR)(void* pOwner,CTCPServer*,int nErrorCode); 
//���������������ʱ��
typedef void (CALLBACK* ONSERVERERESTARTTASK)(void* pOwner,CTCPCustom*,int nRestartType,E_RESTART_REASON eReason); 

//�����������ע��
typedef void (CALLBACK* ONSERVERERREGEDIT)(void* pOwner,CTCPCustom*,E_RESTART_REASON eRegType, char* strProgName); 

class __declspec(dllexport) CTCPServer  
{
public:
    CTCPServer();
    virtual ~CTCPServer();

public:
  int Open(); //��TCP����
  int Close(); //�ر�TCP���� 
  //BOOL SendData(CTCPCustom* pCustomCE, const char * buf , DWORD dwBufLen); 
  //bool	SendData(string strClientMark,const char * buf , DWORD dwBufLen);
  //ɾ���ͻ��˶��� 
  void RemoteClient(CTCPCustom *pClient /*�ͻ��˶���*/); 

  void	RemoteInactiveClient();
  bool	GetExitThread() const;
  CTCPCustom* FindClient(CString strRemoteHost,DWORD nPort);  
  bool	GetAllClientConnectInfo(vector<CString> &vecConnectInfo);

  bool	AckRestart(E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult);
private:
    //�̴߳�����
    static DWORD WINAPI SocketThreadFunc(PVOID lparam);

	//�رճ�ʱ��δ�յ����ݵ�����
	static DWORD WINAPI CloseThreadFunc(PVOID lparam);
public:
    int m_LocalPort; //���÷���˿ں�
	void * m_pOwner;   //�������� 


	//�洢�ͻ���Socket���
	CTCPCustom FindClient(SOCKET m_client);
    CWnd * m_pOwnerWnd;   //�����ھ��
	bool	m_bexitthread;
	Mutex	m_utex;
	Project::Tools::Mutex m_lock;		//�Ƴ��ͻ��˼���
private:
    SOCKET m_ServerSocket;     //TCP�������socket
    HANDLE m_serverThreadHandle;  //ͨѶ�߳̾��
	HANDLE m_serverCloseHandle;  //�߳̾��
    HANDLE m_exitThreadEvent;  //ͨѶ�߳��˳��¼����

public:  //�����¼�
	//�ͻ������ӽ����¼����ص����� 
	ONCLIENTCONNECT    OnClientConnect; 
	//�ͻ������ӶϿ��¼����ص����� 
	ONCLIENTCLOSE OnClientClose; 
	//�ͻ��˽��������¼����ص����� 
	ONCLIENTREAD       OnClientRead; 
	//�ͻ��˷��������¼����ص����� 
	ONCLIENTERROR      OnClientError; 
	//�������˷��������¼�,�ص����� 
	ONSERVERERROR	   OnServerError; 

	ONSERVERERESTARTTASK	   OnServerRestartTask; 

	ONSERVERERREGEDIT	OnClientRegedit;
};

#endif // !defined(AFX_TCPSERVER_CE_H__711FE909_4A87_4123_95F8_45160691659D__INCLUDED_)
