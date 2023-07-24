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

class CModbusRTUCtrl : public CSerialPortCtrl
{
public:
	CModbusRTUCtrl(void);
	virtual ~CModbusRTUCtrl(void);

	bool	Init();
	bool	Exit();
	bool	ReInitPort(int nType=0);
	void	SetPortAndBaud(UINT nPortnr, UINT bBaud, char parity = 'N');
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	void	GetValueByDataTypeAndRange(DataPointEntry entry,vector< pair<wstring, wstring> >& valuelist);	
	void	GetValueByDataTypeAndRange(DataPointEntry entry,E_MODBUS_VALUE_TYPE type,vector<WORD>& vecValue,vector< pair<wstring, wstring> >& valuelist);
	void	GetValueByDataTypeAndRange(DataPointEntry entry,double dValue,vector<WORD>& vecCmd,bool bOrder);		//tureĬ�ϴӸ��ֽ�������

	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);
	bool	InitDataEntryValue(const wstring& pointname, double value);

	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);

	////ʹ�ü�������ģʽ//////////////////////////////////////////////////////////////////////
	void	SendReadCommandsByActive();				//�ظ�����ģʽ
	void	SetSendCmdEvent();						//���ü����¼�
	HANDLE	GetSendCmdEvent();						//��ȡ�����¼�
	void	SendOneByOneByActive();
	bool	SendOneByOneByActive(_ModbusReadUnit& unit);
	bool	SendReadMutilByActive(_ModbusReadUnit& unit);		//�޸�дֵ˳����� 20190132
	bool	SetValueByActive(const wstring& pointname, double fValue);
	//////////////////////////////////////////////////////////////////////////

	void	SetReadCommands_();
	void	SendOneByOne_();
	bool	SendOneByOne_(_ModbusReadUnit& unit);
	bool	SendReadMutil_(_ModbusReadUnit& unit);
	bool	SendWriteCmd(DataPointEntry entry,double fValue);			//�豸ͨѶ����
	bool	SendWriteMutilCmd(DataPointEntry entry,double fValue);			//�豸ͨѶ����
	bool	SendWriteBYTECmd(DataPointEntry entry,double fValue);			//�豸ͨѶ����
	bool	SendWriteWORDCmd(DataPointEntry entry,double fValue);			//�豸ͨѶ����
	void    SetNetworkError();
	void    ClearNetworkError();

	void    SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount=99,int nIDInterval = 500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false,bool bSaveErrLog = false);
	void	SetReadOneByOneMode(bool bReadOneByOneMode);
	void	SetPointList(const vector<DataPointEntry>& pointlist);

	DataPointEntry*	FindEntry_(DWORD slaveid, DWORD headaddress, DWORD funccode);
	DataPointEntry*	FindEntryFromQueryCache_(DWORD slaveid, DWORD headaddress, DWORD funccode);
	void	SetModbusReadUnitErrFlag(unsigned int nSlaveId,DWORD dwFuncCode,DWORD dwAdd,bool bHasErrPoint=true);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	SetIndex(int nIndex);

	void    SortPointByDevice_();
	bool	CombineReadUnit(const _ModbusReadUnit &  mrUnit,bool bModbusEnd=true);
	void	CreateEntryByDataTypeAndRange(DataPointEntry entry,vector<DataPointEntry>& vecModbus);

	//ת������
	int		GetBitFromWord(WORD nData, int nIndex);
	WORD	SetBitToWord(WORD nData, int nIndex,int nValue);
	DWORD	MergeTwoWORD(WORD wGao,WORD wDi,bool bOrder = true);		//
	double	MergeFourDouble(WORD wGaoGao,WORD wGao,WORD wDi,WORD wDiDi,bool bOrder = true);		//
	double	strtodbl(const wchar_t * str);

	//
	bool	SetDebug(int nDebug);
	bool	OutPutModbusReceive(const unsigned char* pRevData, DWORD length,WORD wStart);
	string* byteToHexStr(unsigned char* byte_arr, int arr_len);  
	string  charToHexStr(unsigned char* byte_arr, int arr_len);  

	int		GetMultiReadCount(DWORD slaveid, DWORD headaddress, DWORD funccode);

	bool	InsertLog(const wstring& loginfo, COleDateTime oleTime);
	bool	SaveLog();
	void	SumReadAndResponse();	
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
protected:
    virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

	//Initialize hash_map Info
	bool Init_HashMap(void);

	void SendCmd(char *pData, BYTE bLength,WORD nStartAddress = 0,bool bWrited=false);
	BOOL Is_DataValid(BYTE *pData, WORD dwLength);

public:
	//Initialize COMPort Info
	bool InitCOMPort(UINT nPortnr, UINT bBaud, char parity = 'N');

	//Send Read Command
	virtual bool SendReadCmd(unsigned int nSlaveId   //�豸��Slave ID ��
		, WORD wAddrFrom		//��ȡ���Ե���ʼ��ַ
		, WORD wAddrTo			//��ȡ���Ե���ֹ��ַ
		, WORD wFunc);			//�豸ͨѶ����

	//Send Write Command
	virtual bool SendWriteCmd(unsigned int nSlaveId   //�豸��Slave ID ��
		, WORD wAddress			//д�����Եĵ�ַ
		, WORD wValue			//����ֵ
		, WORD wFunc);			//�豸ͨѶ����

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
	int							m_nCmdCount;						//�������
	int							m_nResponseCount;					//����ظ�����
	int							m_nUpdatePointCount;				//���µ�
	string						m_strUpdateLog;						//����log

	_ModbusReadUnit				m_lastReadUnit;						//��һ�ζ�ȡ�ṹ
	DataPointEntry				m_lastReadEntry;
	HANDLE						m_sendCmdEvent;

	Project::Tools::Mutex		m_lock_send_log;				//������
};