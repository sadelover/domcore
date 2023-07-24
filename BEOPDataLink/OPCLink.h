#pragma once




/*
 **
 **
 **
 ** this class defines the opc engine.\
 ** it may include more than one opc ctrls for reading or writing files. 
 **	
 **
 **
 **/

#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

#include "DB_BasicIO/RealTimeDataAccess.h"
#include "BEOPDataPoint/DataPointEntry.h"


class COPCConnection_AutoBrowse;
class CLogDBAccess;
class CBEOPDataLinkManager;


class COPCEngine
{
public:
	COPCEngine(CBEOPDataLinkManager* datalinker,Beopdatalink::CCommonDBAccess*	dbsession_history, wstring strOPCServerIP,int nSleep = 5,int nMutilCount =1,bool bUseThread = false,int nSleepFromDevice = 50,int nPollSleep = 5,int nPollSleepFromDevice = 60,bool bDisableCheckQuality=false,int nPrecision=6,bool bAsync20Mode=false,int nRefresh20Interval = 60, int nLanguageID=0, int nMultilAddItems =1000, int nDebugOPC=0, int nUpdateRate =500);
	~COPCEngine(void);

	void	SetOpcPointList(const vector<DataPointEntry>& pointlist);
	void	SetMultiMode(bool bMultiMode);
	void	SetDebug(int nDebug,int nOPCCheckReconnectInterval=5,int nOPCReconnectInertval=10,int nReconnectMinute = 0,bool bEnableSecurity=false,int nOPCMainThreadOPCPollSleep = 2);
	bool	Init(int nOPCClientIndex = -1);
	bool	Exit();

	void	GetValueSet(vector< pair<wstring, wstring> >& valuelist );

	static UINT WINAPI ThreadUpdateOPCData(LPVOID lparam);
	static UINT WINAPI ThreadConnectOPCData(LPVOID lparam);
	bool	ReConnectOPC();
	bool	ReLoadItemHandle();					//��Ϊ�̰߳�ȫ��Ե�� ֻ����������ȥ��
	bool	UpdateMutilOPCData();
	bool	UpdateSingleOPCData();
	bool	UpdateOPCDataBuffer();				//����OPC�㻺��

	vector<DataPointEntry> & GetPointList();


	//////////////////////////////////////////////////////////////////////////
	//20180926
	void	GetValueListV1(vector< pair<wstring, wstring> >& valuelist );	//��pointlist�з���ֵ
	bool	UpdateMutilOPCDataV1();
	bool	UpdateSingleOPCDataV1();
	bool	UpdateOPCDataBufferV1();				//������OPC�㻺��
	bool	ReReadAsync20ItemV1();			//�첽дֵ�� ÿ����ֵʱ�� ��������ȡ�����ڴ����첽�ص�д�ɹ��ĵ�λ
	bool	GetValueFromOPCItemV1();				//20180926 ��OPCitem�а�ֵ���µ�pointlist
	bool	GetMutilOPCDataV1(int nStart, int nEnd, vector<DataPointEntry>& pointlist, vector<wstring> & strBadPointList);
	//////////////////////////////////////////////////////////////////////////

	bool	Refresh20(bool bDeviceRead = false);		//�첽ˢ��ȫ������
	bool	GetMutilOPCData(int nStart, int nEnd, vector<DataPointEntry>& pointlist);
	double	GetBitFromFloat(float dValue, int nIndex);			//�ӵ�һλ��ʼ����
	int		GetBitFromInt(int nValue, int nIndex);
	wstring	GetBitFromString(wstring strValue, int nIndex);
	bool	ReReadAsync20Item();			//�첽дֵ�� ÿ����ֵʱ�� ��������ȡ�����ڴ����첽�ص�д�ɹ��ĵ�λ

	double	SetBitToFloat(double nData, int nIndex,double nValue);
	wstring	SetBitToString(double nData, int nIndex,wstring strValue);

	void	ClearResource();
	static COPCConnection_AutoBrowse* CreateOPCObject(OPCSERVER_TYPE type);

	//��opc server�ж�ֵ
	bool	SetValue(const wstring& pointname, double value);
	bool	SetValue(const wstring& pointname, wstring strvalue);
	bool	GetValue(const wstring& pointname, wstring& strvalue);
	bool	GetValue(const wstring& pointname, bool& strvalue);
	bool	GetValue(const wstring& pointname, int& strvalue);
	bool	GetValue(const wstring& pointname, double& strvalue);

	//��ʼ��ֵ
	bool InitDataEntryValue(const wstring& pointname, double value);

	//ֱ�Ӵ�plc��ֵ
	bool	GetValueFromDevice(const wstring& pointname, wstring& strvalue);
	bool	GetValueFromDevice(const wstring& pointname, bool& strvalue);
	bool	GetValueFromDevice(const wstring& pointname, int& strvalue);
	bool	GetValueFromDevice(const wstring& pointname, double& strvalue);

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);

	void	OutErrLog(wstring strLog);
	void	SumReadAndResponse();						//ÿ�η���ǰͳ����һ�ε��շ����
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	vector<DataPointEntry>  m_pointlist;
	COPCConnection_AutoBrowse* m_opcsession;		// opc engine will own the object.
	Beopdatalink::CLogDBAccess* m_logsession;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
	CBEOPDataLinkManager*	m_datalinker;

	wstring m_strOPCServerIP;
	bool	m_bMultiClientMode;
	bool	m_bConnectOK;
	HANDLE  m_hDataReadThread;
	HANDLE  m_hOPCConnectThread;
	HANDLE  m_hDataReadDeviceThread;
	bool	m_bExitThread;
	bool	m_bUseThread;			//ʹ���߳�ȥ��ȡ
	int		m_nMutilCount;			//������ȡ����
	int		m_nSleep;				//OPC���������ӻ��棩
	int		m_nReSponse;
	int		m_nReadCount;					//OPC������(����)
	int		m_nReadDeviceCount;				//OPC������(�豸)
	int		m_nReSponseDevice;				//OPC����(�豸)
	int		m_nOPCClientIndex;
	int		m_nOPCCmdSleepFromDevice;		//OPC������(���豸)
	int		m_nOPCPollSleep;				//OPC��ѯ������ӻ��棩
	int		m_nOPCPollSleepFromDevice;		//OPC��ѯ���(���豸)
	int		m_nOPCCheckReconnectInterval;	//OPC������������min��
	int		m_nOPCReconnectInertval;		//OPC N��������δ��������(min)
	bool	m_bEnableSecurity;				//�������̰�ȫ������
	bool	m_bDisableCheckQuality;			//��������Ʒ�����ݼ��
	int		m_nOPCMainThreadOPCPollSleep;	//OPC��ѯ���s�������̴ӻ��棩
	int		m_nMainUpdateIndex;				//OPC�����̸��´���

	COleDateTime m_oleDeviceTime;
	COleDateTime m_oleUpdateTime;			//����ʱ��,���ӳɹ�����10������δ���³ɹ�һ�Σ�������
	COleDateTime m_oleReConnectTime;		//������N�����ڲ��洢����
	int			 m_nReconnectMinute;		//������N����
	bool		 m_bReconnect;				//������־λ
	Project::Tools::Mutex		m_lock;		//���ڲ�������
	int			 m_nSingleReadCount;
	int			 m_nReloadErrItem;			//�����̻�ȡ����N��ʱ�����¼�����Ч��Item
	hash_map<wstring, double>	m_PositionValuemap;		//ӳ��ȡλ�ĵ�����ֵ�Ĺ���
	int			m_nPrecision;				//���ݾ��� Ĭ��6
	bool		m_bAsync20Mode;				//�첽��дģʽ
	COleDateTime m_oleRefresh20;			//�첽ˢ��ʱ��
	int			m_nRefresh20Interval;		//�첽ˢ�¼��
	int			m_nLanguageID;				//����ID
	int			m_nMultiAddItems;			//������ӵ�
	int			m_nDebugOPC;				//����OPC
	int			m_nUpdateRate;			//����������������ˢ����
	string m_strUpdateLog;
};

