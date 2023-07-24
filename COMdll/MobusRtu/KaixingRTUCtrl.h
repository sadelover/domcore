#pragma once

#include "ModbusRTU.h"
#include "SerialPortCtrl.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "Tools/StructDefine.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;


//hash_map 4251
#pragma warning(disable:4251)

#define  _CMD_REV_LENTH_  0x100   //�������ݵĳ���
#define _CMD_KAIXING_LENTH_	 256	//Э�鳤��

class CKaixingRTUCtrl : public CSerialPortCtrl
{
public:
	CKaixingRTUCtrl(void);
	virtual ~CKaixingRTUCtrl(void);

	bool	Init();
	bool	Exit();
	bool	ReInitPort();
	bool	InitCOMPort(UINT nPortnr, UINT bBaud, char parity = 'N');
	void	SetPortAndBaud(UINT nPortnr, UINT bBaud, char parity = 'N');
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);
	bool	InitDataEntryValue(const wstring& pointname, double value);

	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);

	void    SetNetworkError();
	void    ClearNetworkError();

	void    SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount=99,int nIDInterval = 500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false,bool bSaveErrLog = false);
	void	SetPointList(const vector<DataPointEntry>& pointlist);

	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	SetIndex(int nIndex);

	//
	bool	SetDebug(int nDebug);
	bool	OutPutModbusReceive(const unsigned char* pRevData, DWORD length,WORD wStart);
	string* byteToHexStr(unsigned char* byte_arr, int arr_len);  
	string  charToHexStr(unsigned char* byte_arr, int arr_len);  

	//////////////////////////////////////////////////////////////////////////
	void	SendReadCommandsByActive();
	HANDLE	GetSendCmdEvent();							//��ȡ�����¼�
	static void	SetSendCmdEvent();						//���ü����¼�
		
	bool	SendConnectFirstStep();						//���ӵ�һ��
	bool	SendConnectSecondStep();					//���ӵڶ���
	void	SendCmd(char *pData, BYTE bLength);			//
	BOOL	Is_DataValid(BYTE *pData, WORD dwLength);	//�ۼӺ� ����У��
		
protected:
    virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);


private:
	char *m_pSendData;					//the send package buffer
	CModbusRTU					m_Modbus;
	CRITICAL_SECTION			m_CriticalSection; //�ٽ�������
	
	WORD						m_StartAddress;		//��ʼ��ַ �����
	WORD						m_WriteStartAddress;		//��ʼ��ַ д���
	int							m_nReadNum;			//��ȡ����


public:
	HANDLE						m_hsendthread;
	HANDLE						m_hDataCheckRetryConnectionThread;
	bool						m_bExitThread;
	vector<DataPointEntry>		m_vecPointList;
	vector<_ModbusReadUnit>		m_ReadUnitList; //��ȡ���ݰ���Ԫ����ϲ���Ӷ����ٶ�ȡ
	vector<DataPointEntry>		m_pointlist;	//���.
	int							m_nSendReadCmdTimeInterval;				//����������
	int							m_nRecevieTimeOut;						//���ճ�ʱ
	int							m_nPollSendInterval;				//��ѯ���
	int							m_nRecvedSlave;
	int							m_nMutilReadCount;		//������������ Ĭ��99
	bool						m_bReadOneByOneMode;
	COleDateTime				m_oleUpdateTime;
	string						m_strErrInfo;
	int							m_nEngineIndex;
	int							m_nIDInterval;			//��ͬID֮��ļ�� Ĭ��500ms
	
	Project::Tools::Mutex		m_lock;
	Project::Tools::Mutex	m_lock_connection;
	Project::Tools::Mutex		m_lock_log;				//log��
	map<wstring,DataPointEntry*>	m_mapErrPoint;		//��ȡ�����
	map<wstring,COleDateTime>	m_mapErrLog;			//log��־������Ӵ�һ��
	COleDateTime				m_oleLastUpdateLog;		//��һ�α���logʱ��
	bool						m_bSaveErrLog;			//�Ƿ�洢������־

	UINT						m_nPortnr;
	UINT						m_nBaud;
	char						m_cParity;
	bool						m_bInitPortSuccess;		//���Ӷ˿ڳɹ�
	int							m_nModbusNetworkErrorCount;
	Beopdatalink::CLogDBAccess* m_logsession;
	int							m_nOutPut;				//������յ���cmd
	int							m_nPrecision;				//���ݾ��� Ĭ��6
	bool						m_bDisSingleRead;		//��ֹ����ʧ�ܺ��Ϊ����
	hash_map<wstring,DataPointEntry*> m_mapPointQuery;		//���ҵ�λָ��
	CRITICAL_SECTION			m_CriticalPointSection; //�ٽ�������

	//////////////////////////////////////////////////////////////////////////
	static HANDLE		m_sendCmdEvent;
	int					m_nEvent;								//�¼���� 0:Ĭ��
	int					m_nReadTimeOut;							//��ʱʱ��
	bool				m_bNeedActive;							//��Ҫ�����¼�
	static COleDateTime	m_oleSendCmdTime;						//��������ʱ��
	bool				m_bConnectFirstSuccess;					//���Ӽ������ɹ�
	BYTE				m_bSendCmd[_CMD_KAIXING_LENTH_];		//��������
	int					m_nConcentratorType;					//����������
};