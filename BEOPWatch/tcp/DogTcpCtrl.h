#pragma once

#include "TcpClient.h"
#include "Tools/CustomTools/CustomTools.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

const int g_nDogRecMaxNum = 8192;
const int g_nDogPackageSize = 4096;	//ÿ�����ݰ�����

struct dogpackage				//�ְ�һ����
{
	dogpackage()
	{
		nLength = 0;
		memset(cData,0,g_nDogPackageSize);
	}
	char   cData[g_nDogPackageSize];				//����
	int	   nLength;					//����
};

enum E_SLAVE_MODE
{
	E_SLAVE_CORE =1,
	E_SLAVE_LOGIC,
	E_SLAVE_UPDATE,
	E_SLAVE_MASTER,
	E_SLAVE_DTUENGINE,
};

enum E_SLAVE_REASON
{
	E_REASON_NULL = 0,
	E_REASON_REMOTE,		//Զ������
	E_REASON_UPDATE,		//��������

};

class CDogTcpCtrl : public CTcpClient
{
public:
	CDogTcpCtrl();
	virtual ~CDogTcpCtrl();

protected:
	//����ͨѶ�����İ�.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:

	static CDogTcpCtrl* GetInstance(bool bConnected = true);  // get the singleton instance of assign object
	static void DestroyInstance();                   // destroy the singleton instance of assign objec

	//Initialize Tcp Info
	bool	CloseTcp();
	bool	Init(bool bConnected = true);
	bool	Exit();
	bool	GetConnectSuccess();
	bool	GetDogRestartFunctonOK();				//�汾��֧�� ���ӳɹ�
	bool	SetDogMode(E_SLAVE_MODE eMode);				
	bool	SendActiveSignal();
	bool	SendRestart(int nRestartExe, int nRestartType=-1);		//���� 1��Core  2:Watch
	bool	SendRestart(E_SLAVE_MODE nRestartExe, E_SLAVE_REASON nReason);	

	bool	GetRestartSuccess(bool& bRestartCore,bool& bRestartWatch);
	bool	GetRestartSuccess(bool& bRestartCore,bool& bRestartLogic,bool& bRestartUpdate,bool& bRestartMaster);
	bool	SetRestartCoreSuccess(bool bRestartCore);
	bool	SetRestartLogicSuccess(bool bRestartLogic);
	bool	SetRestartUpdateSuccess(bool bRestartUpdate);
	bool	SetRestartMasterSuccess(bool bRestartMaster);

	bool	SetRestartWatchSuccess(bool bRestartWatch);

protected:
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	bool	ReConnect();
	void    ClearNetworkError();

private:
	bool	AnalyzeReceive(const unsigned char* pRevData, DWORD length);	
	bool	WriteToPort(char *pData,int nLen);
	bool	WriteToPortByWrap(char *pData,int nLen);			//�Զ���ӻ��з�
	bool	HandleRecData(const char* buffer,int nSize);
	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<dogpackage>& resultlist);		//����;\n����
	string GetHostIP();
private:
	string					m_host;
	u_short					m_port;
	Project::Tools::Mutex	m_reveivelock;
	Project::Tools::Mutex	m_lock;
	//�Զ���������
	Project::Tools::Mutex	m_lock_connection;
	Project::Tools::Mutex	m_lock_send;
	HANDLE					m_hDataCheckRetryConnectionThread;
	int						m_nModbusNetworkErrorCount;
	bool					m_bExitThread;
	bool					m_bConnectSuccess;			//���ӳɹ���־λ
	bool					m_bRestartCoreSuccess;		//����Core�ɹ�
	bool					m_bRestartLogicSuccess;		//����Core�ɹ�
	bool					m_bRestartUpdateSuccess;		//����Core�ɹ�
	bool					m_bRestartMasterSuccess;		//����Core�ɹ�

	bool					m_bRestartWatchSuccess;		//����Watch�ɹ�

	char					*m_szWriteBuffer;			//����������
	int						m_nWriteSize;				//�����ʹ�С
	int						m_nReConnectInterval;		//�������
	E_SLAVE_MODE			m_nMode;					
	
	char					m_cSpiltBuffer[g_nDogRecMaxNum];			//���ݽ�������
	char					m_cRecvBuf[g_nDogRecMaxNum];
	int						m_nRecSize;

	static CDogTcpCtrl*     m_csDogCommObj;     // singleton object
	static Mutex            m_mutexLock;                    // mutex object

	wstring					m_strMasterVersion;
};

