#pragma once

#include "TcpClient.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

typedef LRESULT (*LPRecDataProc)(const unsigned char* pRevData, DWORD length,DWORD userdata); //�������ݵĻص�����  add by robert 20150430

const int g_nRecMaxNum = 8192;
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

class CTCPDataSender : public CTcpClient
{
public:
	CTCPDataSender();
	virtual ~CTCPDataSender();

protected:
	
	//����ͨѶ�����İ�.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);
public:
	void	SetHost(string strhost);
	void	SetPort(u_short portnumer);
	void	SetName(string strname);

	//Initialize Tcp Info
	bool	InitTcp(string strHost,u_short portnumer,string strTCPName,int nReConnectInterval = 30,UINT writebuffersize = 1024);
	bool	CloseTcp();


	bool	Init();
	bool	Exit();

	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	bool	ReConnect();

	void    SetNetworkError();
	void    ClearNetworkError();

	// start/stop comm watching
	BOOL	StartMonitoring(LPRecDataProc proc1=NULL,DWORD userdata=NULL);
	BOOL	RestartMonitoring();
	BOOL	StopMonitoring();

	bool	WriteToPort(char *pData,int nLen);
	bool	WriteToPortByWrap(char *pData,int nLen);			//�Զ���ӻ��з�

	bool	UpdateDTUWriteTime();
	bool	GetDTUIdleState(int nSeconds = 4);				//������ʱ����4s,����ʾ����,�����ڲ�����ʷ����
	std::string	GetSuccessBuffer();

	bool	HandleRecData(const char* buffer,int nSize);
	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<datapackage>& resultlist);		//����;\n����

	bool	GetTCPConnectOK();
	void	SetHistoryDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
private:
	string					m_host;
	u_short					m_port;
	Project::Tools::Mutex	m_lock;
	//�Զ���������
	Project::Tools::Mutex	m_lock_connection;
	Project::Tools::Mutex	m_lock_send;
	HANDLE					m_hDataCheckRetryConnectionThread;
	int						m_nModbusNetworkErrorCount;
	bool					m_bExitThread;
	bool					m_bConnectSuccess;			//���ӳɹ���־λ
	bool					m_bSendTcpNameSuccess;		//�������Ƴɹ���־λ
	string					m_strTcpName;				//����

	LPRecDataProc			m_lpRecDataProc;			//�յ����ݻظ��ص�
	DWORD					m_dwUserData;				//�û�����
	char					*m_szWriteBuffer;			//����������
	DWORD					m_nWriteBufferSize;			//
	int						m_nWriteSize;				//�����ʹ�С
	int						m_nReConnectInterval;		//�������
	COleDateTime			m_oleLastWriteTime;			//��һ��дDTUʱ��
	std::string				m_strSendSuccessBuffer;			//���һ�γɹ����͵��ַ���
	char					m_cSpiltBuffer[g_nRecMaxNum];			//���ݽ�������
	char					m_cRecvBuf[g_nRecMaxNum];
	int						m_nRecSize;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
};

