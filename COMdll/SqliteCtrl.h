#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataPoint/sqlite/SqliteAcess.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

class __declspec(dllexport) CSqliteCtrl 
{
public:
	CSqliteCtrl(void);
	virtual ~CSqliteCtrl(void);

public:
	bool	Init();
	bool    Exit();

	static UINT WINAPI ThreadUpdateValueFunc(LPVOID lparam);
	void	TerminateUpdateValueThread();
	void	UpdateValue();

	bool	SetValue(wstring strName, wstring strValue);

	static UINT WINAPI ThreadUpdatePointFunc(LPVOID lparam);
	void	TerminateUpdatePointThread();
	void	UpdatePoint();

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );

	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	
	void	SetIndex(int nIndex);
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
public:
	string		m_strS3dbPath;				//s3db�ļ�λ��
	string		m_strS3dbPsw;				//s3db����
	string		m_strS3dbTName;				//s3db����
	string		m_strS3dbNameColumn;		//s3db����lie
	string		m_strS3dbValueColumn;		//s3dbֵ��
	int			m_nS3dbInterval;			//s3dbˢ�¼��
	HANDLE		m_hupdatethread;
	HANDLE		m_hupdatepointthread;
	bool		m_bExitUpdateThread;
	vector<DataPointEntry> m_pointlist;	//���.
	Project::Tools::Mutex	m_lock;
	Project::Tools::Mutex	m_dblock;
	Beopdatalink::CLogDBAccess* m_logsession;
	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
	int				m_nEngineIndex;
	map<wstring,wstring>	m_runTimeDataMap;
};
