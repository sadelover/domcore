#include "StdAfx.h"
#include "OPCLink.h"
#include "../COMdll/DebugLog.h"
#include "../OPCdll/OPCCtrl.h"
#include "BEOPDataLinkManager.h"
#include "Tools/EngineInfoDefine.h"

COPCEngine::COPCEngine(CBEOPDataLinkManager* datalinker,Beopdatalink::CCommonDBAccess*	dbsession_history, wstring strOPCServerIP,int nSleep,int nMutilCount,bool bUseThread,int nSleepFromDevice,int nPollSleep,int nPollSleepFromDevice,bool bDisableCheckQuality,int nPrecision,bool bAsync20Mode,int nRefresh20Interval, int nLanguageID, int nMultilAddItems, int nDebugOPC, int nUpdateRate)
	:m_datalinker(datalinker)
	,m_opcsession(NULL)
	,m_strOPCServerIP(strOPCServerIP)
	,m_bMultiClientMode(false)
	,m_bConnectOK(false)
	,m_hDataReadThread(NULL)
	,m_hDataReadDeviceThread(NULL)
	,m_hOPCConnectThread(NULL)
	,m_bExitThread(false)
	,m_bUseThread(bUseThread)
	,m_nMutilCount(nMutilCount)
	,m_nSleep(nSleep)
	,m_nReSponse(0)
	,m_nReadCount(0)
	,m_nReadDeviceCount(0)
	,m_nReSponseDevice(0)
	,m_nOPCClientIndex(-1)
	,m_nOPCCmdSleepFromDevice(nSleepFromDevice)
	,m_nOPCPollSleep(nPollSleep)
	,m_nOPCPollSleepFromDevice(nPollSleepFromDevice)
	,m_nOPCReconnectInertval(30)
	,m_nOPCCheckReconnectInterval(5)
	,m_bReconnect(false)
	,m_nReconnectMinute(0)
	,m_dbsession_history(dbsession_history),
	m_nOPCMainThreadOPCPollSleep(2),
	m_bEnableSecurity(false),
	m_nMainUpdateIndex(0),
	m_nSingleReadCount(0),
	m_bDisableCheckQuality(bDisableCheckQuality),
	m_nReloadErrItem(0),
	m_nPrecision(nPrecision),
	m_bAsync20Mode(bAsync20Mode),
	m_nRefresh20Interval(nRefresh20Interval),
	m_nLanguageID(nLanguageID),
	m_nMultiAddItems(nMultilAddItems),
	m_nDebugOPC(nDebugOPC),
	m_nUpdateRate(nUpdateRate)
{
	if(m_nOPCCmdSleepFromDevice <= 0)
		m_nOPCCmdSleepFromDevice = 50;

	if(m_nOPCPollSleepFromDevice <= 0)
		m_nOPCPollSleepFromDevice = 60;

	m_oleDeviceTime = COleDateTime::GetCurrentTime();
	m_oleUpdateTime = COleDateTime::GetCurrentTime(); 
	m_oleReConnectTime = COleDateTime::GetCurrentTime();
	m_oleRefresh20 = COleDateTime::GetCurrentTime() - COleDateTimeSpan(1,0,0,0);
	m_PositionValuemap.clear();
}

COPCEngine::~COPCEngine(void)
{
	ClearResource();
}

void COPCEngine::SetOpcPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

bool COPCEngine::Init(int nOPCClientIndex)
{
	if (m_pointlist.empty())
	{
		return false;
	}
	m_PositionValuemap.clear();
	m_nOPCClientIndex = nOPCClientIndex;
	m_nSingleReadCount = 0;
	const DataPointEntry& entry = m_pointlist[0];
	m_opcsession = CreateOPCObject(entry.GetServerType());
	ASSERT(m_opcsession);
	if (!m_opcsession)
	{
		return false;
	}

	wstring progname = entry.GetServerProgName();
	if (progname.empty())
	{
		return false;
	}

	m_opcsession->SetPointList(m_pointlist);
	m_opcsession->SetServerProgName(entry.GetServerProgName());

	m_bConnectOK = m_opcsession->Init(m_strOPCServerIP,m_bDisableCheckQuality,nOPCClientIndex,m_nUpdateRate,m_nLanguageID,m_nMultiAddItems,m_nDebugOPC);
	if(!m_bConnectOK)
	{
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_CONNECT_OPC,true);
	}
	m_hOPCConnectThread = (HANDLE) _beginthreadex(NULL, 0, ThreadConnectOPCData, this, NORMAL_PRIORITY_CLASS, NULL);
	if(nOPCClientIndex>=0 && m_bUseThread )
	{
		m_hDataReadThread = (HANDLE) _beginthreadex(NULL, 0, ThreadUpdateOPCData, this, NORMAL_PRIORITY_CLASS, NULL);
	}
	return m_bConnectOK;
}
//
//void COPCEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
//{
//	CString strTemp;
//	strTemp.Format(_T("COPCEngine::GetValueSet Once. m_nMainUpdateIndex:%d\r\n"), m_nMainUpdateIndex);
//	_tprintf(strTemp.GetString());
//
//	//20180926////////////////////////////////////////////////////////////////////////
//	//�첽дֵ�� ÿ����ֵʱ�� ��������ȡ�����ڴ����첽�ص�д�ɹ��ĵ�λ
//	ReReadAsync20Item();
//	//////////////////////////////////////////////////////////////////////////
//
//	//���첽��ͬ����д
//	if(m_bAsync20Mode)				//�첽��ʽ��ȡ
//	{
//		COleDateTime oleCurrent = COleDateTime::GetCurrentTime();
//		COleDateTimeSpan oleSpan = oleCurrent - m_oleRefresh20;
//		if(oleSpan.GetTotalMinutes() >= m_nRefresh20Interval)
//		{
//			Refresh20();
//			m_oleRefresh20 = oleCurrent;
//		}
//
//		//ÿ�������� �첽ˢ��һ��
//		UpdateOPCDataBuffer();
//	}
//	else
//	{
//		if(m_bUseThread && !m_bEnableSecurity)		//�����߳���δ���ý��̰�ȫ����  ���������߳�������ȡ
//		{
//			//���߳���ִ�� UpdateMutilOPCData
//		}
//		else if(m_bUseThread && m_bEnableSecurity)		//�����߳������ý��̰�ȫ����  ֻ������������������ȡ
//		{
//			int nUpdateInterval = (m_nOPCMainThreadOPCPollSleep/2>0)?m_nOPCMainThreadOPCPollSleep/2:1;
//			if(m_nMainUpdateIndex%nUpdateInterval == 0)
//			{
//				CString strlog;
//				strlog.Format(_T("Start Read Update Mutil OPC Data(m_nOPCMainThreadOPCPollSleep:%d,m_nMainUpdateIndex:%d) Once\r\n"), m_nOPCMainThreadOPCPollSleep, m_nMainUpdateIndex);
//				_tprintf(strlog.GetString());
//				UpdateMutilOPCData();
//			}
//
//			m_nMainUpdateIndex++;
//			if(m_nMainUpdateIndex >= 100000)
//				m_nMainUpdateIndex = 0;
//		}
//		else
//		{
//			int nUpdateInterval = (m_nOPCMainThreadOPCPollSleep/2>0)?m_nOPCMainThreadOPCPollSleep/2:1;
//			if(m_nMainUpdateIndex%nUpdateInterval == 0)
//			{
//				wstring wstrlog = L"Start Read Update Single OPC Data(UpdateSingleOPCData) Once";
//				_tprintf(wstrlog.c_str());
//				UpdateSingleOPCData();
//			}
//			m_nMainUpdateIndex++;
//			if(m_nMainUpdateIndex >= 100000)
//				m_nMainUpdateIndex = 0;
//		}
//	}
//
//	if(m_bConnectOK)				//���ӳɹ�ʱ���ȥ��ֵ
//	{
//		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReConnectTime;
//		if(oleSpan.GetTotalMinutes() >= m_nReconnectMinute)
//		{
//			for (unsigned int i = 0; i < m_pointlist.size(); i++)
//			{
//				DataPointEntry& entry = m_pointlist[i];
//				if(entry.GetUpdateSignal()<10)
//				{
//					if(entry.GetOpcPointType() == VT_BSTR)
//					{
//						wstring strValueSet = entry.GetSValue();
//						valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
//					}
//					else
//					{
//						int nPosition = _wtoi(entry.GetParam(7).c_str());
//						if(nPosition>0)			//ȡλ����
//						{
//							wstring strValueSet = GetBitFromString(entry.GetSValue(),nPosition);
//							valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
//						}
//						else		//���Լ��㱶��
//						{
//							wstring strValueSet = Project::Tools::RemoveDecimalW(entry.GetMultipleReadValue(entry.GetValue()),m_nPrecision);
//							valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
//						}
//						
//					}
//				}
//			}
//		}
//
//		if(m_pointlist.size() >0 )
//		{
//			wstring strUpdateTime;
//			Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);
//
//			CString strName;
//			strName.Format(_T("%s%d"),TIME_UPDATE_OPC,m_nOPCClientIndex);
//			valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
//
//			if(m_dbsession_history && m_dbsession_history->IsConnected())
//			{
//				m_dbsession_history->WriteCoreDebugItemValue(strName.GetString(),strUpdateTime);
//			}
//			valuelist.push_back(make_pair(LOG_OPC, Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str())));
//		}
//	}
//	//////////////////////////////////////////////////////////////////////////
//
//	//���ð�ȫ����ʱ�� ���е�OPC���������������������
//	if(m_bEnableSecurity)
//	{
//		m_nReloadErrItem++;
//		if(m_nReloadErrItem%30 == 0)
//		{
//			ReLoadItemHandle();
//			if(m_nReloadErrItem%60 == 0)
//				ReConnectOPC();
//		}
//
//		if(m_nReloadErrItem>=10000)
//			m_nReloadErrItem = 0;
//	}
//}

//20180926
void COPCEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	//20180926////////////////////////////////////////////////////////////////////////
	//�첽дֵ�� ÿ����ֵʱ�� ��������ȡ�����ڴ����첽�ص�д�ɹ��ĵ�λ
	ReReadAsync20ItemV1();

	//���첽��ͬ����д
	if(m_bAsync20Mode)				//�첽��ʽ��ȡ
	{
		COleDateTime oleCurrent = COleDateTime::GetCurrentTime();
		COleDateTimeSpan oleSpan = oleCurrent - m_oleRefresh20;
		if(oleSpan.GetTotalMinutes() >= m_nRefresh20Interval)
		{
			Refresh20();
			m_oleRefresh20 = oleCurrent;
		}

		//ÿ�������� �첽ˢ��һ��
		UpdateOPCDataBufferV1();
	}
	else
	{
		if(m_bUseThread && !m_bEnableSecurity)		//�����߳���δ���ý��̰�ȫ����  ���������߳�������ȡ
		{
			//���߳���ִ�� UpdateMutilOPCDataV1
		}
		else if(m_bUseThread && m_bEnableSecurity)		//�����߳������ý��̰�ȫ����  ֻ������������������ȡ
		{
			int nUpdateInterval = (m_nOPCMainThreadOPCPollSleep/2>0)?m_nOPCMainThreadOPCPollSleep/2:1;
			if(m_nMainUpdateIndex%nUpdateInterval == 0)
			{				
				UpdateMutilOPCDataV1();
			}

			m_nMainUpdateIndex++;
			if(m_nMainUpdateIndex >= 100000)
				m_nMainUpdateIndex = 0;
		}
		else
		{
			int nUpdateInterval = (m_nOPCMainThreadOPCPollSleep/2>0)?m_nOPCMainThreadOPCPollSleep/2:1;
			if(m_nMainUpdateIndex%nUpdateInterval == 0)
			{
				UpdateSingleOPCDataV1();
			}
			m_nMainUpdateIndex++;
			if(m_nMainUpdateIndex >= 100000)
				m_nMainUpdateIndex = 0;
		}
	}

	//��OPCitem�а�ֵ���µ�pointlist
	GetValueFromOPCItemV1();

	//��pointlist�з���ֵ
	GetValueListV1(valuelist);
	//////////////////////////////////////////////////////////////////////////

	//���ð�ȫ����ʱ�� ���е�OPC���������������������
	if(m_bEnableSecurity)
	{
		m_nReloadErrItem++;
		if(m_nReloadErrItem%30 == 0)
		{
			ReLoadItemHandle();
			if(m_nReloadErrItem%60 == 0)
				ReConnectOPC();
		}

		if(m_nReloadErrItem>=10000)
			m_nReloadErrItem = 0;
	}
}

void COPCEngine::ClearResource()
{
	m_bConnectOK = false;
	if (m_opcsession)
	{
		delete m_opcsession;
		m_opcsession = NULL;
	}
}

COPCConnection_AutoBrowse* COPCEngine::CreateOPCObject( OPCSERVER_TYPE type )
{
	if (TYPE_AUTOBROWSEITEM == type){
		COPCConnection_AutoBrowse* presult =  new COPCConnection_AutoBrowse();
	}
	else if (TYPE_MANUALBROWSEITEM == type){
		return new COPCConnection_ManualBrowse();
	}
	else{
		ASSERT(FALSE);
		return NULL;
	}

	return NULL;
}

bool COPCEngine::SetValue( const wstring& pointname, double value )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(m_bConnectOK)				//���ӳɹ�ʱ���ȥ��ֵ
	{
		for (unsigned int i = 0; i < m_pointlist.size(); i++)
		{
			const DataPointEntry& entry = m_pointlist[i];
			if (entry.GetShortName() == pointname)			//�ҵ��õ�
			{
				value = entry.GetMultipleReadValue(value,false);
				int nPosition = _wtoi(entry.GetParam(7).c_str());
				if(nPosition>0)				//ȡλ
				{
					double dwValue = 0;
					hash_map<wstring, double>::iterator iter = m_PositionValuemap.find(pointname);
					if(iter != m_PositionValuemap.end())
					{
						dwValue = (*iter).second;
					}
					value = SetBitToFloat(dwValue,nPosition,value);	
				}
				bool	bResult = false;
				VARTYPE pointtype = m_opcsession->GetItemTypeByShortName(pointname);
				switch(pointtype)
				{
				case  VT_BOOL:
					{
						bool bValue = (fabs(value)>=1e-10);
						bResult = m_opcsession->SetValue(pointname, bValue);
					}
					break;
				case VT_I2:
				case VT_I4:
					{
						int nValue = (int) value;
						bResult = m_opcsession->SetValue(pointname, nValue);
					}
					break;
				case VT_R4:
					bResult = m_opcsession->SetValue(pointname, value);
					break;
				default:
					bResult = m_opcsession->SetValue(pointname, value);
					break;
				}
				return bResult;
			}
		}
	}
	return false;
}

//20180926 ͳһд���
bool COPCEngine::SetValue( const wstring& pointname, wstring strvalue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(m_bConnectOK)				//���ӳɹ�ʱ���ȥ��ֵ
	{
		for (unsigned int i = 0; i < m_pointlist.size(); i++)
		{
			const DataPointEntry& entry = m_pointlist[i];
			if (entry.GetShortName() == pointname)			//�ҵ��õ�
			{
				double value =  entry.GetMultipleReadValue(_wtof(strvalue.c_str()), false);
				int nPosition = _wtoi(entry.GetParam(7).c_str());
				if(nPosition>0)				//ȡλ
				{
					double dwValue = 0;
					hash_map<wstring, double>::iterator iter = m_PositionValuemap.find(pointname);
					if(iter != m_PositionValuemap.end())
					{
						dwValue = (*iter).second;
					}
					value = SetBitToFloat(dwValue,nPosition,value);	
				}
				bool	bResult = false;
				VARTYPE pointtype = m_opcsession->GetItemTypeByShortName(pointname);
				switch(pointtype)
				{
				case  VT_BOOL:
					{
						bool bValue = (fabs(value)>=1e-10);
						bResult = m_opcsession->SetValue(pointname, bValue);
					}
					break;
				case VT_I2:
				case VT_I4:
					{
						int nValue = (int) value;
						bResult = m_opcsession->SetValue(pointname, nValue);
					}
					break;
				case VT_R4:
					bResult = m_opcsession->SetValue(pointname, value);
					break;
				default:
					bResult = m_opcsession->SetValue(pointname, value);
					break;
				}
				return bResult;
			}
		}
	}
	return false;
}

void COPCEngine::AddLog( const wchar_t* loginfo )
{
	if (m_logsession)
	{
		m_logsession->InsertLog(loginfo);
	}
}

void COPCEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool COPCEngine::GetValue( const wstring& pointname, wstring& strvalue )
{
	if(m_bConnectOK)				//���ӳɹ�ʱ���ȥ��ֵ
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReConnectTime;
		if(oleSpan.GetTotalMinutes() >= m_nReconnectMinute)
		{
			for (unsigned int i = 0; i < m_pointlist.size(); i++)
			{
				DataPointEntry& entry = m_pointlist[i];
				if (entry.GetShortName() == pointname)
				{
					if(entry.GetUpdateSignal()<10)
					{
						if(entry.GetOpcPointType() == VT_BSTR)
						{
							strvalue = entry.GetSValue();
							return true;
						}
						else
						{
							int nPosition = _wtoi(entry.GetParam(7).c_str());
							if(nPosition>0)			//ȡλ����
							{
								strvalue = GetBitFromString(entry.GetSValue(),nPosition);
								return true;
							}
							else		//���Լ��㱶��
							{
								strvalue = Project::Tools::RemoveDecimalW(entry.GetMultipleReadValue(entry.GetValue()),m_nPrecision);
								return true;
							}
						}
					}
					return false;
				}
			}
		}
	}
	return false;
}

bool COPCEngine::GetValue( const wstring& pointname, bool& strvalue )
{
	if(m_bConnectOK)				//���ӳɹ�ʱ���ȥ��ֵ
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReConnectTime;
		if(oleSpan.GetTotalMinutes() >= m_nReconnectMinute)
		{
			for (unsigned int i = 0; i < m_pointlist.size(); i++)
			{
				DataPointEntry& entry = m_pointlist[i];
				if (entry.GetShortName() == pointname)
				{
					if(entry.GetUpdateSignal()<10)
					{
						strvalue = (int)entry.GetMultipleReadValue(entry.GetValue());
						return true;
					}
					return false;
				}
			}
		}
	}
	return false;
}

bool COPCEngine::InitDataEntryValue(const wstring& pointname, double value)
{
//	if (m_opcsession){
//		return m_opcsession->InitDataEntryValue(pointname, strvalue);
//	}

	return false;

}

bool COPCEngine::GetValue( const wstring& pointname, int& strvalue )
{
	if(m_bConnectOK)				//���ӳɹ�ʱ���ȥ��ֵ
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReConnectTime;
		if(oleSpan.GetTotalMinutes() >= m_nReconnectMinute)
		{
			for (unsigned int i = 0; i < m_pointlist.size(); i++)
			{
				DataPointEntry& entry = m_pointlist[i];
				if (entry.GetShortName() == pointname)
				{
					if(entry.GetUpdateSignal()<10)
					{
						if(entry.GetOpcPointType() == VT_BSTR)
						{
							return false;
						}
						else
						{
							int nPosition = _wtoi(entry.GetParam(7).c_str());
							strvalue = (int)entry.GetMultipleReadValue(entry.GetValue());
							if(nPosition>0)			//ȡλ����
							{
								strvalue = GetBitFromInt(strvalue,nPosition);
								return true;
							}
							else		//���Լ��㱶��
							{								
								return true;
							}
						}
					}
					return false;
				}
			}
		}
	}
	return false;
}

bool COPCEngine::GetValue( const wstring& pointname, double& strvalue )
{
	if(m_bConnectOK)				//���ӳɹ�ʱ���ȥ��ֵ
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReConnectTime;
		if(oleSpan.GetTotalMinutes() >= m_nReconnectMinute)
		{
			for (unsigned int i = 0; i < m_pointlist.size(); i++)
			{
				DataPointEntry& entry = m_pointlist[i];
				if (entry.GetShortName() == pointname)
				{
					if(entry.GetUpdateSignal()<10)
					{
						if(entry.GetOpcPointType() == VT_BSTR)
						{
							return false;
						}
						else
						{
							int nPosition = _wtoi(entry.GetParam(7).c_str());
							strvalue = entry.GetMultipleReadValue(entry.GetValue());
							if(nPosition>0)			//ȡλ����
							{
								strvalue = GetBitFromFloat(strvalue,nPosition);
								return true;
							}
							else		//���Լ��㱶��
							{
								return true;
							}
						}
					}
					return false;
				}
			}
		}
	}
	return false;
}

bool COPCEngine::GetValueFromDevice( const wstring& pointname, wstring& strvalue )
{
	if (m_opcsession){
		return m_opcsession->ReadValue(pointname, strvalue);
	}

	return false;
}

bool COPCEngine::GetValueFromDevice( const wstring& pointname, bool& strvalue )
{
	if (m_opcsession){
		return m_opcsession->ReadValue(pointname, strvalue);
	}

	return false;
}

bool COPCEngine::GetValueFromDevice( const wstring& pointname, int& strvalue )
{
	if (m_opcsession){
		return m_opcsession->ReadValue(pointname, strvalue);
	}

	return false;
}

bool COPCEngine::GetValueFromDevice( const wstring& pointname, double& strvalue )
{
	if (m_opcsession){
		return m_opcsession->ReadValue(pointname, strvalue);
	}

	return false;
}

bool COPCEngine::Exit()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	m_bExitThread = true;
	if(m_hDataReadThread)
	{
		CloseHandle(m_hDataReadThread);
		m_hDataReadThread = NULL;
	}
	if(m_hDataReadDeviceThread)
	{
		CloseHandle(m_hDataReadDeviceThread);
		m_hDataReadDeviceThread = NULL;
	}
	if(m_hOPCConnectThread)
	{
		CloseHandle(m_hOPCConnectThread);
		m_hOPCConnectThread = NULL;
	}
	m_PositionValuemap.clear();
	m_opcsession->Exit();
	return true;
}

void COPCEngine::SetMultiMode( bool bMultiMode )
{
	m_bMultiClientMode = bMultiMode;
}

UINT WINAPI COPCEngine::ThreadUpdateOPCData( LPVOID lparam )
{
	COPCEngine* pthis = (COPCEngine*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(!pthis->m_bEnableSecurity)		//��;�ӷǰ�ȫģʽ�л���ȫ�߳�ģʽbug
			{
				if(pthis->m_bAsync20Mode == false)
					pthis->UpdateMutilOPCDataV1();	

				int nSleep = pthis->m_nOPCPollSleep;
				while(!pthis->m_bExitThread)
				{
					nSleep--;
					if(nSleep <= 0)
						break;
					Sleep(1000);
				}
			}
			else
			{
				Sleep(1000);
			}
		}
	}

	return 0;
}

bool COPCEngine::UpdateMutilOPCData()
{
	SumReadAndResponse();
	if(m_bConnectOK)
	{
		unsigned int nStart = 0;
		unsigned int nEnd = 0;
		for(nEnd; nEnd < m_pointlist.size(); ++nEnd)
		{		
			if(nEnd-nStart+1>=m_nMutilCount)	//��ѯ
			{
				GetMutilOPCData(nStart,nEnd,m_pointlist);
				nStart = nEnd;
				Sleep(m_nSleep);
			}
		}

		if(nStart != nEnd)
		{
			GetMutilOPCData(nStart,m_pointlist.size(),m_pointlist);
			Sleep(m_nSleep);
		}

		//���һ�δ��
		if(m_opcsession)
		{
			m_opcsession->ActiveReloadSynErrorItems();
		}
		return true;
	}
	else
	{
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_OPC_GET_VALUE_FAIL);
	}
	return false;
}

bool COPCEngine::GetMutilOPCData( int nStart, int nEnd, vector<DataPointEntry>& pointlist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(nStart>nEnd || nEnd>pointlist.size() || pointlist.size()<=0)
		return false;
	if(m_bExitThread || !m_bConnectOK)
		return false;
	map<wstring,OPC_VALUE_INFO> mapItem;
	for (unsigned int i = nStart; i < nEnd; i++)
	{
		DataPointEntry& entry = pointlist[i];
		if(entry.GetOPCDataSourceType() == 3)
			continue;
		wstring pointname = entry.GetShortName();
		entry.SetToUpdate();
		m_nReadCount++;
		wstring strValue = L"";

		OPC_VALUE_INFO info;
		info.strOPCName = pointname;
		info.strOPCValue = strValue;
		info.wstrMultiple = entry.GetParam(4);
		info.nIndex = i;
		info.nPosition = _wtoi(entry.GetParam(7).c_str());
		info.vType = entry.GetOpcPointType();
		mapItem[pointname] = info;
	}
	if(mapItem.size() <= 0)
		return false;

	//��ѯ
	bool bresult = m_opcsession->GetMutilItemValue(mapItem);
	if(!bresult)
	{
		map<wstring,OPC_VALUE_INFO>::iterator iter = mapItem.begin();
		if(iter != mapItem.end())
		{
			CString strLog;
			strLog.Format(_T("ERROR: GetMutilOPCData 1#(%d):Read MutilValue num %d(%s) Failed.\r\n"),m_nOPCClientIndex,mapItem.size(),iter->first.c_str());
			PrintLog(strLog.GetString(),false);
		}
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_GET_MUTIL_OPC_VALUES);
		return false;
	}
	else
	{
		m_oleUpdateTime = COleDateTime::GetCurrentTime(); 
		map<wstring,OPC_VALUE_INFO>::iterator iter = mapItem.begin();
		while(iter != mapItem.end())
		{
			wstring strOPCName = iter->first;
			int nIndex = iter->second.nIndex;
			if(nIndex<0 || nIndex>=pointlist.size())
			{
				++iter;
			}
			DataPointEntry& entry = pointlist[nIndex];
			if(iter->second.bUpdate && (iter->second.vType == VT_BSTR || iter->second.strOPCValue.length()>0))
			{
				entry.SetUpdated();
				double fValueRead = entry.GetMultipleReadValue(_wtof(iter->second.strOPCValue.c_str()));
				if(iter->second.nPosition>0)  //ȡλ
				{
					m_PositionValuemap[entry.GetShortName()] = fValueRead;
					fValueRead = GetBitFromFloat(fValueRead,iter->second.nPosition);
				}
				entry.SetValue(fValueRead);
				if(entry.GetOpcPointType() == VT_BSTR)
				{
					entry.SetSValue(iter->second.strOPCValue);
				}
				m_nReSponse++;
			}
			else
			{
				CString strLog;
				strLog.Format(_T("ERROR: GetMutilOPCData 2#(%d):Read MutilValue(%s) Failed.\r\n"),m_nOPCClientIndex,entry.GetShortName().c_str());
				PrintLog(strLog.GetString(),false);
				CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_GET_MUTIL_OPC_VELUE);
			}
			++iter;
		}
	}
	return true;
}

void COPCEngine::SetDebug( int nDebug ,int nOPCCheckReconnectInterval,int nOPCReconnectInertval,int nReconnectMinute,bool bEnableSecurity,int nOPCMainThreadOPCPollSleep)
{
	m_nOPCReconnectInertval = nOPCReconnectInertval;
	m_nOPCCheckReconnectInterval = nOPCCheckReconnectInterval;
	m_nReconnectMinute = nReconnectMinute;
	m_bEnableSecurity = bEnableSecurity;
	m_nOPCMainThreadOPCPollSleep = nOPCMainThreadOPCPollSleep;
	if(m_opcsession)
		return m_opcsession->SetDebug(nDebug);
}

UINT WINAPI COPCEngine::ThreadConnectOPCData( LPVOID lparam )
{
	COPCEngine* pthis = (COPCEngine*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(!pthis->m_bEnableSecurity)
				pthis->ReConnectOPC();	
			int nSleep = pthis->m_nOPCCheckReconnectInterval*60;
			while(!pthis->m_bExitThread)
			{
				nSleep--;
				if(nSleep%60 == 0 && !pthis->m_bEnableSecurity)		//δ���ð�ȫ����
					pthis->ReLoadItemHandle();
				if(nSleep <= 0)
					break;
				Sleep(1000);
			}
		}
	}

	return 0;
}

bool COPCEngine::ReConnectOPC()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleUpdateTime;
	if(oleSpan.GetTotalMinutes() >= m_nOPCReconnectInertval)
	{
		m_opcsession->SetPointList(m_pointlist);
		m_bConnectOK = m_opcsession->ReConnectOPC();
		if(m_bConnectOK)
		{
			m_nSingleReadCount = 0;
			m_oleReConnectTime = COleDateTime::GetCurrentTime();
			m_oleUpdateTime = m_oleReConnectTime;
		}
		else
		{
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_RECONNECT_OPC,true);
		}
	}
	return false;
}

bool COPCEngine::UpdateSingleOPCData()
{
	SumReadAndResponse();

	m_nSingleReadCount++;
	if(m_nSingleReadCount >= 10000)
		m_nSingleReadCount = 0;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
		DataPointEntry& entry = m_pointlist[i];
		wstring pointname = entry.GetShortName();
		entry.SetToUpdate();
		m_nReadCount++;

		if(m_bConnectOK)	//���ӳɹ�
		{
			if(m_opcsession)
			{
				CKItem* pItem = m_opcsession->FindItem(entry.GetPlcAddress());
				if(pItem)
				{
					if(pItem->IsValid())
					{
						wstring strValue = L"";
						bool bresult = false;
						VARTYPE type = m_opcsession->GetItemTypeByShortName(pointname);
						CString strType = _T("Undefine");
						bresult = m_opcsession->GetValue(pointname, strValue);
						StringFromVartype(type,strType);

						if (bresult && (type == VT_BSTR || strValue.length()>0))
						{
							entry.SetUpdated();
							m_nReSponse++;
							m_oleUpdateTime = COleDateTime::GetCurrentTime(); 
							double fValueRead = entry.GetMultipleReadValue(_wtof(strValue.c_str()));						

							int nPosition = _wtoi(entry.GetParam(7).c_str());
							if(nPosition>0)  //ȡλ
							{
								m_PositionValuemap[pointname] = fValueRead;
								fValueRead = GetBitFromFloat(fValueRead,nPosition);
							}
							entry.SetValue(fValueRead);
							if(entry.GetOpcPointType() == VT_BSTR)
							{
								entry.SetSValue(strValue);
							}
							Sleep(m_nSleep);
						}
						else if(bresult && type != VT_BSTR  && strValue.length() == 0)
						{
							CString strlog;
							strlog.Format(_T("ERROR: Get Point %s(%s) Value Success But Value Is Null.Please Check!\r\n"),pointname.c_str(),strType);
							PrintLog(strlog.GetString(),false);
						}
						else
						{		
							CString strLog_;
							strLog_.Format(_T("ERROR: Get Point %s(%s) Value Failed(%d).Please Check!\r\n"),pointname.c_str(),strType,m_nSingleReadCount);
							PrintLog(strLog_.GetString(),false);
							CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_GET_OPC_CACHE);
						}
					}
					else		//not valid
					{
						CString strLog_;
						strLog_.Format(_T("ERROR: Get Value %s Fail(Not Valid)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
						PrintLog(strLog_.GetString(),false);
					}
				}
				else		//�Ҳ��� item
				{
					CString strLog_;
					strLog_.Format(_T("ERROR: Get Value %s Fail(Can't Find)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
					PrintLog(strLog_.GetString(),false);
				}
			}
		}
		else				//����ʧ��
		{
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_OPC_GET_VALUE_FAIL,true);		
		}
	}
	return true;
}

void COPCEngine::OutErrLog( wstring strLog_ )
{
	try
	{
		wstring strPath;
		GetSysPath(strPath);
		strPath += L"\\log";
		if(Project::Tools::FindOrCreateFolder(strPath))
		{
			COleDateTime oleNow = COleDateTime::GetCurrentTime();
			CString strLogPath;
			strLogPath.Format(_T("%s\\core_err_%d_%02d_%02d.log"),strPath.c_str(),oleNow.GetYear(),oleNow.GetMonth(),oleNow.GetDay());

			CString strLog;
			CString strTime;
			strTime.Format(_T("%02d:%02d:%02d"),oleNow.GetHour(),oleNow.GetMinute(),oleNow.GetSecond());
			strLog.Format(_T("%s::%s"),strTime,strLog_.c_str());

			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			setlocale( LC_ALL, "chs" );
			//��¼Log
			CStdioFile	ff;
			if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			{
				ff.Seek(0,CFile::end);
				ff.WriteString(strLog);
				ff.Close();
			}
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 	
		}
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}

bool COPCEngine::ReLoadItemHandle()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(m_opcsession)
	{
		return m_opcsession->ReloadErrorItemsHandle();
	}
	return false;
}

double COPCEngine::GetBitFromFloat(float dValue, int nIndex)
{
	WORD dwValue = (WORD)dValue;
	WORD dwTemp = dwValue<<(16-nIndex);
	int nTemp = dwTemp>> 15;
	return (double)nTemp;
}

int COPCEngine::GetBitFromInt(int nValue, int nIndex)
{
	WORD dwValue = (WORD)nValue;
	WORD dwTemp = dwValue<<(16-nIndex);
	int nTemp = dwTemp>> 15;
	return nTemp;
}

wstring COPCEngine::GetBitFromString(wstring strValue, int nIndex)
{
	WORD dwValue = (WORD)_wtoi(strValue.c_str());
	WORD dwTemp = dwValue<<(16-nIndex);
	int nTemp = dwTemp>> 15;
	CString strValue_;
	strValue_.Format(_T("%d"),nTemp);
	return strValue_.GetString();
}

double COPCEngine::SetBitToFloat(double dwData, int nIndex,double dwValue)
{
	nIndex = nIndex-1;
	WORD nData = (WORD)dwData;
	int nValue = (int)dwValue;
	nData &= ~(1<<nIndex); //��*num�ĵ�posλ����Ϊ0
	nData |= nValue<<nIndex;
	return (double)nData;
}

wstring COPCEngine::SetBitToString(double dwData, int nIndex,wstring strValue)
{
	nIndex = nIndex-1;
	WORD nData = (WORD)dwData;
	int nValue = _wtoi(strValue.c_str());
	nData &= ~(1<<nIndex); //��*num�ĵ�posλ����Ϊ0
	nData |= nValue<<nIndex;
	CString strValue_;
	strValue_.Format(_T("%d"),nData);
	return strValue_.GetString();
}

void COPCEngine::SumReadAndResponse()
{
	if(m_nReadCount > 0)
	{
		std::ostringstream sqlstream;
		sqlstream << "INFO : OPC Engine(" << m_nOPCClientIndex << ") Read Cache(" << m_nReadCount << ") Response Cache(" << m_nReSponse << ").\r\n";
		m_strUpdateLog = sqlstream.str();
		PrintLog(Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str()),false);
	}
	
	m_nReSponse = 0;
	m_nReadCount = 0;
	m_nReadDeviceCount = 0;
	m_nReSponseDevice = 0;
}

bool COPCEngine::UpdateOPCDataBuffer()
{
	SumReadAndResponse();
	m_nSingleReadCount++;
	if(m_nSingleReadCount >= 10000)
		m_nSingleReadCount = 0;

	bool bUpdateSuccess = false;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
		DataPointEntry& entry = m_pointlist[i];
		wstring pointname = entry.GetShortName();
		entry.SetToUpdate();
		
		if(m_bConnectOK)
		{
			COleDateTime oleUpdate;
			bool bUpdateTimeAsync20 = false;
			if(m_opcsession->GetUpdateTime20(oleUpdate,bUpdateTimeAsync20,m_nDebugOPC))
			{
				m_oleUpdateTime = oleUpdate;
			}
			if(bUpdateTimeAsync20)
			{
				if(m_opcsession)
				{
					CKItem* pItem = m_opcsession->FindItem(entry.GetPlcAddress());
					if(pItem)
					{
						if(pItem->IsValid())			//��Ч��λ��ȥ��ȡֵ
						{	
							if(pItem->IsUpdated() && pItem->IsActive())
							{
								VARTYPE type = pItem->GetDataType();
								CString strType = _T("Undefine");
								StringFromVartype(type,strType);
								CString strTemp = _T("");
								pItem->GetValue(strTemp);
								wstring strValue = strTemp.GetString();
								if (type == VT_BSTR || strValue.length()>0)
								{
									double fValueRead = entry.GetMultipleReadValue(_wtof(strValue.c_str()));
									int nPosition = _wtoi(entry.GetParam(7).c_str());
									if(nPosition>0)  //ȡλ
									{
										m_PositionValuemap[pointname] = fValueRead;
										fValueRead = GetBitFromFloat(fValueRead,nPosition);
									}
									entry.SetUpdated();
									entry.SetValue(fValueRead);
									if(entry.GetOpcPointType() == VT_BSTR)
									{
										entry.SetSValue(strValue);
									}
									bUpdateSuccess = true;
								}
								else if(type != VT_BSTR  && strValue.length() == 0)
								{
									CString strLog;
									strLog.Format(_T("ERROR: Get Point %s Value Success But Value Is Null.Please Check!\r\n"),pointname.c_str());
									PrintLog(strLog.GetString(),false);
								}
							}	
						}
						else
						{
							CString strLog_;
							strLog_.Format(_T("ERROR: Get Value %s Fail(Not Valid)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
							PrintLog(strLog_.GetString(),false);
						}
					}
					else
					{
						CString strLog_;
						strLog_.Format(_T("ERROR: Get Value %s Fail(Can't Find)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
						PrintLog(strLog_.GetString(),false);
					}
				}
			}
		}
		else
		{

		}
	}
	return bUpdateSuccess;
}

bool COPCEngine::Refresh20( bool bDeviceRead /*= false*/ )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(m_bConnectOK && m_opcsession)
	{
		return m_opcsession->RefreshAsync20(false);
	}
	return true;
}

void COPCEngine::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
	if(bSaveLog)
	{
		OutErrLog(strLog);
	}
}

bool COPCEngine::ReReadAsync20Item()
{
	if(m_opcsession)
	{
		CObArray cAsync20WriteItemList;
		if(m_opcsession->ReReadAsync20Item(cAsync20WriteItemList))		//�ɹ�����µ�pointlist��ȥ
		{
			DWORD cdwItems = cAsync20WriteItemList.GetSize();
			hash_map<wstring,CKItem *> mapAsync20WriteValue;
			for (DWORD dwIndex = 0; dwIndex < cdwItems; dwIndex++)
			{
				CKItem *pItem = (CKItem *)cAsync20WriteItemList [dwIndex];
				if(pItem)		//���¶�Ӧ���ֵ
				{			
					wstring wstrItemID = pItem->GetItemID();
					mapAsync20WriteValue[wstrItemID] = pItem;
				}
			}

			if(mapAsync20WriteValue.size() > 0)
			{
				for (unsigned int iterator = 0; iterator < m_pointlist.size(); iterator++)
				{
					DataPointEntry& entry = m_pointlist[iterator];
					wstring wstrItemID = entry.GetPlcAddress();
					wstring pointname = entry.GetShortName();
					hash_map<wstring,CKItem *>::iterator iterFind = mapAsync20WriteValue.find(wstrItemID);
					if(iterFind != mapAsync20WriteValue.end())
					{
						CKItem * pItem = (*iterFind).second;
						if(pItem)
						{
							VARTYPE type = pItem->GetDataType();
							CString strType = _T("Undefine");
							StringFromVartype(type,strType);
							CString strTemp = _T("");
							pItem->GetValue(strTemp);
							wstring strValue = strTemp.GetString();
							if (type == VT_BSTR || strValue.length()>0)
							{
								double fValueRead = entry.GetMultipleReadValue(_wtof(strValue.c_str()));
								int nPosition = _wtoi(entry.GetParam(7).c_str());
								if(nPosition>0)  //ȡλ
								{
									m_PositionValuemap[pointname] = fValueRead;
									fValueRead = GetBitFromFloat(fValueRead,nPosition);
								}
								entry.SetUpdated();
								entry.SetValue(fValueRead);
								if(entry.GetOpcPointType() == VT_BSTR)
								{
									entry.SetSValue(strValue);
								}
							}							
						}
					}
				}
			}
		}	
	}
	return true;
}

bool COPCEngine::GetValueFromOPCItemV1()
{
	if(m_opcsession && m_bConnectOK)	//���ӳɹ�ʱ���ȥ��ֵ
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReConnectTime;
		if(oleSpan.GetTotalMinutes() >= m_nReconnectMinute)
		{
			for (unsigned int i = 0; i < m_pointlist.size(); i++)
			{
				DataPointEntry& entry = m_pointlist[i];
				wstring wstrItemID = entry.GetPlcAddress();
				wstring pointname = entry.GetShortName();
				CKItem* pItem = m_opcsession->FindItem(wstrItemID);

				if(m_bAsync20Mode)		//�첽ģʽʱ��
				{
					if(pItem->IsUpdated() && pItem->IsActive())
					{
						entry.SetUpdated();
					}
				}

				if(pItem && pItem->IsValid())
				{
					VARTYPE type = pItem->GetDataType();
					CString strType = _T("Undefine");
					StringFromVartype(type,strType);
					CString strTemp = _T("");
					pItem->GetValue(strTemp);
					wstring strValue = strTemp.GetString();
					if (type == VT_BSTR || strValue.length()>0)
					{
						double fValueRead = entry.GetMultipleReadValue(_wtof(strValue.c_str()));

						int nPosition = _wtoi(entry.GetParam(7).c_str());
						if(nPosition>0)  //ȡλ
						{
							m_PositionValuemap[pointname] = fValueRead;
							fValueRead = GetBitFromFloat(fValueRead,nPosition);
						}
						//entry.SetUpdated();
						entry.SetValue(fValueRead);
						if(entry.GetOpcPointType() == VT_BSTR)
						{
							entry.SetSValue(strValue);
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool COPCEngine::UpdateMutilOPCDataV1()
{
	SumReadAndResponse();
	if(m_bConnectOK)
	{
		unsigned int nStart = 0;
		unsigned int nEnd = 0;
		vector<wstring> strErrorPointList;
		for(nEnd; nEnd < m_pointlist.size(); ++nEnd)
		{		
			if(nEnd-nStart+1>m_nMutilCount)	//��ѯ
			{
				GetMutilOPCDataV1(nStart,nEnd,m_pointlist, strErrorPointList);
				nStart = nEnd;
				Sleep(m_nSleep);
			}
		}

		if(nStart != nEnd)
		{
			GetMutilOPCDataV1(nStart,m_pointlist.size(),m_pointlist, strErrorPointList);
			Sleep(m_nSleep);
		}

		//log bad points
		if(strErrorPointList.size()>0)
		{
			CString strTemp;
			strTemp.Format(_T("BAD POINTS START =======================\r\n"));

			CString strAll = strTemp;
			for(int i=0;i<strErrorPointList.size();i++)
			{

				strTemp.Format(_T("%s,"), strErrorPointList[i].c_str());
				strAll +=strTemp;
			}
			strAll+=_T("\r\n");


			strAll +=_T("BAD POINTS END =======================\r\n");

			PrintLog(strAll.GetString(), true);
		}

		//���һ�δ��
		if(m_opcsession)
		{
			m_opcsession->ActiveReloadSynErrorItems();
		}
		return true;
	}
	else
	{
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_OPC_GET_VALUE_FAIL);
	}
	return false;
}

bool COPCEngine::UpdateSingleOPCDataV1()
{
	PrintLog(L"UpdateSingleOPCDataV1: Start\r\n",false);
	SumReadAndResponse();

	m_nSingleReadCount++;
	if(m_nSingleReadCount >= 10000)
		m_nSingleReadCount = 0;

	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
		DataPointEntry& entry = m_pointlist[i];
		wstring pointname = entry.GetShortName();
		entry.SetToUpdate();
		m_nReadCount++;

		if(m_bConnectOK)	//���ӳɹ�
		{
			if(m_opcsession)
			{
				CKItem* pItem = m_opcsession->FindItem(entry.GetPlcAddress());
				if(pItem)
				{
					if(pItem->IsValid())
					{
						wstring strValue = L"";
						bool bresult = false;
						VARTYPE type = m_opcsession->GetItemTypeByShortName(pointname);
						CString strType = _T("Undefine");
						bresult = m_opcsession->GetValue(pointname, strValue);
						StringFromVartype(type,strType);

						if (bresult && (type == VT_BSTR || strValue.length()>0))
						{						
							m_nReSponse++;
							entry.SetUpdated();
							m_oleUpdateTime = COleDateTime::GetCurrentTime(); 				
							Sleep(m_nSleep);
						}
						else if(bresult && type != VT_BSTR  && strValue.length() == 0)
						{
							CString strlog;
							strlog.Format(_T("ERROR: Get Point %s(%s) Value Success But Value Is Null.Please Check!\r\n"),pointname.c_str(),strType);
							PrintLog(strlog.GetString(),true);
						}
						else
						{		
							CString strLog_;
							strLog_.Format(_T("ERROR: Get Point %s(%s) Value Failed(%d).Please Check!\r\n"),pointname.c_str(),strType,m_nSingleReadCount);
							PrintLog(strLog_.GetString(),true);
							CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_GET_OPC_CACHE);
						}
					}
					else		//not valid
					{
						CString strLog_;
						strLog_.Format(_T("ERROR: Get Value %s Fail(Not Valid)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
						PrintLog(strLog_.GetString(),true);
					}
				}
				else		//�Ҳ��� item
				{
					CString strLog_;
					strLog_.Format(_T("ERROR: Get Value %s Fail(Can't Find)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
					PrintLog(strLog_.GetString(),true);
				}
			}
		}
		else				//����ʧ��
		{
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_OPC_GET_VALUE_FAIL,true);		
		}
	}

	PrintLog(L"UpdateSingleOPCDataV1: Finish\r\n",false);
	return true;
}

bool COPCEngine::UpdateOPCDataBufferV1()
{
	SumReadAndResponse();

	m_nSingleReadCount++;
	if(m_nSingleReadCount >= 10000)
		m_nSingleReadCount = 0;

	bool bUpdateSuccess = false;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
		DataPointEntry& entry = m_pointlist[i];
		wstring pointname = entry.GetShortName();
		//entry.SetToUpdate();

		if(m_bConnectOK)
		{
			COleDateTime oleUpdate;
			bool bUpdateTimeAsync20 = false;
			if(m_opcsession->GetUpdateTime20(oleUpdate,bUpdateTimeAsync20,m_nDebugOPC))
			{
				m_oleUpdateTime = oleUpdate;
			}
			if(bUpdateTimeAsync20)
			{
				if(m_opcsession)
				{
					bUpdateSuccess = true;				
				}
			}
		}
	}
	return bUpdateSuccess;
}

bool COPCEngine::GetMutilOPCDataV1( int nStart, int nEnd, vector<DataPointEntry>& pointlist , vector<wstring> & strBadPointList)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(nStart>nEnd || nEnd>pointlist.size() || pointlist.size()<=0)
		return false;

	if(m_bExitThread || !m_bConnectOK)
		return false;

	map<wstring,OPC_VALUE_INFO> mapItem;
	for (unsigned int i = nStart; i < nEnd; i++)
	{
		DataPointEntry& entry = pointlist[i];
		if(entry.GetOPCDataSourceType() == 3)
			continue;

		wstring pointname = entry.GetShortName();
		entry.SetToUpdate();
		m_nReadCount++;
		wstring strValue = L"";

		OPC_VALUE_INFO info;
		info.strOPCName = pointname;
		info.strOPCValue = strValue;
		info.wstrMultiple = entry.GetParam(4);
		info.nIndex = i;
		info.nPosition = _wtoi(entry.GetParam(7).c_str());
		info.vType = entry.GetOpcPointType();
		mapItem[pointname] = info;
	}

	if(mapItem.size() <= 0)
		return false;

	//��ѯ
	bool bresult = m_opcsession->GetMutilItemValue(mapItem);
	if(!bresult)
	{
		map<wstring,OPC_VALUE_INFO>::iterator iter = mapItem.begin();
		if(iter != mapItem.end())
		{
			CString strLog;
			strLog.Format(_T("ERROR: GetMutilOPCDataV1 1#(%d):Read MutilValue num %d(%s) Failed.\r\n"),m_nOPCClientIndex,mapItem.size(),iter->first.c_str());
			PrintLog(strLog.GetString(),false);
		}
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_GET_MUTIL_OPC_VALUES);
		return false;
	}
	else
	{
		m_oleUpdateTime = COleDateTime::GetCurrentTime(); 
		map<wstring,OPC_VALUE_INFO>::iterator iter = mapItem.begin();

		while(iter != mapItem.end())
		{
			wstring strOPCName = iter->first;
			int nIndex = iter->second.nIndex;
			if(nIndex<0 || nIndex>=pointlist.size())
			{
				++iter;
			}
			DataPointEntry& entry = pointlist[nIndex];
			if(iter->second.bUpdate && (iter->second.vType == VT_BSTR || iter->second.strOPCValue.length()>0))
			{
				entry.SetUpdated();
				m_nReSponse++;
			}
			else
			{
				CString strLog;
				strLog.Format(_T("ERROR: GetMutilOPCDataV1 2#(%d):Read MutilValue(%s) Failed.\r\n"),m_nOPCClientIndex,entry.GetShortName().c_str());
				PrintLog(strLog.GetString(),false);
				CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_GET_MUTIL_OPC_VELUE);

				strBadPointList.push_back(entry.GetShortName());
			}
			++iter;
		}

		
	}
	return true;
}

bool COPCEngine::ReReadAsync20ItemV1()
{
	if(m_opcsession)
	{
		CObArray cAsync20WriteItemList;
		if(m_opcsession->ReReadAsync20Item(cAsync20WriteItemList))		//�ɹ�����µ�pointlist��ȥ
		{
	
		}	
	}
	return true;
}

void COPCEngine::GetValueListV1( vector< pair<wstring, wstring> >& valuelist )
{
	if(m_bConnectOK)				//���ӳɹ�ʱ���ȥ��ֵ
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReConnectTime;
		if(oleSpan.GetTotalMinutes() >= m_nReconnectMinute)
		{
			for (unsigned int i = 0; i < m_pointlist.size(); i++)
			{
				DataPointEntry& entry = m_pointlist[i];
				if(entry.GetUpdateSignal()<10)
				{
					if(entry.GetOpcPointType() == VT_BSTR)
					{
						wstring strValueSet = entry.GetSValue();
						valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
					}
					else
					{
						int nPosition = _wtoi(entry.GetParam(7).c_str());
						if(nPosition>0)			//ȡλ����
						{
							wstring strValueSet = GetBitFromString(entry.GetSValue(),nPosition);
							valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
						}
						else		//���Լ��㱶��
						{
							wstring strValueSet = Project::Tools::RemoveDecimalW(entry.GetMultipleReadValue(entry.GetValue()),m_nPrecision);
							valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
						}

					}
				}
			}
		}

		if(m_pointlist.size() >0 )
		{
			wstring strUpdateTime;
			Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

			CString strName;
			strName.Format(_T("%s%d"),TIME_UPDATE_OPC,m_nOPCClientIndex);
			valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));

			if(m_dbsession_history && m_dbsession_history->IsConnected())
			{
				m_dbsession_history->WriteCoreDebugItemValue(strName.GetString(),strUpdateTime);
			}
			//valuelist.push_back(make_pair(LOG_OPC, Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str())));
		}
	}
}



vector<DataPointEntry> & COPCEngine::GetPointList()
{
	return m_pointlist;
}
