#include "StdAfx.h"
#include "BeopGatewayCoreWrapper.h"
#include "DumpFile.h"
#include "../COMdll/ModbusTcp/DogTcpCtrl.h"

CBeopGatewayCoreWrapper::CBeopGatewayCoreWrapper(void)
{

	m_pDataEngineCore = NULL;
	 m_pDataAccess_Arbiter = NULL;
	m_pLogicManager = NULL;
	m_bFirstInit = true;
	m_bExitCore = false;
	GetLocalTime(&m_stNow);
}

CBeopGatewayCoreWrapper::~CBeopGatewayCoreWrapper(void)
{
}

bool CBeopGatewayCoreWrapper::Run()
{
	//��ʼ��Dump�ļ�
	DeclareDumpFile();

	HANDLE m_hmutex_instancelock = CreateMutex(NULL, FALSE, L"BEOPLogicEngine");
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		PrintLog(L"ERROR: Another BEOPLogicEngine is running, Start failed.\r\n",false);
		Sleep(10000);
		exit(0);
		return false;
	}

	wchar_t wcChar[1024];
	COleDateTime tnow = COleDateTime::GetCurrentTime();
	wsprintf(wcChar, L"---- BEOP BEOPLogicEngine(%s) starting ----\r\n", CORE_VERSION);
	PrintLog(wcChar,false);

	PrintLog(L"INFO : Check Database Status...\r\n",false);
	m_strDBFilePath = PreInitFileDirectory();

	while(!Init(m_strDBFilePath))
	{
		Release();
		PrintLog(L"ERROR: Init Failed, Try again after 30 seconds automatic...\r\n",false);
		Sleep(30000);
	}

	OutPutLogString(_T("BEOPLogicEngine.exe start"));

	m_bFirstInit = false;

	//��ʼ��dog
	CDogTcpCtrl::GetInstance()->SetDogMode(E_SLAVE_LOGIC);			//logicģʽ
	
	FeedDog();

	while(!m_bExitCore)
	{
		Sleep(2000);
		
		GetLocalTime(&m_stNow);

		//���·�����ʱ��
		wstring strTimeServer;
		Project::Tools::SysTime_To_String(m_stNow, strTimeServer);
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicservertime", strTimeServer);

		//ι��
		FeedDog();
		
		//update logic
		UpdateCheckLogic();

		//update restart
		UpdateResetCommand();

		//save log
		wstring strAlwaysLog = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"alwayslog");
		if(strAlwaysLog==L"1")
		{
			SaveAndClearLog();
		}
	}
	return true;
}

bool CBeopGatewayCoreWrapper::UpdateResetCommand()
{
	if(m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"resetlogic")==L"1")
	{
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"resetlogic", L"0");
		PrintLog(L"INFO : Reset Signal Recved, Restarting...\r\n");
		Reset();
		PrintLog(L"INFO : logic engine restarted, initializing...\r\n\r\n");
		OutPutLogString(_T("BEOPLogicEngine.exe manual restart"));
		Init(m_strDBFilePath);
	}

	return true;
}

bool CBeopGatewayCoreWrapper::Reset()
{

	PrintLog(L"INFO : Start Unloading Logic Threads...");
	
	if(m_pLogicManager)
	{
		int nDllCount = m_pLogicManager->GetAllImportDll();
		wstring strPrefix = L"Unloading Threads:";
		for(int i=0; i<m_pLogicManager->m_vecDllThreadList.size(); i++)
		{
			CLogicDllThread *pLogicThread = m_pLogicManager->m_vecDllThreadList[i];
			wstring strDllName = pLogicThread->GetName();
			wstring strOut = strPrefix + strDllName ;
			bool bSuccess = pLogicThread->Exit();
			if(bSuccess)
				strOut+= L" Success.\r\n";
			else
				strOut+= L" Failed ERROR!!!\r\n";
			PrintLog(strOut);
		}

		PrintLog(L"INFO : Unloading logic Manager: ");
		if(m_pLogicManager->Exit())
			PrintLog(L"Success. \r\n");
		else
			PrintLog(L"Failed ERROR!!! \r\n");

		delete(m_pLogicManager);
		m_pLogicManager = NULL;
	}
	else
		PrintLog(L"ERROR.\r\n");

	PrintLog(L"INFO : Exit DataEngineCore...");
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->SetDBAcessState(false);
		m_pDataEngineCore->TerminateScanWarningThread();
		m_pDataEngineCore->TerminateReconnectThread();

		m_pDataEngineCore->Exit();
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		PrintLog(L"Successfully.\r\n");
	}
	else
		PrintLog(L"ERROR.\r\n");
	
	PrintLog(L"INFO : Disconnect Database for Logic...");
	if(m_pDataAccess_Arbiter)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
		PrintLog(L"Successfully\r\n");
	}
	else
		PrintLog(L"ERROR.\r\n");

	return true;

}

wstring CBeopGatewayCoreWrapper::PreInitFileDirectory()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);

	wstring strDBFileName;


		CFileFind s3dbfilefinder;
		wstring strSearchPath = cstrFile + L"\\*.s3db";
		wstring filename;
		BOOL bfind = s3dbfilefinder.FindFile(strSearchPath.c_str());
		wstring SourcePath, DisPath;
		vector<wstring> strAvailableFileNameList;
		while (bfind)
		{
			bfind = s3dbfilefinder.FindNextFile();
			filename = s3dbfilefinder.GetFileName();

			strAvailableFileNameList.push_back(filename);
		}


		if(strAvailableFileNameList.size()>0)
		{
			strDBFileName = cstrFile + L"\\" + strAvailableFileNameList[0];
			for(int nFileIndex=1; nFileIndex<strAvailableFileNameList.size();nFileIndex++)
			{
				wstring strFileNameToDelete = cstrFile + L"\\" + strAvailableFileNameList[nFileIndex];
				if(!UtilFile::DeleteFile(strFileNameToDelete.c_str()))
				{
					_tprintf(L"ERROR: Delete UnNeccesary DB Files Failed!");
				}
			}
		}

	

	//logĿ¼
	wstring strLogDir = cstrFile + L"\\Temp";
	if(!UtilFile::DirectoryExists(strLogDir))
	{
		UtilFile::CreateDirectory(strLogDir);
	}
	wstring strDBFileDir = cstrFile + L"\\DBFileVersion";
	if(!UtilFile::DirectoryExists(strDBFileDir))
	{
		UtilFile::CreateDirectory(strDBFileDir);
	}

	return strDBFileName;
}

bool CBeopGatewayCoreWrapper::InitCheckMySQLDB(wstring wstrServerIP,wstring wstrDBName)
{

	m_pDataEngineCore->m_dbset.strDBIP = Project::Tools::WideCharToAnsi(wstrServerIP.c_str());

	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\beopgateway.ini";
	wchar_t wcChar[1024];
	wchar_t charDBBuild[1024];

	GetPrivateProfileString(L"beopgateway", L"dbbuild", L"", charDBBuild, 1024, exepath.c_str());
	wstring wstDBBuild = charDBBuild;
	WritePrivateProfileString(L"beopgateway", L"dbbuild", wstDBBuild.c_str(), exepath.c_str());
	//�����ݿ�
	CString strLog;
	strLog.Format(_T("INFO : Connect to Server Database: %s\r\n"),wstrServerIP.c_str());
	PrintLog(strLog.GetString(),false);

	wsprintf(wcChar, L"INFO : Check DB On Server: %s...\r\n", wstrServerIP.c_str());
	PrintLog(wcChar,false);
	bool bMySQLbeopDBExist = m_pDataEngineCore->CheckDBExist(wstrDBName);
	if(!bMySQLbeopDBExist)
	{
		wstring strErr = L"ERROR: " + wstrDBName + L" database connect failed.\r\n";
		PrintLog(strErr);
		return false;
	}
	return true;
}

bool CBeopGatewayCoreWrapper::Init(wstring strDBFilePath)
{
	if(strDBFilePath.length()<=0)
		return false;

	wchar_t wcChar[1024];
	if(m_pDataEngineCore == NULL)
		m_pDataEngineCore = new CBeopDataEngineCore(strDBFilePath);

	if(m_pDataEngineCore->m_strDBFileName.length()<=0)
	{
		PrintLog(L"ERROR: None DB File Found.\r\n");
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}

	wsprintf(wcChar, L"INFO : Reading DBFile:%s\r\n", m_pDataEngineCore->m_strDBFileName.c_str());
	PrintLog(wcChar,false);

	//��ȡ���������ݿ�
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\beopgateway.ini";
	m_strLogFilePath  = cstrFile + L"\\beopwatch.log";

	wchar_t charDBName[1024];
	GetPrivateProfileString(L"beopgateway", L"defaultdb", L"", charDBName, 1024, exepath.c_str());
	wstring wstrDBName = charDBName;
	if(wstrDBName == L"" || wstrDBName == L"0")
		wstrDBName = L"beopdata";
	
	WritePrivateProfileString(L"beopgateway", L"defaultdb", wstrDBName.c_str(), exepath.c_str());

	//init dataaccess
	gDataBaseParam realDBParam;
	m_pDataEngineCore->ReadRealDBparam(realDBParam,wstrDBName);
	if(realDBParam.strDBIP.length()<=0 ||
		realDBParam.strUserName.length()<=0)
	{
		PrintLog(L"ERROR: Read s3db Info(IP,Name) Failed, Please Check the s3db File.\r\n");	//edit 2016-03-11 S3db���ݱ�project_config��Ϣ�����ˣ�
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}

	//����ini�ļ� ������ʱ��
	GetLocalTime(&m_stNow);
	wstring strTimeServer;
	Project::Tools::SysTime_To_String(m_stNow, strTimeServer);

	wchar_t charServerIP[1024];
	GetPrivateProfileString(L"beopgateway", L"server", L"", charServerIP, 1024, exepath.c_str());
	wstring wstrServerIP = charServerIP;

	if(wstrServerIP.length()<=7)
	{
		wstrServerIP = L"localhost";
	}

	realDBParam.strDBIP = Project::Tools::WideCharToAnsi(wstrServerIP.c_str());
	if(m_bFirstInit && !InitCheckMySQLDB(wstrServerIP,wstrDBName))
	{
		PrintLog(L"ERROR: Check MySQL DB Failed, Please Check the MySQL installation.\r\n");
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}


	if(m_pDataAccess_Arbiter == NULL)
		m_pDataAccess_Arbiter = new CBEOPDataAccess;
	m_pDataAccess_Arbiter->SetDBparam(realDBParam);
	if(m_pDataAccess_Arbiter->InitConnection(true, true, true)==false)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
		return false;
	}

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicservertime",strTimeServer);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicstarttime",strTimeServer);		//Core ����ʱ��
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicversion",CORE_VERSION);			//д��Core�İ汾��

	PrintLog(L"INFO : Init Logic Engine...\r\n",false);
	m_pDataEngineCore->InitLogic();
	if(!m_pDataEngineCore->InitDBConnection())
	{
		PrintLog(L"ERROR: InitDBConnection Failed.\r\n");
		return false;
	}

	//init dataenginecore
	m_pDataEngineCore->SetDataAccess(m_pDataAccess_Arbiter);

	m_pDataEngineCore->SetDBAcessState(true);

	PrintLog(L"INFO : Reading Logics...",false);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runlogic", L"0");
	if(ReadLogic())
		PrintLog(L"INFO : Reading Logics Success!\r\n",false);
	else
		PrintLog(L"INFO : Reading Logics Failed! No Logic will run, Please check logic file in DBFile.\r\n",false);

	//clear unit02 when starting
	m_pDataAccess_Arbiter->ClearLogicParameters();

	COleDateTime tnow = COleDateTime::GetCurrentTime();
	wchar_t wcTemp[1024];
	wsprintf(wcTemp, L"INFO : %s engine started successfully!\r\n", tnow.Format(L"%Y-%m-%d %H:%M:%S ") );
	PrintLog(wcTemp);
	return true;
}

void CBeopGatewayCoreWrapper::UpdateCheckLogic()
{
	if(m_pLogicManager==NULL)
		return;

	int i = 0;
	wchar_t wsChar[1024];
	for (i=0;i<m_pLogicManager->m_vecDllThreadList.size();++i)
	{	
		CLogicDllThread *pDllThread = m_pLogicManager->m_vecDllThreadList[i];
		wstring strDllRunValue = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(pDllThread->GetName());
		if(strDllRunValue==L"1" &&pDllThread->GetRunStatus()==false)
		{
			wsprintf(wsChar, L"INFO : Logic %s Thread Loaded.\r\n",  pDllThread->GetName().c_str() );
			PrintLog(wsChar,false);
			pDllThread->SetRunStatus(true);		
		}
		else if(strDllRunValue==L"0" && pDllThread->GetRunStatus()==true)
		{
			wsprintf(wsChar, L"INFO : Logic %s Thread UnLoaded.\r\n", pDllThread->GetName().c_str());
			PrintLog(wsChar,false);
			pDllThread->SetRunStatus(false);		
		}
	}

	//ɨ������䶯
	wstring strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType;
	if(m_pDataAccess_Arbiter->ReadLogicParameters(strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType))
	{
	
		//�޸�db�ļ�
		if(strSetType==L"0")//�޸�����
		{
			PrintLog(L"INFO : Logic Parameter Input Changed, Updating...\r\n");
			CDLLObject *pObject = m_pLogicManager->GetDLLObjectByName(strLogicName.c_str());
			if(pObject)
			{
				m_pLogicManager->SaveParametertoDB(0, strLogicName.c_str(), strVariableName.c_str(), strLinkName.c_str(), strLinkType.c_str());
				pObject->SetDllInputParamter(strVariableName.c_str(), strLinkName.c_str(), strLinkType.c_str());//�޸��ڴ�

			}
		}
		else if(strSetType==L"1")//�޸����
		{
			PrintLog(L"INFO : Logic Parameter Output Changed, Updating...\r\n");
			CDLLObject *pObject = m_pLogicManager->GetDLLObjectByName(strLogicName.c_str());
			if(pObject)
			{
				m_pLogicManager->SaveParametertoDB(1, strLogicName.c_str(), strVariableName.c_str(), strLinkName.c_str(), strLinkType.c_str());
				pObject->SetDllOutputParamter(strVariableName.c_str(), strLinkName.c_str(), strLinkType.c_str());//�޸��ڴ�
			}
		}
		else if(strSetType==L"2")//�޸Ĳ���ʱ��
		{
			PrintLog(L"INFO : Logic Parameter Period Changed, Updating...\r\n");
			CDLLObject *pObject = m_pLogicManager->GetDLLObjectByName(strLogicName.c_str());
			if(pObject)
			{

				double fTimeSpan = _wtof(strVariableName.c_str());
				if(fTimeSpan<=0.f)
					fTimeSpan = 20.f;
				m_pLogicManager->SaveSpanTimetoDB(strLogicName.c_str(), _wtof(strVariableName.c_str()));
				m_pLogicManager->SetTimeSpan(pObject, fTimeSpan); //ʵʱ�����޸�ʱ�䣬������Ч����������
			}
		}
		else if(strSetType==L"3")//����dll�ļ�
		{
			PrintLog(L"INFO : Logic File Changed, Updating...\r\n");
			//�ȸ����ļ����������ļ�(mysql)
			if(strVariableName.length()>0)
			{
				//int nFileID = _wtoi(strVariableName.c_str());
				wstring cstrFile;
				Project::Tools::GetSysPath(cstrFile);
				wstring strDllPath;
				strDllPath = cstrFile + L"\\Temp\\";
				strDllPath += strVariableName.c_str();
				if(m_pDataAccess_Arbiter->DownloadFile(strVariableName, strDllPath))
				{
					bool bUpdate = m_pLogicManager->UpdateDllObjectByFile(strVariableName, strDllPath);
					if(bUpdate)
						PrintLog(L"INFO : Logic File Download and Update Successfully.\r\n");
					else
						PrintLog(L"ERROR: Logic File Download and Update Failed.\r\n");

					m_pDataAccess_Arbiter->DeleteFile(strVariableName);
				}
			}
					
		}
		

		m_pDataAccess_Arbiter->DeleteLogicParameters(strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType);
	}

}

bool CBeopGatewayCoreWrapper::ReadLogic()
{
	m_pLogicManager = new CBEOPLogicManager(m_pDataEngineCore->m_strDBFileName, NULL);
	if(m_pLogicManager)
	{
		m_pLogicManager->m_pDataAccess = m_pDataAccess_Arbiter ;
		m_pLogicManager->InitSchedule();
		bool bReadInDB = m_pLogicManager->ReadDLLfromDB(m_pDataEngineCore->m_strDBFileName,m_pLogicManager->vecImportDLLList);
		if(!bReadInDB)
			return false;

		if(m_pLogicManager->vecImportDLLList.size() > 0)
			m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runlogic", L"1");

		bool bLoad = m_pLogicManager->LoadDllDetails();
		if(!bLoad)
			return false;

		m_pLogicManager->InitRelationships();

		int i =0;
		for (i=0;i<m_pLogicManager->m_vecDllThreadList.size();++i)
		{	
			CLogicDllThread *pDllThread = m_pLogicManager->m_vecDllThreadList[i];
			if(!m_pDataEngineCore->m_bUpdateInputTableFromSite)
			{//����ģʽ��ֱ�����в���
				wstring strRunnableValue = L"0";
				bool bRead = m_pDataAccess_Arbiter->ReadCoreDebugItemValue(pDllThread->GetName(), strRunnableValue);
				if(bRead)
				{//��������ֵ����Ϊ�Ѿ����ù�����ô�����ϴεĿ���
					if(strRunnableValue==L"1")
						pDllThread->SetRunStatus(true);
					else
						pDllThread->SetRunStatus(false);
				}
				else
				{//����������ֵ�Ļ�������Ϊ���ݿ�ս�����demo���÷��棬Ĭ��Ӧ�ô����в���
					m_pDataAccess_Arbiter->WriteCoreDebugItemValue(pDllThread->GetName(), L"1");
					pDllThread->SetRunStatus(true);
				}
			}
			else
			{	
				//�ֳ�ģʽ�£�ֻ�ж����������Դ򿪲ſ������������Ĭ�Ϲر�
				wstring strDllRunValue = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(pDllThread->GetName());

				if(strDllRunValue==L"1")
					pDllThread->SetRunStatus(true);
				else
					pDllThread->SetRunStatus(false);
			}

		}

		for (i=0;i<m_pLogicManager->m_vecDllThreadList.size();++i)
		{
			CLogicDllThread *pDllThread = m_pLogicManager->m_vecDllThreadList[i];

			PrintLog(pDllThread->GetStructureString(),false);
			pDllThread->StartThread(); //�����߳����������ǲ��Կ��������ڲ�����

		}

		return true;
	}
	return false;
}

void CBeopGatewayCoreWrapper::SaveAndClearLog()
{
	if(g_strLogAll.size()<=0)
		return;

	if(m_pDataAccess_Arbiter)
		m_pDataAccess_Arbiter->InsertLog(g_tLogAll, g_strLogAll);

	g_strLogAll.clear();
	g_tLogAll.clear();

}

void CBeopGatewayCoreWrapper::PrintLog(const wstring &strLog,bool bSaveLog)
{
	_tprintf(strLog.c_str());
	if(bSaveLog)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		g_tLogAll.push_back(st);
		g_strLogAll.push_back(strLog);
		if(g_strLogAll.size()>=100)
		{
			// ����һ��������������¼�����ݿ⣬���
			SaveAndClearLog();
		}
	}
}

std::wstring CBeopGatewayCoreWrapper::Replace( const wstring& orignStr, const wstring& oldStr, const wstring& newStr, bool& bReplaced )
{
	size_t pos = 0;
	wstring tempStr = orignStr;
	wstring::size_type newStrLen = newStr.length();
	wstring::size_type oldStrLen = oldStr.length();
	bReplaced = false;
	while(true)
	{
		pos = tempStr.find(oldStr, pos);
		if (pos == wstring::npos) break;

		tempStr.replace(pos, oldStrLen, newStr);        
		pos += newStrLen;
		bReplaced = true;
	}

	return tempStr; 
}

bool CBeopGatewayCoreWrapper::OutPutLogString( CString strOut )
{
	COleDateTime tnow = COleDateTime::GetCurrentTime();
	CString strLog;
	strLog += tnow.Format(_T("%Y-%m-%d %H:%M:%S "));
	strLog += strOut;
	strLog += _T("\n");

	//��¼Log
	CStdioFile	ff;
	if(ff.Open(m_strLogFilePath.c_str(),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
	{
		ff.Seek(0,CFile::end);
		ff.WriteString(strLog);
		ff.Close();
		return true;
	}
	return false;
}

bool CBeopGatewayCoreWrapper::Exit()
{
	m_bExitCore = true;
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->Exit();
		delete m_pDataEngineCore;
		m_pDataEngineCore = NULL;
	}

	//dog�˳�
	CDogTcpCtrl::GetInstance()->Exit();
	CDogTcpCtrl::GetInstance()->DestroyInstance();

	return true;
}

bool CBeopGatewayCoreWrapper::FeedDog()
{
	return CDogTcpCtrl::GetInstance()->SendActiveSignal();
}

bool CBeopGatewayCoreWrapper::Release()
{
	if(m_pLogicManager)
	{
		delete m_pLogicManager;
		m_pLogicManager = NULL;
	}

	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->SetDBAcessState(false);
		m_pDataEngineCore->TerminateScanWarningThread();
		m_pDataEngineCore->TerminateReconnectThread();

		m_pDataEngineCore->Exit();
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;

	}

	if(m_pDataAccess_Arbiter)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
	}

	return true;
}