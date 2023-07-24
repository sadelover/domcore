#pragma once
#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF","ENDOFFILE")
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "../DB_BasicIO/DatabaseSession.h"
#include "Tools/StructDefine.h"
#include <vector>
#include <utility>
#include <string>
#include "DB_BasicIO/RealTimeDataAccess.h"
using std::vector;
using std::pair;
using std::wstring;
using namespace std;

class __declspec(dllexport) CSqlServerCtrl
{
public:
	CSqlServerCtrl(void);
	virtual ~CSqlServerCtrl(void);

public:
	//Initialize Modbus Tcp Info
	bool	Init();
	bool    Exit();

	//////////////////////////////////////////////////////////////////////////
	bool ConnectSqlDB( const wstring& host, const wstring& username, 
						const wstring& password, const wstring& database);
	void ExitConnect();
	bool IsConnected();
	bool ReConnect();
	//////////////////////////////////////////////////////////////////////////

	static UINT WINAPI ThreadUpdateValueFunc(LPVOID lparam);
	void	TerminateUpdateValueThread();
	void	UpdateValue();
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);

	bool	UpdateValueByCrossTable();
	bool	UpdateValuesByLongitudinalTable();

	bool	SetValue(wstring strName, wstring strValue);

	bool	QueryTableColumn0();

	static UINT WINAPI ThreadUpdatePointFunc(LPVOID lparam);
	void	TerminateUpdatePointThread();
	void	UpdatePoint();

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
	vector<wstring>	m_vecHLongTableFilterName;	//���ɸѡ�ֶ�ֵ
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

	::_ConnectionPtr	m_pConnection;
	::_RecordsetPtr		m_pRecordSet;
};
