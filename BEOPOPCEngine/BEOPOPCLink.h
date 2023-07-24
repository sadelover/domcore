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
#include "DataPointEntry.h"

class COPCConnection_AutoBrowse;


class COPCEngine
{
public:
	COPCEngine(wstring strOPCServerIP,int nSleep = 5,int nMutilCount =1,bool bUseThread = false,int nSleepFromDevice = 50,int nPollSleep = 5,int nPollSleepFromDevice = 60,bool bDisableCheckQuality=false,int nPrecision=6,bool bAsync20Mode=false,int nRefresh20Interval = 60);
	~COPCEngine(void);

	void	SetOpcPointList(const vector<OPCDataPointEntry>& pointlist);
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
	bool	Refresh20(bool bDeviceRead = false);		//�첽ˢ��ȫ������
	bool	GetMutilOPCData(int nStart, int nEnd, vector<OPCDataPointEntry>& pointlist);
	double	GetBitFromFloat(float dValue, int nIndex);			//�ӵ�һλ��ʼ����
	int		GetBitFromInt(int nValue, int nIndex);
	wstring	GetBitFromString(wstring strValue, int nIndex);

	double	SetBitToFloat(double nData, int nIndex,double nValue);
	wstring	SetBitToString(double nData, int nIndex,wstring strValue);

	void	ClearResource();
	static COPCConnection_AutoBrowse* CreateOPCObject(OPCSERVER_TYPE_ type);

	//��opc server�ж�ֵ
	bool	SetValue(const wstring& pointname, double value);
	bool	SetValue(const wstring& pointname, wstring strvalue);
	bool	GetValue(const wstring& pointname, wstring& strvalue);
	bool	GetValue(const wstring& pointname, bool& strvalue);
	bool	GetValue(const wstring& pointname, int& strvalue);
	bool	GetValue(const wstring& pointname, double& strvalue);

	void	OutErrLog(wstring strLog,bool bDaily=false);

	bool	GetOPCConnectState();
	
private:
	vector<OPCDataPointEntry>  m_pointlist;
	COPCConnection_AutoBrowse* m_opcsession;		// opc engine will own the object.

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
	int			 m_nDebugOPC;
	int			m_nPrecision;				//���ݾ��� Ĭ��6
	bool		m_bAsync20Mode;				//�첽��дģʽ
	COleDateTime m_oleRefresh20;			//�첽ˢ��ʱ��
	int			m_nRefresh20Interval;		//�첽ˢ�¼��
};

