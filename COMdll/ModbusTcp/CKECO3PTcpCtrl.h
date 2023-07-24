#pragma once

#include "TcpIpComm.h"
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

#define _CMD_CO3P_LENTH_	    0x08		//Э�鳤��

/*
	param1:��ַ��
	param2:��������ַ
	param3:�ն˻���ַ
	param4:�ܿ��ն˻������ͷ�
	param5:�ض��Ĳ�������
	param6:����λ��
	param7:���ݸ�ʽ
		5.2 �������ֳ���.С�����ֳ���
		1/16 ȡλ
*/


class CCKECO3PTcpCtrl : public CTcpIpComm
{
public:
	CCKECO3PTcpCtrl(void);
	virtual ~CCKECO3PTcpCtrl(void);

protected:
	//����ͨѶ�����İ�.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:
	void	SetHost(const string& strhost);
	void	SetPort(u_short portnumer);
	void	SetIndex(int nIndex);
	void    SetSendCmdTimeInterval(int nTimeMs,int nRollInterval=2,int nTimeOut=5000,int nPrecision=6);

	//Initialize Modbus Tcp Info
	bool	Init();
	bool	Exit();

	bool	SumValid(const unsigned char* pRevData,int nStartIndex,int len);			//�ۼӺ�У��
	unsigned  char Sum__(const unsigned char* pRevData,int nStartIndex,int len);		//�ۼӺ�У����

	//Send Read Command
	virtual bool SendReadCmd(WORD wAddr1			//��������ַ, ���ö����Ʊ�ʾ������Ϊ 1Byte��Ѱַ��ΧΪ 00��7FH��
							, WORD wAddr2			//�ն˻���ַ ������Ϊ 1Byte��Ѱַ��ΧΪ 00��7FH��FFH Ϊ�����ַ�����ù㲥��ʽʱ��ʹ�øõ�ַ 
							, WORD wType			//�ܿ��ն˻������ͷ�������Ϊ 1Byte, ����ѹ�� BCD ���ʾ�� 
							, WORD wCmd);			//�����ض��Ĳ����������Ϊ 1Byte

	void	SendReadCommands();
	bool	SendReadMutil_(_CKECO3PReadUnit& unit);
	
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	void	TerminateSendThread();

	//////////////////////////////////////////////////////////////////////////
	_CKECO3PReadUnit* FindCKECO3PReadUnit(WORD wAddr1,WORD wAddr2,WORD wType,WORD wCmd);
	void	SetPointList(const vector<DataPointEntry>& pointlist);
	bool	CombineReadUnit(const _CKECO3PReadUnit &  mrUnit);
	
	bool	ReConnect();
	void    SetNetworkError();
	void    ClearNetworkError();

	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	bool    GetValueByDataTypeAndRange(DataPointEntry entry,wstring& strValue);
	int		GetBitFromWord(WORD nData, int nIndex);
	bool	UpdateValue();
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	Project::Tools::Mutex	m_lock;
	string					m_host;
	u_short					m_port;
	WORD					m_wAddr1;
	WORD					m_wAddr2;
	WORD					m_wCmd;
	WORD					m_wType;
	HANDLE					m_hsendthread;
	HANDLE					m_hDataCheckRetryConnectionThread;
	bool					m_bExitThread;
	bool					m_bReSponseSuccess;			//�ظ�����

	vector<_CKECO3PReadUnit> m_ReadUnitList;
	vector<DataPointEntry>	m_vecPointList;
	BYTE					m_SendBuffer[_CMD_CO3P_LENTH_];	//��������

	//�Զ���������
	Project::Tools::Mutex	m_lock_connection;
	int						m_nNetworkErrorCount;
	int						m_nRollInterval;			//��ѯ���
	int						m_nSendReadCmdTimeInterval;				//����������
	int						m_nRecevieTimeOut;						//���ճ�ʱ
	COleDateTime			m_oleUpdateTime;
	int						m_nEngineIndex;
	int						m_nPrecision;				//���ݾ��� Ĭ��6
};

