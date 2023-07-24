#pragma once

/*
	This dtu sender will send the data to the dtu.
	the dtu is connected to the com port. So we just need to 
	write to the com port.
*/
#include <memory>
#include <string>
#include <vector>
#include <queue>

#include "../COMdll/COM/SerialPort.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "../COMdll/ModbusTcp/TCPDataSender.h"
using std::string;
using std::vector;

class CSerialPort;
class CTCPDataSender;
//class CBEOPDataLinkManager;
using namespace Beopdatalink;

struct _declspec(dllexport) DTUSendInfo
{
	DTUSendInfo()
	{
		nType = 0;
		nSubType = 0;
		nPointCount = 0;
		strPacketIndex = "-1";
		GetLocalTime(&tSendTime);
	}
	SYSTEMTIME tSendTime;
	string	   strSendBuf;
	int		   nType;		//0:ʵʱ����  1����������  2���������� 3:unit01 4 ExecuteLog 5 resend 6History 7realdata file 8exe���»�ִ 9�û�����
	int		   nSubType;	//0:send 1:syn  2:edit 3:restart 4�����޸� 5��sql���� 6������ 7 start 8 history
	int		   nPointCount;		//���͵ĵ����
	string	   strPacketIndex;	//�����   0-9 ��������ͬ������ 10ͬ��unit01���� 11 �޸�unit01���� 12 ����Core 13 ����������  14�����޸�ʵʱֵ
};

struct _declspec(dllexport) DTUServerCmdInfo
{
	DTUServerCmdInfo()
	{
		nLength = 0;
		memset(cData,0,g_nPackageSize);
	}
	char   cData[g_nPackageSize];				//����
	int	   nLength;					//����
};

//CmdType:   1:data 2:ͬ��Unit01�� 3����Core 4 ���õ� 5 ����Unit01 6 �����޸ĵ�(��,�ָ�)  7 sql��� 8 ������ 9�ط���ʧ���ݰ�
//StateType: 0:send 1:syn  2:edit 3 resend 

class _declspec(dllexport) CDTUSender
{
public:
	CDTUSender(CCommonDBAccess* dbsession_history,bool bDTUChecked,bool bDTURecCmd,string strTCPName,string strHost="",u_short portnumer=9500,bool bDTUMode = true,
			  int portnumber = 1, int baudrate = 9600);
	~CDTUSender(void);

	bool SendData(char* buffer, int buffersize);

	bool SendDataByWrap(char* buffer, int buffersize);//�Զ���ӻ��з�

	//��ʼ���ӿ�
	bool InitDTU();
	bool Exit();

	static UINT WINAPI DTUThreadFunc(LPVOID lparam);
	static UINT WINAPI DTUThreadHandRecCmd(LPVOID lparam);
	bool	ScanAndSendLostPackage(string& strUpdateExeAck,string& strLostFile);

	bool	ScanAndSendLostPackage();

	bool SendData();

	bool SendData(const SYSTEMTIME tSendTime, const string& strSendBuff);						//DTU����ʵʱ����

	bool SendWarningData(const SYSTEMTIME tSendTime, const string& strSendBuff);				//DTU���ͱ�������

	bool SendWarnigOperationData(const SYSTEMTIME tSendTime, const string& strSendBuff);		//DTU ���ͱ�����������

	bool SendUserOperationData(const SYSTEMTIME tSendTime, const string& strSendBuff);		//DTU �����û���������

	bool SendExecuteLog(const SYSTEMTIME tSendTime, const string& strSendBuff);				//DTU ����ִ��˳��

	bool SendUnit01(const SYSTEMTIME tSendTime, const string& strSendBuff);		//DTU ����Unit01����

	bool SendHeartBeat(const SYSTEMTIME tSendTime);								//DTU ������������Ӧ

	bool SendCoreStart(const string strProjectName);								//DTU ����������־

	bool SendFile(string strFilePath);					//DTU�����ļ�

	bool SendRealDataFile(string strFilePath,int nPointCount);					//DTU����ʵʱ�����ļ�

	bool SendHistoryDataFile(string strFilePath,int nPointCount);					//DTU������ʷ��ʧ�ļ�

	bool SendUpdateFileAck(int nState, string strContetn);		//DTU���͸����ļ���ִ  1:����exe�ɹ�  2:����exeʧ��  3���Ͷ�ʧ�ļ��ɹ� 4���Ͷ�ʧ�ļ�ʧ��

	bool SendCmdState(const SYSTEMTIME tSendTime,int nCmdType, int nCmdState, string strPacketIndex);

	bool SendRDP_AT();			//RDP  ����ATָ��

	bool SendRDP_Login();		//AT ��¼ָ��

	bool SendRDP_QueryDTUID();	//��ѯDTU��ʾ��

	bool SendRDP_QueryDSCIP();	//��ѯDSCIP

	bool SendRDP_QueryCSQ();	//��ѯ�ź�

	bool SendRDP_Reset();		//�˳�AT ����DTU

	bool SendQueryState();

	bool RestartDTU();			//����DTU

	bool SendLogin();

	bool SendLogin1();

	//DTUÿ���ֻ�ܴ���1024���ֽڡ���ȫ�����ÿ�����ֻ����512���ֽڵ����ݡ�
	//��Ҫ�����
	void BuildPackage(const string& buffer, vector<string>& resultlist);

	void BuildAndSendOperationRecord();

	void	ExitThreads();

	bool	GetReadThreadExit() const;
	bool	GetSendThreadExit() const;
	bool	GetRecThreadExit() const;
	//ÿż�����Ӷ�ʱ�����ݡ�
	void	BuildData(const SYSTEMTIME& st);

	void	AddLog(const wchar_t* loginfo);
	std::string	GetSuccessBuffer();

	//��������
	static LRESULT OnRecData(const unsigned char* pRevData, DWORD length,DWORD userdata);
	void   GetOutputByRecData(const char* buffer,vector<Beopdatalink::CRealTimeDataEntry>& entrylist);

	void   GetDTU_TO_RTU(const unsigned char* pRevData, DWORD length);

	bool	AnalyzeReceive(const unsigned char* pRevData, DWORD length);
	void	AnalyzeUpdateFile(const unsigned char* buffer);
	void	AnalyzeRecLogFile(const unsigned char* buffer);

	bool	GetReceiveDTUServerEvent(vector<vector<string>>& vecCmd);		//DTUServer�������
	bool	SetReceiveDTUServerEvent(bool bReceiveDTUServerEvent);
	void	UpdateReceiveBuffer(char* pBuffer);
	void	UpdateDTUWriteTime(char* pBuffer);

	void   SplitString(const std::string& source, const char* sep, std::vector<string>& resultlist);

	bool	StoreSendInfo(string strSendContent);
	string	GetStoreInfo(int nIndex,int& nBase);
	bool	ReSendRealData(string strIndex);

	bool	OutPutLogString(char* pOut);
	bool	OutPutLogErrString(char* pOut);

	bool	HandRecCmd();
	bool	GetSendIdle(int nSeconds=4);
	void	SetDTUToRTU(bool bDTU_To_RTU);

	string	GetDTUCSQInfo();

	bool	StoreDTUCmd(DTUServerCmdInfo data);
	//void	SplitStringSpecial(const char* buffer,int nSize, std::vector<char*>& resultlist,std::vector<int>& sizelist);		//����;\n����
	unsigned int BufferToInt(const unsigned char* buffer);

	bool	GetDTUSuccess();
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	std::auto_ptr<CSerialPort> m_porthandler;
	std::auto_ptr<CSerialPort> m_porthandler2;

	std::auto_ptr<CTCPDataSender> m_tcphandler;

	int m_portnumber;		//�˿ں�
	int m_baudrate;			//������

	HANDLE m_sendthread;
	bool	m_bsendthreadexit;

	HANDLE	m_readthread;
	HANDLE m_recthread;
	bool    m_brecthreadexit;
	std::string m_sendbuffer;
	SYSTEMTIME  m_sendtime;
	bool    m_breadthreadexit;
	bool	m_bDTU_To_RTU;
	bool	m_bDTUConnectOK;		//DTU���ӳɹ�
	bool	m_bReceiveDTU;			//�յ�DTU����
	bool	m_bReceiveDTUServerEvent;			//�յ�DTUServer��������
	bool	m_bDTUChecked;
	bool	m_bDTURecCmd;
	int		m_nReceiveDTUCmdType;				//�յ�DTUServer��������
	char*   m_pReceiveDTUServer;
	DWORD	m_dReceiveDTUServer;
	vector<string>	m_vecCmd;

	Project::Tools::Mutex	m_lock;
	Project::Tools::Mutex	m_sendlock;
	Project::Tools::Mutex	m_reveivelock;
	Project::Tools::Mutex	m_reclock;
	//CBEOPDataLinkManager* m_pBeopdatalinker;
	COleDateTime	m_oleLastWriteTime;			//��һ��дDTUʱ��

	int		m_nDTUPackageNumber;				//����ʵʱ���ݰ����
	int		m_nDTUPackageBase;					//ʵʱ���ݷ�������
	int		m_nDTUFilePackageNumber;			//�ļ������
	map<int, string> m_mapStoreSendPackage;		//�洢������͵İ�
	map<int,int>	 m_mapStoreSendPackageBase;	//�洢������Ͱ��Ļ����ţ�����ȷ��ʱ��ʹ�ã�
	map<int, string> m_mapStoreFilePackage;		//�洢���һ�η����ļ�������

	vector<vector<string>>	m_vecDTUCmd;			//�յ��ķ���������
	queue<string>	m_qRecDTUCmd;					//�յ��ķ���������

	char			m_cRXBuff[MAX_REC_COUNT];			//DTU�����ַ���
	char			m_cSpiltBuffer[MAX_REC_COUNT];			//���ݽ�������
	char			m_cRTUBuffer[MAX_REC_COUNT];			//RTU����
	char			m_cSpiltBufferSpilt[MAX_REC_COUNT];			//���ݽ�������
	char			m_cFileBuffer[1024][g_nPackageSize];					//�ļ��洢
	char			m_cFileUpdateBuffer[1024][g_nPackageSize];			//Core����
	map<int,int>	m_mapFileUpdateState;						//�ļ��հ����
	

	int				m_nReceCount;
	bool			m_bRunReserverPort;
	int				m_nRunReserverPort;

	string			m_strDSCIP;
	string			m_strDTUID;				//DTU��������ID
	string			m_strCSQ;
	int				m_nRTUAnswerMode;		//RTU��Ӧ

	CRITICAL_SECTION m_csDTUDataSync;			//�ٽ������� ����������Դ
	queue<DTUServerCmdInfo>	m_queCmdPaket;		//�洢�����������

	string			m_strRecFileName;				//�ļ�����
	int				m_nFileCount;					//�ļ�������
	int				m_nRecFileCount;				//�յ����ļ�������


	string			m_strCoreFileName;				//Core�ļ�����
	int				m_nCoreFileCount;				//Core�ļ�������
	int				m_nRecCoreFileCount;			//�յ���Core�ļ�������
	COleDateTime	m_oleUpdateExeTime;				//�յ�����exeʱ��
	bool			m_bHasLostFileFlag;				//��Ҫ�ط���־
	string			m_strUpdateExeAck;				//�ɹ�����EXE��ִ
	int				m_nLastUpdateExeSize;				//���һ�����Ĵ�С

	bool			m_bDTUMode;						//trueΪDTUģʽ  false ΪTCPģʽ
	string			m_host;
	u_short			m_port;
	string			m_strTcpName;					//����
	bool			m_bdtuSuccess;					//DTU�����ʼ���ɹ�

	int				m_nTCPSendFilePackageSize;				//TCPÿ�η����ĳ��� Ĭ��Ϊ1000��2gDTUֻ��ѡ������ã� TCPΪ4000
	int				m_nTCPSendFilePackageInterval;			//TCPÿ�η����ĳ��� Ĭ��Ϊ2000ms��2gDTUֻ��ѡ������ã�	TCP���Ϊ200ms

	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
};

