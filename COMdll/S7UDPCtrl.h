#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "nodaveinclude/nodave.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

/*
B��Byte����˼��û��Ĭ��ΪX
VB,VW,VD  �ֽڣ��֣�˫��Ѱַ
����ӳ��Ĵ���				(I0.0~I15.7����λ����			λ
���ӳ��Ĵ���				��Q0.0~Q15.7��					λ	
�����洢��V					(VB0.0~VB5119.7)  			λ���ֽڣ��֣�˫��
�ڲ���־λM(�м�Ĵ���)		��M0.0~M31.7��		λ���ֽڣ��֣�˫��
˳����Ƽ̵���S				��S0.0~S31.7��			λ���ֽڣ��֣�˫��
�����־λ	SM				(SM0.0~SSM179.7)				λ
�ֲ��洢��	L				��LB0.0~LB63.7��
��ʱ�� T					��T0~T255��
������ C					(C0~C255)
ģ��������/���ӳ��Ĵ���	��AI/AQ 0~62�� ��ʼ��ַż���ֽڵ�ַ 1���ֳ�
�ۼ���	AC					��AC0~AC3��		�ֽ�B,��W,˫��D
���ټ����� HC				(HC0~HC5)		˫�ӳ��ķ�������

*/

class __declspec(dllexport) CS7UDPCtrl
{
public:
	CS7UDPCtrl(wstring strPLCIPAddress, int nSlack, int nSlot);
	~CS7UDPCtrl(void);

	bool	Init();
	bool    Exit();
	void	AddLog( const wchar_t* loginfo );
	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	DataPointEntry*	FindEntry(int nFrameType, int nPointID);


	bool	InitPLCConnection();
	bool	ExitPLCConnection();

	bool	ReadOneByOneAndRemoveErrPoint();		//һ�����㷢��,���Ƴ������
	bool    ReadOneDataByIndex(int nIndex);
	bool	GetReadOneByOneSuccess();

	bool	ReadDataOnce();
	bool    ReadDataIndex(int nFromIndex, int nToIndex);

	bool	SetValue(const wstring& pointname, double fValue);
	void	InitDataEntryValue(const wstring& pointname, double fvalue); //������ͨ��ָ��, add by golding

	bool	GetValue(const wstring& pointname, double& result);

	bool	GetConnectSuccess();

	void	EnableLog(BOOL bEnable);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	SetPLCError();
	void	ClrPLCError();
	int		GetErrCount();

	wstring GetIP();

	VARENUM GetPointDBPos(wstring strPLCAddress, int &nPos, int &nOffset, int &nBit,int &nBlock,VARENUM varType = VT_INT);
	VARENUM GetPointDBPos_200(wstring strPLCAddress, int &nPos, int &nOffset, int &nBit,int &nBlock,VARENUM varType = VT_INT);
	VARENUM GetPointPos_200(wstring strPLCAddress,  int &nOffset, int &nBit,VARENUM varType = VT_INT);			//������ĸ
	VARENUM GetPointPos_200_(wstring strPLCAddress,  int &nOffset, int &nBit,VARENUM varType = VT_INT);			//˫����ĸ


	//����log
	string	GetExecuteOrderLog();
	SYSTEMTIME	m_sExecuteTime;
	string		m_strExecuteLog;

	bool	OutPutLogString(wstring strOut);
	bool	UpdateSimensTime();
	void	SetIndex(int nIndex);
	void	SetHistoryDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	Mutex						m_mutex;
	Project::Tools::Mutex		m_lock;		//���ڲ�������
	wstring	m_strCPUAddress;
	int m_nSlack;
	int m_nSlot;
	bool	m_bReadOneByOneSuccess;		//��һ��һ������ȡ�ɹ���־λ

	vector<DataPointEntry> m_pointlist;	//���.
	map<wstring, int> m_mapNameIndex; //�����������Ĺ�ϵmap
	
	vector<VARENUM> m_pointVarTypeList; //DB����ַ
	vector<int> m_pointDBIndexList; //DB����ַ
	vector<int> m_pointOffsetIndexList; //DBƫ����
	vector<int> m_pointOffsetBitList; //Bit
	vector<int>	m_pointBlockArea;		//�Ĵ�����


	Beopdatalink::CLogDBAccess* m_logsession;

	//PLC Connection by DAVE
	daveInterface * m_pPLCInterface;
	daveConnection * m_pPLCConnection;
	bool			m_bPLCConnectionSuccess;		//���ӱ�־λ

	int				m_nPLCReadErrorCount;
	HANDLE			m_hTCPSocket;

	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
	int				m_nEngineIndex;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
};

