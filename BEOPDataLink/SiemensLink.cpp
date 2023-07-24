#include "StdAfx.h"
#include "SiemensLink.h"

#include "../COMdll/S7UDPCtrl.h"

CSiemensLink::CSiemensLink(CBEOPDataLinkManager* datalinker, vector<_SiemensPLCProp> propPLCList,Beopdatalink::CCommonDBAccess*	dbsession_history)
	:m_datalinker(datalinker)
	,m_dbsession_history(dbsession_history)
{

	m_hDataReadThread = NULL;
	m_hDataCheckRetryConnectionThread = NULL;

	m_bExitThread = false;
	
	for(int i=0;i< propPLCList.size();i++)
	{
		CS7UDPCtrl *pCtrl = new CS7UDPCtrl(propPLCList[i].strIP,  propPLCList[i].nSlack, propPLCList[i].nSlot);
		pCtrl->SetHistoryDbCon(m_dbsession_history);
		m_mapS7Ctrl[propPLCList[i].strIP] = pCtrl;

		m_pS7CtrlList.push_back(pCtrl); //删除将在这里维护，golding ,map容易出现一个ip对多个指针，就会内存泄漏
	}

}


CSiemensLink::~CSiemensLink(void)
{
	unsigned int i = 0;
	for(i=0;i<m_pS7CtrlList.size();i++)
	{
		SAFE_DELETE(m_pS7CtrlList[i]);
	}

	m_pS7CtrlList.clear();
}

void CSiemensLink::SetPointList( const std::vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

bool CSiemensLink::Init()
{
	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	int i=0;
	while(iter3 != m_mapS7Ctrl.end())
	{
		CS7UDPCtrl *pS7Ctrl = iter3->second;

		std::vector<DataPointEntry> tempSiemenslist;
		for(int j=0;j<m_pointlist.size();j++)
		{
			if(m_pointlist[j].GetParam(3) == pS7Ctrl->GetIP())
			{
				tempSiemenslist.push_back(m_pointlist[j]);
			}
		}
		pS7Ctrl->SetPointList(tempSiemenslist);
		pS7Ctrl->SetLogSession(m_logsession);
		pS7Ctrl->SetIndex(i);
		pS7Ctrl->Init();
		pS7Ctrl->UpdateSimensTime();
		iter3++;
		i++;
	}


	m_hDataReadThread = (HANDLE) _beginthreadex(NULL, 0, ThreadReadAllPLCData, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	if (!m_hDataReadThread)
	{
		return false;
	}

}



void	CSiemensLink::TerminateReadThread()
{
	m_bExitThread = true;
	WaitForSingleObject(m_hDataReadThread, INFINITE);
	WaitForSingleObject(m_hDataCheckRetryConnectionThread, INFINITE);
}


UINT WINAPI CSiemensLink::ThreadReadAllPLCData(LPVOID lparam)
{
	CSiemensLink* pthis = (CSiemensLink*) lparam;
	if (pthis != NULL)
	{

		while (!pthis->m_bExitThread)
		{

			map<wstring,CS7UDPCtrl*>::iterator iter3 = pthis->m_mapS7Ctrl.begin();
			while(iter3 != pthis->m_mapS7Ctrl.end())
			{
				CS7UDPCtrl *pS7Ctrl = iter3->second;
				Project::Tools::Scoped_Lock<Mutex>	scopelock(pthis->m_lock);

				if(pS7Ctrl->GetReadOneByOneSuccess())		//第一次逐个读取成功后
				{
					if(!pS7Ctrl->ReadDataOnce())
						pS7Ctrl->SetPLCError();
				}
				else
				{
					if(!pS7Ctrl->ReadOneByOneAndRemoveErrPoint())		//逐个读取去除错误点
					{
						pS7Ctrl->SetPLCError();
					}
				}
				iter3++;
			}

			Sleep(1000);
		}
	}

	return 0;
}

UINT WINAPI CSiemensLink::ThreadCheckAndRetryConnection(LPVOID lparam)
{
	CSiemensLink* pthis = (CSiemensLink*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			//读Simens 失败次数重连间隔
			int nReadErrConnectCount = pthis->GetErrrConnectCount();
			map<wstring,CS7UDPCtrl*>::iterator iter3 = pthis->m_mapS7Ctrl.begin();
			while(iter3 != pthis->m_mapS7Ctrl.end())
			{
				CS7UDPCtrl *pS7Ctrl = iter3->second;
				Project::Tools::Scoped_Lock<Mutex>	scopelock(pthis->m_lock);
				if(pS7Ctrl->GetErrCount()>=nReadErrConnectCount || !pS7Ctrl->GetConnectSuccess())
				{
					pS7Ctrl->ExitPLCConnection();
					pS7Ctrl->InitPLCConnection();
					pS7Ctrl->ClrPLCError();
				}

				iter3++;
			}

			
			Sleep(10000);
		}
	}

	return 0;
}


bool CSiemensLink::Exit()
{
	TerminateReadThread();

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		if(iter3->second)
		{
			iter3->second->Exit();
		}

		iter3++;
	}

	m_mapS7Ctrl.clear();

	return true;
}

void CSiemensLink::GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		iter3->second->GetValueSet(valuelist);

		iter3++;
	}
}

bool CSiemensLink::SetValue( const wstring& pointname, double fValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		bool bSetValueSuccess =  iter3->second->SetValue(pointname, fValue);

		if(bSetValueSuccess)
			return true;

		iter3++;
	}

	return false;
}

bool CSiemensLink::SetValue( const wstring& pointname, float fValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		bool bSetValueSuccess =  iter3->second->SetValue(pointname, fValue);

		if(bSetValueSuccess)
			return true;

		iter3++;
	}

	return false;
}

void CSiemensLink::InitDataEntryValue( const wstring& pointname, double fValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		iter3->second->InitDataEntryValue(pointname, fValue);

		iter3++;
	}


}

bool CSiemensLink::GetValue( const wstring& pointname, double& fValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		if(iter3->second->GetValue(pointname, fValue))
			return true;

		iter3++;
	}

	return false;
}

void CSiemensLink::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

vector<string> CSiemensLink::GetExecuteOrderLog()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	vector<string> vecLog;
	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		string strLog = iter3->second->GetExecuteOrderLog();
		if(strLog.length()>0)
			vecLog.push_back(strLog);
		iter3++;
	}
	return vecLog;
}

int CSiemensLink::GetErrrConnectCount()
{
	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		wstring wstrReadErrConnectCount = m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"errrconnectcount");
		if(wstrReadErrConnectCount == L"" || wstrReadErrConnectCount == L"0")
		{
			wstrReadErrConnectCount = L"30";
			m_dbsession_history->WriteCoreDebugItemValue(L"errrconnectcount",wstrReadErrConnectCount);
		}
		return _wtoi(wstrReadErrConnectCount.c_str());
	}
	return 30;
}

