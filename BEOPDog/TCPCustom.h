// TCPCustom.h: interface for the CTCPCustom class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPCUSTOM_CE_H__0E8B4A18_8A99_438E_B5F6_B5985FFC117D__INCLUDED_)
#define AFX_TCPCUSTOM_CE_H__0E8B4A18_8A99_438E_B5F6_B5985FFC117D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock.h>
#include "TCPServer.h"
#include <queue>
#include "ProcessView.h"

const int g_nRecMaxNum = 8192;	  //4096*2
const int g_nRecAveNum = 4096;    //4096
const int g_nPackageSize = 4096;	//ÿ�����ݰ�����

struct datapackage				//�ְ�һ����
{
	datapackage()
	{
		nLength = 0;
		memset(cData,0,g_nPackageSize);
	}
	char   cData[g_nPackageSize];				//����
	int	   nLength;					//����
};

class __declspec(dllexport) CTCPCustom  
{
public:
    CTCPCustom();
    virtual ~CTCPCustom();
public:
    CTCPServer* m_pTCPServer;		//����TCP����˼���Socket
    CString		m_RemoteHost;		//Զ������IP��ַ
    DWORD		m_RemotePort;		//Զ�������˿ں�
    SOCKET		m_socket;			//ͨѶSocket���
	COleDateTime m_oleLastReceive;	//��һ���յ�����ʱ��
	char		m_cRecvBuf[g_nRecMaxNum];
	int			m_nRecSize;
	char		m_cSpiltBuffer[g_nRecMaxNum];			//���ݽ�������
	bool		m_bOK;				//�ɹ���־
	bool		m_bOutRec;			//�������LOG
	bool		m_bexitthread;
	CProcessView		m_PrsV;
	E_RESTART_REASON		m_eType;	//�ͻ����
	string	m_strThirdName;				//��������������
	Project::Tools::Mutex m_lock;
	Project::Tools::Mutex m_reveivelock;
	Project::Tools::Mutex m_sendlock;
	CRITICAL_SECTION m_csTCPSocketSync;			//�ٽ������� ����Socket��Դ

	queue<datapackage>	m_queDataPaket;					//�洢���ݰ�
	CRITICAL_SECTION m_csDTUDataSync;			//�ٽ������� ����������Դ
	HANDLE m_event;
private:
    HANDLE m_exitThreadEvent;  //ͨѶ�߳��˳��¼����
    HANDLE m_tcpThreadHandle;  //ͨѶ�߳̾��
	HANDLE m_datathread;
private:
    //ͨѶ�̺߳���
    static DWORD WINAPI SocketThreadFunc(PVOID lparam);
	static UINT WINAPI ThreadFunc(LPVOID lparam);				//�����յ�����
public:
    //��socket������ͨѶ�߳�
    bool Open(CTCPServer *pTCPServer);
    
    //�ر�socket���ر��̣߳��ͷ�Socket��Դ
    bool Close();

    //��ͻ��˷�������
    bool  SendData(const char * buf , int len);
	
	bool	IsTimeOut(int nSeconds = 600);		//10����δ�յ�����ʱ��
	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<datapackage>& resultlist);		//����;\n����
	bool	HandleRecData(const char* buffer,int nSize);

	bool	StoreRecData(const char* buffer,int nSize);
	bool	AnalyzeReceive(const char* pRevData, DWORD length);	
	void	SetDataEvent();
	void	HandleData();

	bool	AckReg();
	bool	SendAck(E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult);
	bool	SendAck(int nCmdType,int nResult,int nRestartType = -1);	// nCmdType 0:������ 1������Core 2������Watch    nResult 0:ʧ�� nRestartType:������ʽ
};

#endif // !defined(AFX_TCPCUSTOM_CE_H__0E8B4A18_8A99_438E_B5F6_B5985FFC117D__INCLUDED_)
