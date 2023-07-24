#pragma once

#include "../BEOPDataEngineCore/BEOPDataEngineCore.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "../BEOPDataLink/BEOPDataLinkManager.h"
#include "../BEOPDataEngineCore/RedundencyManager.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../BEOPDataEngineCore/BEOPLogicManager.h"

#include "CoreFileTransfer.h"
#include "../BEOPDataPoint/DataPointManager.h"
#include "../BEOPDataEngineCore/LogicDllThread.h"
#include "../DB_BasicIO/BEOPDataBaseInfo.h"
#include "../Tools/Util/UtilsFile.h"


class CBeopGatewayCoreWrapper
{
public:
	CBeopGatewayCoreWrapper(void);
	~CBeopGatewayCoreWrapper(void);

	bool 			Run();
	bool 			Init(wstring strDBFilePath);
	bool 			Exit();
	bool 			Release();
	bool 			Reset();


	bool 			UpdateInputOutput();
	bool 			UpdateS3DBDownloaded();
	bool 			UpdateS3DBUploaded();
	bool 			UpdateSiteModeChanged();
	bool 			UpdateResetCommand();

	////20171018//////////////////////////////////////////////////////////////////////
	bool			UpdateSaveLog();				//���´洢log�ܿ���
	//////////////////////////////////////////////////////////////////////////

	bool 			InitCheckMySQLDB(wstring wstrServerIP, wstring wstrDBName);
	bool 			InitS3DBDownloadManager();
	wstring			PreInitFileDirectory();
	wstring			PreInitFileDirectory_4db();
	void 			SaveAndClearLog();
	void 			PrintLog(const wstring &strLog,bool bSaveLog = true);
	
	bool			SumRestartTime(int nMinute = 40);					//ͳ��ʱ�����Զ�����3������ ��������Ҫ�ӳ�1����
	string			GetHostIP();
	std::wstring	Replace(const wstring& orignStr, const wstring& oldStr, const wstring& newStr, bool& bReplaced);
	bool			OutPutLogString(CString strOut);
	void 			GetHostIPs(vector<string> & IPlist); //��IP��core�Ļ�ȡ

public:
	CBeopDataEngineCore *	m_pDataEngineCore;
	CBEOPDataAccess *		m_pDataAccess_Arbiter;
	CRedundencyManager *	m_pRedundencyManager;
	CBEOPLogicManager *		m_pLogicManager;
	CCoreFileTransfer  *	m_pFileTransferManager;
	CCoreFileTransfer *		m_pFileTransferManagerSend;

	HANDLE					m_hEventActiveStandby;
	HANDLE 					m_Backupthreadhandle;
	
	bool 					m_bFirstInit;
	bool 					m_bExitCore;
	bool 					m_bFitstStart;

	vector<SYSTEMTIME>		g_tLogAll;
	vector<wstring>			g_strLogAll;
	vector<COleDateTime>	m_vecReStartTime;			//�Զ�����ʱ��

	wstring 				m_strDBFilePath;
	int						m_nDBFileType; //0:s3db, 1:4db;
	wstring  				m_strIniFilePath;
	wstring  				m_strWatchExePath;
	wstring  				m_strLogFilePath;
	SYSTEMTIME  			m_stNow;
	CProcessView			m_PrsV;
	COleDateTime			m_oleStartTime;
	COleDateTime			m_oleRunTime;				//����ʱ��
	COleDateTime			m_oleCheckEveryHourTime;				//����ʱ��
	int						m_nDelayWhenStart;
};

