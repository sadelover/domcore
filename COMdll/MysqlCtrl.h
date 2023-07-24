#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "../DB_BasicIO/DatabaseSession.h"
#include "Tools/StructDefine.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

#include "DB_BasicIO/RealTimeDataAccess.h"
class __declspec(dllexport) CMysqlCtrl : public CDatabaseSeesion
{
public:
	CMysqlCtrl(void);
	virtual ~CMysqlCtrl(void);

public:
	//Initialize Modbus Tcp Info
	bool Init();
	bool    Exit();

	static UINT WINAPI ThreadUpdateValueFunc(LPVOID lparam);
	void	TerminateUpdateValueThread();
	void	UpdateValue();

	bool	UpdateValueByCrossTable();
	bool	UpdateValuesByLongitudinalTable();

	bool	SetValue(wstring strName, wstring strValue);

	bool	QueryTableColumn0();

	static UINT WINAPI ThreadUpdatePointFunc(LPVOID lparam);
	void	TerminateUpdatePointThread();
	void	UpdatePoint();

	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	bool	ReConnect();
	void    SetNetworkError();
	void    ClearNetworkError();

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );

	int		IsExistInVec(wstring strSer, vector<wstring> vecSer);
	int		IsExistInVec(wstring strTName,wstring strOrderColumn, wstring strFilterColumn,vector<_MysqlCrossTableUnit> vecSer);

	DataPointEntry*	FindEntry(wstring nRuntimeID);

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	EnableLog(BOOL bEnable);
	string	GetHostIP();

	void	SetIndex(int nIndex);
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	wstring m_strhost;
	wstring m_strusername;
	wstring m_strpassword;
	wstring m_strdatabase;
	int		m_nPort;
	int		m_nTableType;		//0 Ϊ�ݱ�  1Ϊ���1 2 Ϊ�ݱ�����ʷ����ģʽ 3��ʷ���

	vector<wstring> m_vecTableName;
	vector<wstring> m_vecCrossTableName;		//��� �Ե�һ������ ȥ��һ�� ���ֶ���Ϊ����
	vector<wstring> m_vecLongTableName;			//�ݱ� ����mdo/beop��ʵʱ��
	vector<wstring> m_vecHLongTableName;		//��ʷ�ݱ�  uppc��record��¼��ʱ��/����/�У�
	//vector<wstring>	m_vecHLongTableFilterName;	//���ɸѡ�ֶ�ֵ
	//vector<wstring> m_vecHCrossTableName;		//��ʷ���
	vector<_MysqlCrossTableUnit>	m_vecHCrossTableName;		//��ʷ���

	map<wstring,wstring>	m_runTimeDataMap;						//
	map<wstring,map<wstring,wstring>> m_runTimeDataMap1;		//Ϊ�ݱ�����ʷ����ģʽ
	map<wstring,wstring>		m_TableColumn0;			//�ݱ�ĵ�һ������
	map<wstring,map<int,wstring>> m_HCrossTableColumn;		//Ϊ��ʷ��������
	map<wstring,map<int,wstring>>	m_HCrossTableDataMap;		//��ʷ����ֵ

	HANDLE	m_hupdatethread;
	HANDLE	m_hupdatepointthread;

	bool m_bExitUpdateThread;
	vector<DataPointEntry> m_pointlist;	//���.
	
	Project::Tools::Mutex	m_lock;
	Project::Tools::Mutex	m_dblock;
	BOOL m_bLog;
	Beopdatalink::CLogDBAccess* m_logsession;

	string	m_strLocalIP;

	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
	int				m_nEngineIndex;

	//�Զ���������
	HANDLE					m_hDataCheckRetryConnectionThread;
	int						m_nMysqlNetworkErrorCount;
	bool					m_bConnectOK;
	bool					m_bQueryColumnOK;			//��ѯ���ֶγɹ�
};
