#pragma once

#include "StdAfx.h"
#include "ModbusRTUCtrl.h"
#include "../COMdll//DebugLog.h"
#include "Tools/EngineInfoDefine.h"

CModbusRTUCtrl::CModbusRTUCtrl(void)
	:m_hsendthread(NULL)
	,m_nSendReadCmdTimeInterval(30)
	,m_nRecevieTimeOut(5000)
	,m_nRecvedSlave(-1)
	,m_bExitThread(false)
	,m_bReadOneByOneMode(false)
	,m_nMutilReadCount(99)
	,m_nIDInterval(500)
	,m_nPortnr(0)
	,m_nBaud(9600)
	,m_cParity('N')
	,m_bInitPortSuccess(false)
	,m_nModbusNetworkErrorCount(0)
	,m_StartAddress(0)
	,m_WriteStartAddress(0)
	,m_nOutPut(0)
	,m_nReadNum(0)
	,m_nPollSendInterval(2)
	,m_nPrecision(6)
	,m_bDisSingleRead(false)
	,m_logsession(NULL)
	,m_bSaveErrLog(false)
	,m_nCmdCount(0)
	,m_nResponseCount(0)
	,m_nUpdatePointCount(0)
	,m_strUpdateLog("")
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_oleLastUpdateLog = COleDateTime::GetCurrentTime();
	m_pSendData = NULL;
	m_hDataCheckRetryConnectionThread = NULL;
	m_strErrInfo = "";
	m_nEngineIndex = 0;
	m_pointlist.clear();
	m_mapErrLog.clear();
	m_mapPointQuery.clear();
	m_sendCmdEvent = ::CreateEvent(NULL, false, false, L"");
	InitializeCriticalSection(&m_CriticalSection);
	InitializeCriticalSection(&m_CriticalPointSection);
}

CModbusRTUCtrl::~CModbusRTUCtrl(void)
{
	if (m_hsendthread != NULL)
	{
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
	if(m_hDataCheckRetryConnectionThread != NULL)
	{
		::CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}
	ClosePort(2);
	DeleteCriticalSection(&m_CriticalSection);
	DeleteCriticalSection(&m_CriticalPointSection);
}

//static long rxdatacount=0;  //�ñ������ڽ����ַ�����
//bool CModbusRTUCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
//{
//	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
//	OutPutModbusReceive(pRevData,length,m_StartAddress);
//
//	WORD wHeadAddr=0, wValue=0;
//	unsigned int  slaveid = *pRevData;
//	const BYTE *pData = (BYTE*)pRevData;
//	wHeadAddr = m_StartAddress;
//
//	CString strTemp;
//	strTemp.Format(L"DEBUG: Modbus Recv[%s]\r\n",Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)pRevData,3).c_str()).c_str());
//	_tprintf(strTemp);
//	
//	if ( (NULL ==pData) || (!Is_DataValid((BYTE *)pData, (WORD)length)) )
//	{
//		return false;
//	}
//	m_oleUpdateTime = COleDateTime::GetCurrentTime();
//	WORD funccode = pRevData[1];
//	if(funccode != 0x06 && funccode != 0x05 && funccode != 0x10)
//		m_nRecvedSlave = slaveid;  //�յ����ɱ��棬�յ��󼴿ɼ����·�.
//
//	int datanum = 0;
//	if (funccode == 1 || funccode == 2)		//����Ȧ  ����ɢ����
//	{
//		datanum = *(pRevData+2);
//		int nUpdateNum = 0;
//		if(!m_bReadOneByOneMode)
//		{
//			nUpdateNum = GetMultiReadCount(slaveid,wHeadAddr,funccode);
//		}
//		for(int nIndex=0; nIndex < datanum; ++nIndex)
//		{
//			wValue = pRevData[3+nIndex];
//			if(m_bReadOneByOneMode)
//			{
//				m_nResponseCount++;
//				DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
//				if (!entry)
//				{	
//					wHeadAddr++;
//					continue;
//				}
//				else
//				{
//					WORD dval =wValue & 1 ;
//					entry->SetWORD(dval);
//					entry->SetUpdated();
//					wHeadAddr++;
//					m_nUpdatePointCount++;
//				}
//			}
//			else
//			{
//				for(int i=0; i<8; ++i)
//				{
//					nUpdateNum--;
//					if(nUpdateNum<0)
//						break;
//					m_nResponseCount++;
//					DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
//					if (!entry){	
//						wHeadAddr++;
//						continue;
//					}
//					else
//					{
//						WORD dval =(wValue>>i) & 1 ;
//						entry->SetWORD(dval);
//						entry->SetUpdated();
//						wHeadAddr++;
//						m_nUpdatePointCount++;
//					}
//				}
//			}
//		}
//	}
//	else if(funccode == 3)					//�����ּĴ���
//	{
//		datanum = *(pRevData+2)/2;
//		for (int nIndex=0; nIndex < datanum; nIndex++)
//		{
//			m_nResponseCount++;
//			DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
//			if (!entry){
//				wHeadAddr++;
//				continue;
//			}
//			else
//			{
//				wValue = pRevData[3+nIndex*2];
//				wValue = (wValue&0x00ff)<<8;
//				wValue |= (pRevData[3+nIndex*2+1]&0x00ff);
//
//				entry->SetWORD(wValue);
//				entry->SetUpdated();
//				wHeadAddr++;
//				m_nUpdatePointCount++;
//			}
//		}
//	}
//	else if(funccode == 4)			//������Ĵ���
//	{
//		datanum = *(pRevData+2)/2;
//		for (int nIndex=0; nIndex < datanum; nIndex++)
//		{
//			m_nResponseCount++;
//			DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
//			if (!entry){
//				wHeadAddr++;
//				continue;
//			}
//			else
//			{
//				wValue = pRevData[3+nIndex*2];
//				wValue = (wValue&0x00ff)<<8;
//				wValue |= (pRevData[3+nIndex*2+1]&0x00ff);
//
//				entry->SetWORD(wValue);
//				entry->SetUpdated();
//				wHeadAddr++;
//				m_nUpdatePointCount++;
//			}
//		}
//	}
//	else if(funccode == 5)			//д������Ȧ ��Ӧ
//	{	
//		wHeadAddr = pRevData[2];
//		wHeadAddr = (wHeadAddr&0x00ff)<<8;
//		wHeadAddr |= (pRevData[3]&0x00ff);
//		wValue =(pRevData[4]>>7) & 1 ;
//		wHeadAddr++;		//��ַ���1
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
//		if (!entry)
//		{
//			return false;
//		}
//		entry->SetWORD(wValue);
//		entry->SetUpdated();
//	}
//	else if(funccode == 6)			//д�����Ĵ���
//	{
//		wHeadAddr = pRevData[2];
//		wHeadAddr = (wHeadAddr&0x00ff)<<8;
//		wHeadAddr |= (pRevData[3]&0x00ff);
//
//		wValue = pRevData[4];
//		wValue = (wValue&0x00ff)<<8;
//		wValue |= (pRevData[5]&0x00ff);
//
//		wHeadAddr++;		//��ַ���1
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 6);
//		if (!entry)
//		{
//			return false;
//		}
//
//		entry->SetWORD(wValue);
//		entry->SetUpdated();
//	}
//	else if(funccode == 0x10)			//д����Ĵ���
//	{
//		wHeadAddr = pRevData[2];
//		wHeadAddr = (wHeadAddr&0x00ff)<<8;
//		wHeadAddr |= (pRevData[3]&0x00ff);
//
//		datanum = pRevData[4];
//		datanum = (datanum&0x00ff)<<8;
//		datanum |= (pRevData[5]&0x00ff);
//	}
//	else if(funccode == 0x81)		//������
//	{
//		WORD errccode = pRevData[2];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 01 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
//		PrintAndSaveLog(strTemp.GetString());
//
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		SetModbusReadUnitErrFlag(slaveid,1,wHeadAddr);
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 1);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_01);
//		}
//	}
//	else if(funccode == 0x82)
//	{
//		WORD errccode = pRevData[2];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 02 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
//		PrintAndSaveLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		SetModbusReadUnitErrFlag(slaveid,2,wHeadAddr);
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 2);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_02);
//		}
//	}
//	else if(funccode == 0x83)
//	{
//		WORD errccode = pRevData[2];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 03 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
//		PrintAndSaveLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		SetModbusReadUnitErrFlag(slaveid,3,wHeadAddr);
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 3);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_03);
//		}
//	}
//	else if(funccode == 0x84)
//	{
//		WORD errccode = pRevData[2];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 04 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
//		PrintAndSaveLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		SetModbusReadUnitErrFlag(slaveid,4,wHeadAddr);
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 4);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_04);
//		}
//	}
//	else if(funccode == 0x85)
//	{
//		WORD errccode = pRevData[2];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 05 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
//		PrintAndSaveLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		SetModbusReadUnitErrFlag(slaveid,5,wHeadAddr);
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 5);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_05);
//		}
//	}
//	else if(funccode == 0x86)
//	{
//		WORD errccode = pRevData[2];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 06 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
//		PrintAndSaveLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		SetModbusReadUnitErrFlag(slaveid,6,wHeadAddr);
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 6);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_06);
//		}
//	}
//	else if(funccode == 0x90)
//	{
//		WORD errccode = pRevData[2];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 16 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
//		PrintAndSaveLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		SetModbusReadUnitErrFlag(slaveid,16,wHeadAddr);
//		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 16);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_16);
//		}
//	}
//	return true;
//}

bool CModbusRTUCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	if(m_nOutPut>0)
	{
		CString strTemp;
		strTemp.Format(L"DEBUG: Modbus(COM:%d Start:%d) Recv[%s]\r\n",m_nPortnr,m_StartAddress,Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)pRevData,3).c_str()).c_str());
		PrintLog(strTemp.GetString(),false);
	}

	SetSendCmdEvent();

	OutPutModbusReceive(pRevData,length,m_StartAddress);

	WORD wHeadAddr=0, wValue=0;
	unsigned int  slaveid = *pRevData;
	const BYTE *pData = (BYTE*)pRevData;
	wHeadAddr = m_StartAddress;

	if ( (NULL ==pData) || (!Is_DataValid((BYTE *)pData, (WORD)length)) )
	{
		return false;
	}
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	WORD funccode = pRevData[1];
	if(funccode != 0x06 && funccode != 0x05 && funccode != 0x10)
		m_nRecvedSlave = slaveid;  //�յ����ɱ��棬�յ��󼴿ɼ����·�.

	int datanum = 0;
	if (funccode == 1 || funccode == 2)		//����Ȧ  ����ɢ����
	{
		datanum = *(pRevData+2);
		int nUpdateNum = 0;
		if(!m_bReadOneByOneMode)
		{
			nUpdateNum = GetMultiReadCount(slaveid,wHeadAddr,funccode);
		}
		for(int nIndex=0; nIndex < datanum; ++nIndex)
		{
			wValue = pRevData[3+nIndex];
			if(m_bReadOneByOneMode)
			{
				m_nResponseCount++;
				DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
				if (!entry)
				{	
					wHeadAddr++;
					continue;
				}
				else
				{
					WORD dval =wValue & 1 ;
					entry->SetWORD(dval);
					entry->SetUpdated();
					wHeadAddr++;
					m_nUpdatePointCount++;
				}
			}
			else
			{
				for(int i=0; i<8; ++i)
				{
					nUpdateNum--;
					if(nUpdateNum<0)
						break;
					m_nResponseCount++;
					DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
					if (!entry){	
						wHeadAddr++;
						continue;
					}
					else
					{
						WORD dval =(wValue>>i) & 1 ;
						entry->SetWORD(dval);
						entry->SetUpdated();
						wHeadAddr++;
						m_nUpdatePointCount++;
					}
				}
			}
		}
	}
	else if(funccode == 3)					//�����ּĴ���
	{
		datanum = *(pRevData+2)/2;
		for (int nIndex=0; nIndex < datanum; nIndex++)
		{
			m_nResponseCount++;
			DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
			if (!entry){
				wHeadAddr++;
				continue;
			}
			else
			{
				wValue = pRevData[3+nIndex*2];
				wValue = (wValue&0x00ff)<<8;
				wValue |= (pRevData[3+nIndex*2+1]&0x00ff);

				entry->SetWORD(wValue);
				entry->SetUpdated();
				wHeadAddr++;
				m_nUpdatePointCount++;
			}
		}
	}
	else if(funccode == 4)			//������Ĵ���
	{
		datanum = *(pRevData+2)/2;
		for (int nIndex=0; nIndex < datanum; nIndex++)
		{
			m_nResponseCount++;
			DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
			if (!entry){
				wHeadAddr++;
				continue;
			}
			else
			{
				wValue = pRevData[3+nIndex*2];
				wValue = (wValue&0x00ff)<<8;
				wValue |= (pRevData[3+nIndex*2+1]&0x00ff);

				entry->SetWORD(wValue);
				entry->SetUpdated();
				wHeadAddr++;
				m_nUpdatePointCount++;
			}
		}
	}
	else if(funccode == 5)			//д������Ȧ ��Ӧ
	{	
		wHeadAddr = pRevData[2];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[3]&0x00ff);
		wValue =(pRevData[4]>>7) & 1 ;
		wHeadAddr++;		//��ַ���1
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
		if (!entry)
		{
			return false;
		}
		entry->SetWORD(wValue);
		entry->SetUpdated();
	}
	else if(funccode == 6)			//д�����Ĵ���
	{
		wHeadAddr = pRevData[2];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[3]&0x00ff);

		wValue = pRevData[4];
		wValue = (wValue&0x00ff)<<8;
		wValue |= (pRevData[5]&0x00ff);

		wHeadAddr++;		//��ַ���1
		
	}
	else if(funccode == 0x10)			//д����Ĵ���
	{
		wHeadAddr = pRevData[2];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[3]&0x00ff);

		datanum = pRevData[4];
		datanum = (datanum&0x00ff)<<8;
		datanum |= (pRevData[5]&0x00ff);
	}
	else if(funccode == 0x81)		//������
	{
		WORD errccode = pRevData[2];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 01 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
		PrintLog(strTemp.GetString(),false);

		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,1,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 1);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_01);
		}
	}
	else if(funccode == 0x82)
	{
		WORD errccode = pRevData[2];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 02 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,2,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 2);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_02);
		}
	}
	else if(funccode == 0x83)
	{
		WORD errccode = pRevData[2];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 03 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,3,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 3);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_03);
		}
	}
	else if(funccode == 0x84)
	{
		WORD errccode = pRevData[2];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 04 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,4,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 4);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_04);
		}
	}
	else if(funccode == 0x85)
	{
		WORD errccode = pRevData[2];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 05 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,5,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 5);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_05);
		}
	}
	else if(funccode == 0x86)
	{
		WORD errccode = pRevData[2];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 06 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,6,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 6);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_06);
		}
	}
	else if(funccode == 0x90)
	{
		WORD errccode = pRevData[2];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 16 Error:id-%d,addr-%d, errcode-%d(COM:%d)\r\n",slaveid, (int)wHeadAddr, (int)errccode,m_nPortnr);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,16,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 16);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_16);
		}
	}
	return true;
}
/***************************************************
*@brief:��ʼ��hashmap����Ϣ
*@param: void
*@return: bool,true���óɹ���false����ʧ�ܡ�
*@author: ZHW 2011-04-01
****************************************************/
bool CModbusRTUCtrl::Init_HashMap(void)
{
	//vector<gDeviceComm>	szVecDeviceComm;    //�豸ͨѶ��Ϣ����
	//if (!CDeviceCommXml::GetInstance()->GetDeviceComm(szVecDeviceComm))
	//{
	//	return false;
	//}
	////��ʼ��hash_map��Ϣ
	//for (vector<gDeviceComm>::iterator it=szVecDeviceComm.begin(); it!=szVecDeviceComm.end(); it++)
	//{
	//	gDeviceRevData szDeviceRevData;
	//	szDeviceRevData.startaddress = it->vecData.begin()->wAddress;
	//	for (vector<gRevData>::iterator itt=it->vecData.begin(); itt!=it->vecData.end(); itt++)
	//	{
	//		gItemData szItemData;
	//		szItemData.wAddress = itt->wAddress;
	//		szItemData.wFunc    = itt->wFunc;
	//		szItemData.strPlcParam = itt->strPlcParam;
	//		szDeviceRevData.mapItemData.insert(make_pair(itt->wAddress, szItemData));

	//		gData szData;
	//		szData.fMultiple = itt->fMultiple;
	//		szData.bWriteToPlc = itt->m_bWritePlc;
	//		szDeviceRevData.mapData.insert(make_pair(itt->strPlcParam, szData));			
	//	}
	//	szDeviceRevData.wCommSlaveID = it->nId;
	//	m_mapDeviceRevData.insert(make_pair(it->nId, szDeviceRevData));
	//}

	return true;
}

/***************************************************
*@brief:��������(����ͨѶ)
*@param: BYTE *pData ��������, BYTE bLength �������ݳ���
*@return:void
*@author: ZHW 2010-08-30
****************************************************/
BOOL CModbusRTUCtrl::Is_DataValid(BYTE *pData, WORD dwLength)
{
	/*�ɹ���ʧ�ܵ��ź�*/
	WORD wCRC			= 0;
	BYTE byData[2]	    = {0};
	if (dwLength <= 2){
		return FALSE;
	}
	wCRC = m_Modbus.CRC16___(pData, dwLength-2);
	memcpy(byData, &wCRC, sizeof(WORD));

	if ((byData[0]==pData[dwLength-1])
		&& (byData[1]==pData[dwLength-2]))
	{
		return TRUE;
	}
	return FALSE;
}

/***************************************************
*@brief:��������(����ͨѶ)
*@param: char *pData ��������, BYTE bLength �������ݳ���
*@return:void
*@author: ZHW 2010-08-10
****************************************************/
void CModbusRTUCtrl::SendCmd(char *pData, BYTE bLength,WORD nStartAddress,bool bWrited)
{
	if(m_bConnectRTUOK)
	{
		if(m_nOutPut>0)
		{
			CString strTemp;
			strTemp.Format(L"DEBUG: Modbus(COM:%d Start:%d) Send[%s]\r\n",m_nPortnr,nStartAddress, Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)pData,bLength).c_str()).c_str());
			PrintLog(strTemp.GetString(),false);
		}

		EnterCriticalSection(&m_CriticalSection);
		if(!bWrited)
			m_StartAddress = nStartAddress;
		WriteToPort(pData, bLength);

		LeaveCriticalSection(&m_CriticalSection);
	}
}

/***************************************************
*@brief:��ʼ��COM
*@param: UINT nPortnr,���ں�  UINT bBaud������
*@return: BOOL, TRUE�ɹ���FALSEʧ��
*@author: ZHW 2011-04-01
****************************************************/
bool CModbusRTUCtrl::InitCOMPort(UINT nPortnr, UINT bBaud, char parity)
{
	if (!InitPort(nPortnr, bBaud, parity) || !StartMonitoring())
	{
		return false;
	}
	return true;
}

/***************************************************
*@brief:TCP/IPͨѶ��������[��ָ��]
*@param: unsigned int nSlaveId //�豸��Slave ID ��
, WORD wAddrFrom		//��ȡ���Ե���ʼ��ַ
, WORD wAddrTo			//��ȡ���Ե���ֹ��ַ
, WORD wFunc);			//�豸ͨѶ����
*@return: bool,true���ͳɹ���false����ʧ�ܡ�
*@author: ZHW 2011-04-01
****************************************************/
bool CModbusRTUCtrl::SendReadCmd(unsigned int nSlaveId, WORD wAddrFrom
								  , WORD wAddrTo, WORD wFunc)

{
	BYTE bLength = 0;
	if(wFunc==0x06 || wFunc==0x10 || wFunc==0x17)
		wFunc = 0x03;
	else if(wFunc==0x05)
		wFunc = 0x01;
	else if(wFunc==0x0F)
		wFunc = 0x01;

	m_nRecvedSlave = -1; //���������Slave
	m_nReadNum = wAddrTo-wAddrFrom+1;

	m_pSendData = (char *)m_Modbus.ReadCmd(nSlaveId, wAddrFrom, wAddrTo, wFunc, bLength);	
	if ((NULL==m_pSendData) || (0 == bLength))
	{
		return false;
	}
	SendCmd(m_pSendData, bLength,wAddrFrom);

	return true;
}

/**********************************************************************
*@brief:TCP/IPͨѶ��������[дָ��]
*@param: unsigned int nSlaveId   //�豸��Slave ID ��
, WORD wAddress			//д�����Եĵ�ַ
, WORD wValue			//����ֵ
, WORD wFunc);			//�豸ͨѶ����
*@return: bool,true�ɹ���falseʧ��
*@author: ZHW 2011-04-01
***********************************************************************/
bool CModbusRTUCtrl::SendWriteCmd(unsigned int nSlaveId, WORD wAddress
								  , WORD wValue, WORD wFunc)

{
	if(wFunc == 16)
	{	
		//
		WORD wCRC = 0;
		char  cSendBuffer[256] = {0};
		ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
		BYTE byteToEx[2] = {0x00, 0x00};		//�ڴ�˳��
		cSendBuffer[0] = (BYTE)nSlaveId;
		cSendBuffer[1] = (BYTE)wFunc;
		cSendBuffer[2] = ((wAddress-1)&0xFF00)>>8;
		cSendBuffer[3] = ((wAddress-1)&0x00FF);

		WORD wNum = 1;
		WORD wRange = 2;
		cSendBuffer[4] = (wNum&0xFF00)>>8;
		cSendBuffer[5] = (wNum&0x00FF);
		cSendBuffer[6] = (BYTE)wRange;
		int nCount = 7;
		memcpy(byteToEx, &wValue, 2);
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
		nCount++;
		wCRC = m_Modbus.CRC16___((unsigned char*)cSendBuffer, nCount);
		memcpy(byteToEx, &wCRC, 2);
		cSendBuffer[nCount] = byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
		int bLength = nCount+1;
		SendCmd(cSendBuffer, bLength,wAddress,true);
		return true;
	}
	else
	{
		BYTE bLength = 0;
		m_pSendData = (char *)m_Modbus.WriteCmd(nSlaveId, wAddress, wValue, wFunc, bLength);	
		if ((NULL==m_pSendData) || (0 == bLength))
		{
			return false;
		}
		SendCmd(m_pSendData, bLength,wAddress,true);
		return true;
	}
	return false;
}

bool CModbusRTUCtrl::SendWriteCmd( DataPointEntry entry,double fValue )
{
	if(entry.GetValueType() == E_MODBUS_BITE)
	{
		return SendWriteBYTECmd(entry,fValue);
	}
	else
	{
		if(entry.GetValueRange() == 0 || entry.GetValueRange() == 1)				//xie
		{
			//����ԭ����
			return SendWriteWORDCmd(entry,fValue);
		}
		else if(entry.GetValueRange() > 1)
		{
			return SendWriteMutilCmd(entry,fValue);
		}
	}
	return false;
}

UINT WINAPI CModbusRTUCtrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CModbusRTUCtrl* pthis = (CModbusRTUCtrl*)lparam;
	if (pthis != NULL)
	{
		pthis->SortPointByDevice_();
		while (!pthis->m_bExitThread)
		{
			if(!pthis->m_bReadOneByOneMode)
			{
				//pthis->SetReadCommands_();
				pthis->SendReadCommandsByActive();
			}
			else
			{
				//pthis->SendOneByOne_();			//һ����һ���㷢��
				pthis->SendOneByOneByActive();
			}

			int nPollSleep = pthis->m_nPollSendInterval;
			while (!pthis->m_bExitThread)
			{
				if(nPollSleep <= 0)
				{
					break;
				}
				nPollSleep--;
				Sleep(1000);
			}
		}
	}

	return 0;
}

void CModbusRTUCtrl::SetReadCommands_()
{
	if(!m_bInitPortSuccess)
		return;

	int i = 0;
	bool bConnectionExist = false; //�����Ƿ�����
	//set command
	SumReadAndResponse();
	for(i=0;i<m_vecPointList.size();i++)
	{
		m_vecPointList[i].SetToUpdate();
	}

	for (i = 0; i < m_ReadUnitList.size(); i++)
	{
		_ModbusReadUnit & mmUnit = m_ReadUnitList[i];
		if(m_nRecvedSlave != mmUnit.nSlaveId)
		{
			Sleep(m_nIDInterval);			//����ID֮���л�ʱ
		}

		if(mmUnit.bHasErrPoint == false  || m_bDisSingleRead == true)
		{
			if(SendReadMutil_(mmUnit))
				bConnectionExist = true;
		}
		else
		{
			if(SendOneByOne_(mmUnit))
				bConnectionExist = true;
		}
		Sleep(10);
	}

	if(bConnectionExist)//ֻҪ�����ɹ���һ�����ݣ���ô����Ϊ������������
		ClearNetworkError();
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read modbus data Un-Recved(COM:%d).\r\n",m_nPortnr);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_UN_REC);
		PrintLog(loginfo.GetString(),false);
	}
}

void CModbusRTUCtrl::SendOneByOne_()
{
	if(!m_bInitPortSuccess)
		return;
	SumReadAndResponse();
	for(int i=0; i<m_vecPointList.size(); ++i)
	{
		DataPointEntry entry = m_vecPointList[i];
		m_vecPointList[i].SetToUpdate();

		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode());
		if(!bSent)
		{
			SetNetworkError();
			continue;;
		}

		//�ȴ���Ӧ�Ļ��ƣ��ǰ�˫������
		//2��
		int nWaitingCount = m_nRecevieTimeOut; 
		while(m_nRecvedSlave!= entry.GetSlaveID() && nWaitingCount>0)
		{
			Sleep(1);
			nWaitingCount--;
		}
		//�ȴ�������Ӧ����ô����һ�Σ�
		if(nWaitingCount<=0)
		{
			//�ٷ�һ��
			SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode());
			nWaitingCount = m_nRecevieTimeOut; 
			while(m_nRecvedSlave!= entry.GetSlaveID() && nWaitingCount>0)
			{
				Sleep(1);
				nWaitingCount--;
			}

			if(nWaitingCount<=0)
			{
				CString loginfo;
				loginfo.Format(L"ERROR: modbus slave(ADDR: %d) no response(COM:%d)!\r\n", entry.GetSlaveID(),m_nPortnr);
				PrintLog(loginfo.GetString(),false);
				CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
			}
		}
		else
		{//ֻҪ�л�Ӧ������Ϊ�ɹ����������ݻز�������������������⣩			
			Sleep(m_nSendReadCmdTimeInterval); 
		}
	}
}

bool CModbusRTUCtrl::SendOneByOne_( _ModbusReadUnit& unit )
{
	unit.bHasErrPoint = false;
	unit.bMultiRead = false;
	bool bConnectionExist = false;
	for(int i=unit.dwAddFrom; i<= unit.dwAddTo; ++i)
	{
		bool bSent = SendReadCmd(unit.nSlaveId,i,i,unit.dwFuncCode);
		if(!bSent)
		{
			SetNetworkError();
			continue;
		}

		//�ȴ���Ӧ�Ļ��ƣ��ǰ�˫������
		//2��
		int nWaitingCount = m_nRecevieTimeOut; 
		while(m_nRecvedSlave!= unit.nSlaveId && nWaitingCount>0)
		{
			Sleep(1);
			nWaitingCount--;
		}
		//�ȴ�������Ӧ�����֮ǰ�й���Ӧ����ô����һ�Σ�
		if(nWaitingCount<=0)
		{
			if(unit.nReadSuccessCount>0)
			{
				//�������ɹ����ģ����ٷ�һ��
				SendReadCmd(unit.nSlaveId,i,i,unit.dwFuncCode);
				nWaitingCount = m_nRecevieTimeOut; 
				while(m_nRecvedSlave!= unit.nSlaveId && nWaitingCount>0)
				{
					Sleep(1);
					nWaitingCount--;
				}

				if(nWaitingCount<=0)
				{
					CString loginfo;
					loginfo.Format(L"ERROR: modbus slave(ADDR: %d From:%d To:%d) no response(COM:%d)!\r\n", unit.nSlaveId,i,i,m_nPortnr);
					PrintLog(loginfo.GetString(),false);
					CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
					unit.nReadSuccessCount = 0; //�����ζ�ʧ�ܣ��´�ֻ��һ�Ρ�
					unit.bHasErrPoint = true;
				}
			}
		}
		else
		{//ֻҪ�л�Ӧ������Ϊ�ɹ����������ݻز�������������������⣩
			if(unit.nReadSuccessCount<10000)
				unit.nReadSuccessCount++;
			Sleep(m_nSendReadCmdTimeInterval); 
			bConnectionExist = true;
		}
	}
	return bConnectionExist;
}

void CModbusRTUCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CModbusRTUCtrl::SetReadOneByOneMode( bool bReadOneByOneMode )
{
	m_bReadOneByOneMode = bReadOneByOneMode;
}

void CModbusRTUCtrl::SetSendCmdTimeInterval( int nTimeMs,int nMutilReadCount/*=99*/,int nIDInterval /*= 500*/ ,int nTimeOut,int nPollInterval,int nPrecision,bool bDisSingleRead,bool bSaveErrLog)
{
	m_nSendReadCmdTimeInterval = nTimeMs;
	m_nMutilReadCount = nMutilReadCount;
	m_nIDInterval = nIDInterval;
	m_nRecevieTimeOut = nTimeOut;
	m_nPollSendInterval = nPollInterval;
	m_nPrecision = nPrecision;
	m_bDisSingleRead = bDisSingleRead;
	m_bSaveErrLog = bSaveErrLog;
}

DataPointEntry* CModbusRTUCtrl::FindEntry_( DWORD slaveid, DWORD headaddress, DWORD funccode )
{
	for (unsigned int i = 0; i < m_vecPointList.size(); i++)
	{
		const DataPointEntry& entry = m_vecPointList[i];
		if(entry.GetSlaveID() == slaveid && entry.GetHeadAddress() == headaddress)
		{
			//ƥ�书����
			WORD wFunc = entry.GetFuncCode();
			if(wFunc==0x06 || wFunc==0x10 || wFunc==0x17)
				wFunc = 0x03;
			else if(wFunc==0x05)
				wFunc = 0x01;
			else if(wFunc==0x0F)
				wFunc = 0x01;

			if(funccode==0x06 || funccode==0x10 || funccode==0x17)
				funccode = 0x03;
			else if(funccode==0x05)
				funccode = 0x01;
			else if(wFunc==0x0F)
				funccode = 0x01;

			if(wFunc == funccode)
				return &m_vecPointList[i];
		}
	}

	return NULL;
}

void CModbusRTUCtrl::SetModbusReadUnitErrFlag( unsigned int nSlaveId,DWORD dwFuncCode,DWORD dwAdd,bool bHasErrPoint/*=true*/ )
{
	for(int i=0; i<m_ReadUnitList.size(); ++i)
	{
		if(m_ReadUnitList[i].nSlaveId==nSlaveId && m_ReadUnitList[i].dwFuncCode==dwFuncCode)
		{
			if(m_ReadUnitList[i].dwAddFrom <= dwAdd && dwAdd <= m_ReadUnitList[i].dwAddTo)
			{
				if(m_ReadUnitList[i].bMultiRead)
					m_ReadUnitList[i].bHasErrPoint = bHasErrPoint;
				else
					m_ReadUnitList[i].bHasErrPoint = false;
				return;
			}
		}
	}
}

bool CModbusRTUCtrl::Init()
{
	if (m_pointlist.empty())
	{
		return false;
	}

	//Sort Point
	//SortPointByDevice_();

	if(!InitCOMPort(m_nPortnr,m_nBaud,m_cParity))
	{
		m_strErrInfo = Project::Tools::OutTimeInfo("connect fail");
		m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
		m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
		return false;
	}
	m_bInitPortSuccess = true;
	m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	return true;
}

bool CModbusRTUCtrl::Exit()
{
	m_bExitThread = true;
	if(m_hsendthread)
	{
		WaitForSingleObject(m_hsendthread, 5000);
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
	if(m_hDataCheckRetryConnectionThread)
	{
		WaitForSingleObject(m_hsendthread, 5000);
		::CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}
	ClosePort(3);
	return true;
}

void CModbusRTUCtrl::SetPortAndBaud( UINT nPortnr, UINT bBaud, char parity /*= 'N'*/ )
{
	m_nPortnr = nPortnr;
	m_nBaud = bBaud;
	m_cParity = parity;
}

void CModbusRTUCtrl::SortPointByDevice_()
{
	//�Ȱ����ͺ�λ���½�����
	m_vecPointList.clear();
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		DataPointEntry entry = m_pointlist[i];
		CreateEntryByDataTypeAndRange(entry,m_vecPointList);
	}

	//���豸�������豸��ַ��������`����ַ��
	int i=0, j=0;
	for(i=0;(i+1)<m_vecPointList.size();i++)
	{
		for(j=i+1; j<m_vecPointList.size();j++)
		{
			DataPointEntry entryi = m_vecPointList[i];
			DataPointEntry entryj = m_vecPointList[j];
			int nDeviceAdd1 = _wtoi(entryi.GetParam(1).c_str());
			int nDeviceAdd2 = _wtoi(entryj.GetParam(1).c_str());

			if(nDeviceAdd1>nDeviceAdd2)
			{
				m_vecPointList[i] = entryj;
				m_vecPointList[j] = entryi;
			}
			else if(nDeviceAdd1==nDeviceAdd2)
			{
				int nFunCode1 = _wtoi(entryi.GetParam(3).c_str());
				int nFunCode2 = _wtoi(entryj.GetParam(3).c_str());
				if(nFunCode1>nFunCode2)
				{
					m_vecPointList[i] = entryj;
					m_vecPointList[j] = entryi;
				}
				else if(nFunCode1 == nFunCode2)
				{
					int nPointAdd1 = _wtoi(entryi.GetParam(2).c_str());
					int nPointAdd2 = _wtoi(entryj.GetParam(2).c_str());
					if(nPointAdd1> nPointAdd2)
					{
						m_vecPointList[i] = entryj;
						m_vecPointList[j] = entryi;
					}
				}
			}
		}
	}

	//���ɶ�ȡ����
	for(i=0;i<m_vecPointList.size();i++)
	{
		_ModbusReadUnit munit;
		munit.strPointName = m_vecPointList[i].GetShortName();
		munit.nSlaveId = _wtoi(m_vecPointList[i].GetParam(1).c_str());
		munit.dwAddFrom = _wtoi(m_vecPointList[i].GetParam(2).c_str());
		munit.dwAddTo = munit.dwAddFrom;
		munit.dwFuncCode = _wtoi(m_vecPointList[i].GetParam(3).c_str());
		munit.nReadSuccessCount = 100;
		munit.bHasErrPoint = false;
		if(!CombineReadUnit(munit,m_vecPointList[i].GetModbusMutileEnd()))
		{
			m_ReadUnitList.push_back(munit);
		}
	}
}

bool CModbusRTUCtrl::CombineReadUnit( const _ModbusReadUnit & mrUnit,bool bModbusEnd/*=true*/ )
{
	if(m_ReadUnitList.size()<=0)
		return false;

	//find unit
	bool bFound = false;
	int i =0;
	for(i=m_ReadUnitList.size()-1;i>=0;i--)
	{
		if(m_ReadUnitList[i].nSlaveId==mrUnit.nSlaveId && m_ReadUnitList[i].dwFuncCode==mrUnit.dwFuncCode)
		{ //����functionҲҪ��ͬ�������������ֻ����Ӧfunction����ĵ�
			bFound = true;
			break;
		}
	}

	if(bFound)
	{
		if(mrUnit.dwAddFrom > (m_ReadUnitList[i].dwAddTo+1))
		{
			return false;
		}
		else if(m_ReadUnitList[i].dwAddTo -m_ReadUnitList[i].dwAddFrom >= m_nMutilReadCount && m_ReadUnitList[i].bModbusEnd == true)		//�������ں� ����������������λ��
		{
			return false;
		}
		else if(mrUnit.dwAddFrom >= m_ReadUnitList[i].dwAddFrom && mrUnit.dwAddFrom>=m_ReadUnitList[i].dwAddTo)
		{
			m_ReadUnitList[i].dwAddTo = mrUnit.dwAddFrom;
			m_ReadUnitList[i].bModbusEnd = bModbusEnd;
		}

		return true; //combined
	}
	else
	{
		return false;//not combined
	}

	return false;
}

void CModbusRTUCtrl::CreateEntryByDataTypeAndRange( DataPointEntry entry,vector<DataPointEntry>& vecModbus )
{
	if(entry.GetValueType() == E_MODBUS_BITE)			//Byte 0-15  �ϲ���һ����λ��ȡ
	{
		bool bExist = false;
		for(int i=0; i<vecModbus.size(); ++i)
		{
			if(vecModbus[i].GetParam(1) == entry.GetParam(1) && vecModbus[i].GetParam(2) == entry.GetParam(2))
			{
				bExist = true;
				break;
			}
		}
		if(!bExist)
			vecModbus.push_back(entry);
	}
	else if(entry.GetValueType() == E_MODBUS_POWERLINK)			//ֻ��ǰ��λ
	{
		int nDataRange = 2;
		for(int i=0; i<nDataRange; ++i)
		{
			DWORD nAddress = entry.GetHeadAddress();
			CString strNewParm2;
			strNewParm2.Format(_T("%d"),nAddress+i);
			DataPointEntry entryCopy = entry;
			entryCopy.SetParam(2,strNewParm2.GetString());
			if(i < nDataRange-1)
				entryCopy.SetModbusMutileEnd(false);
			vecModbus.push_back(entryCopy);
		}
	}
	else
	{
		int nDataRange =entry.GetValueRange();
		if(nDataRange <= 1)
		{
			vecModbus.push_back(entry);
		}
		else
		{
			for(int i=0; i<nDataRange; ++i)
			{
				DWORD nAddress = entry.GetHeadAddress();
				CString strNewParm2;
				strNewParm2.Format(_T("%d"),nAddress+i);
				DataPointEntry entryCopy = entry;
				entryCopy.SetParam(2,strNewParm2.GetString());
				if(i < nDataRange-1)
					entryCopy.SetModbusMutileEnd(false);
				vecModbus.push_back(entryCopy);
			}
		}
	}
}

bool CModbusRTUCtrl::SendReadMutil_( _ModbusReadUnit& mmUnit )
{
	mmUnit.bHasErrPoint = false;
	mmUnit.bMultiRead = true;
	int slaveid = mmUnit.nSlaveId;
	DWORD headaddressFrom = mmUnit.dwAddFrom;
	DWORD headaddressTo = mmUnit.dwAddTo;
	DWORD funccode = mmUnit.dwFuncCode;
	if (slaveid < 0 || headaddressFrom < 0 || headaddressTo<headaddressFrom || funccode == 0 || funccode > 17)
	{
		// this point entry is not valid.
		CString loginfo;
		loginfo.Format(L"ERROR: modbus entry %s params not valid, skip(COM:%d)!\r\n", mmUnit.strPointName.c_str(),m_nPortnr);
		PrintLog(loginfo.GetString(),false);
		CDebugLog::GetInstance()->SetErrPoint(mmUnit.strPointName,POINTTYPE_MODBUS_,ERROR_CUSTOM_MODBUS_INVALID);
		return false;
	}

	bool bSent = SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode);
	if(!bSent)
	{
		SetNetworkError();
		//�ҵ����е�
		for(int i= mmUnit.dwAddFrom; i<mmUnit.dwAddTo; ++i)
		{
			DataPointEntry* entry = FindEntry_(mmUnit.nSlaveId, i, mmUnit.dwFuncCode);
			if(entry)
			{		
				CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_SEND_MODBUS);
			}
		}
		CString loginfo;
		loginfo.Format(L"ERROR: modbus entry %s send fail(COM:%d).\r\n", mmUnit.strPointName.c_str(),m_nPortnr);
		PrintLog(loginfo.GetString(),false);
		return false;
	}
	return bSent;
}

void CModbusRTUCtrl::SetNetworkError()
{
	if(m_nModbusNetworkErrorCount<10000)
		m_nModbusNetworkErrorCount++;
}

void CModbusRTUCtrl::ClearNetworkError()
{
	m_nModbusNetworkErrorCount = 0;
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
}

void CModbusRTUCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry&  modbusentry = m_pointlist[i];
		GetValueByDataTypeAndRange(modbusentry,valuelist);
	}

	if(m_pointlist.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		CString strName;
		strName.Format(_T("%s%d"),TIME_UPDATE_MODBUS,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
		strName.Format(_T("%s%d"),LOG_MODBUS,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str())));
	}
}

void CModbusRTUCtrl::GetValueByDataTypeAndRange( DataPointEntry entry,vector< pair<wstring, wstring> >& valuelist )
{
	//PrintLog(L"Start to GetValueByDataTypeAndRange\r\n");
	if(entry.GetValueType() == E_MODBUS_BITE)		//BYTE
	{
		//PrintLog(L"Start to GetValueByDataTypeAndRange E_MODBUS_BITE\r\n");
		vector<WORD> vecValue;
		bool bHasFault = false;
		for(int i=0; i<1; ++i)
		{
			DataPointEntry* entryFind = FindEntry_(entry.GetSlaveID(), entry.GetHeadAddress()+i, entry.GetFuncCode());
			if(entryFind)
			{
				if(entryFind->GetUpdateSignal()<10)
				{
					vecValue.push_back(entryFind->GetWORD());
				}
				else
				{
					PrintLog(L"GetValueByDataTypeAndRange Start to GetValueByDataTypeAndRange has fault\r\n");
					bHasFault = true;
					break;
				}
			}
		}

		if(!bHasFault)
		{
			GetValueByDataTypeAndRange(entry,E_MODBUS_VALUE_TYPE(entry.GetValueType()),vecValue,valuelist);
		}
	}
	else if(entry.GetValueType() == E_MODBUS_POWERLINK)		//E_MODBUS_POWERLINK
	{
		//PrintLog(L"Start to GetValueByDataTypeAndRange E_MODBUS_POWERLINK\r\n");
		int nDataRange = 2;
		vector<WORD> vecValue;
		bool bHasFault = false;
		nDataRange = (nDataRange<=0)?1:nDataRange;
		for(int i=0; i<nDataRange; ++i)
		{
			DataPointEntry* entryFind = FindEntry_(entry.GetSlaveID(), entry.GetHeadAddress()+i, entry.GetFuncCode());
			if(entryFind)
			{
				if(entryFind->GetUpdateSignal()<10)
				{
					vecValue.push_back(entryFind->GetWORD());
				}
				else
				{
					bHasFault = true;
					break;
				}
			}
		}

		if(!bHasFault)
		{
			GetValueByDataTypeAndRange(entry,E_MODBUS_VALUE_TYPE(entry.GetValueType()),vecValue,valuelist);
		}
	}
	else
	{
		//PrintLog(L"Start to GetValueByDataTypeAndRange of other type\r\n");
		int nDataRange = entry.GetValueRange();
		vector<WORD> vecValue;
		bool bHasFault = false;
		nDataRange = (nDataRange<=0)?1:nDataRange;
		int nAddress = 0;
		for(int i=0; i<nDataRange; ++i)
		{
			DataPointEntry* entryFind = FindEntry_(entry.GetSlaveID(), entry.GetHeadAddress()+i, entry.GetFuncCode());
			if(entryFind)
			{
				if(entryFind->GetUpdateSignal()<10)
				{
					vecValue.push_back(entryFind->GetWORD());
				}
				else
				{
					nAddress = entry.GetHeadAddress()+i;
					bHasFault = true;
					break;
				}

				//CString strTemp;
				//strTemp.Format(_T("Name:%s, Value:%d\r\n"), entry.GetShortName().c_str(), vecValue.back());
				//PrintLog(strTemp.GetString());
			}
			else
			{
				PrintLog(L"GetValueByDataTypeAndRange NO entry found\r\n");
			}
		}

		if(!bHasFault)
		{
			GetValueByDataTypeAndRange(entry,E_MODBUS_VALUE_TYPE(entry.GetValueType()),vecValue,valuelist);
		}
		else
		{

		}
	}
}

void CModbusRTUCtrl::GetValueByDataTypeAndRange( DataPointEntry entry,E_MODBUS_VALUE_TYPE type,vector<WORD>& vecValue,vector< pair<wstring, wstring> >& valuelist )
{
	if(vecValue.size() <= 0)
		return;

	switch(type)
	{
	case E_MODBUS_SIGNED:
		{
			WORD wValue = vecValue[0];
			int nFlag = (wValue>>15) & 1 ;			//��һλ�Ƿ�Ϊ1 ��Ϊ1��Ϊ����
			int nValue = wValue;
			if(nFlag >0 )		//����
			{
				nValue = nValue-65536;
			}
			double dval = entry.GetMultipleReadValue((double)nValue);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_UNSIGNED:
		{
			WORD wValue = vecValue[0];
			int nValue = wValue;
			double dval = entry.GetMultipleReadValue((double)nValue);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_BITE:
		{
			WORD wValue = vecValue[0];
			int nDataRange = entry.GetValueRange();
			int nFlag = GetBitFromWord(wValue,nDataRange);
			double dval = entry.GetMultipleReadValue((double)nFlag);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_LONG:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0],false);
			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue((double)vt.lVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_LONG_INVERSE:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0]);
			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue((double)vt.lVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_FLOAT:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0],false);
			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue((double)vt.fltVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_FLOAT_INVERSE:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0]);
			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue((double)vt.fltVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_DOUBLE:
		{
			if(vecValue.size() < 4)
				break;
			double dval = entry.GetMultipleReadValue(MergeFourDouble(vecValue[3],vecValue[2],vecValue[1],vecValue[0],false));
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_DOUBLE_INVERSE:
		{
			if(vecValue.size() < 4)
				break;
			double dval = entry.GetMultipleReadValue(MergeFourDouble(vecValue[3],vecValue[2],vecValue[1],vecValue[0]));
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_STRING:
		{
			char cWarnig[256] = {0};
			for(int i=0; i<vecValue.size(); ++i)
			{
				memcpy(cWarnig+2*i,&vecValue[i],2);
			}
			wstring strValueSet = Project::Tools::AnsiToWideChar(cWarnig);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_STRING_INVERSE:
		{
			char cWarnig[256] = {0};
			BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��
			for(int i=0; i<vecValue.size(); ++i)
			{
				memcpy(byteToEx, &vecValue[i], 2);
				memcpy(cWarnig+2*i,&byteToEx[1],1);
				memcpy(cWarnig+2*i+1,&byteToEx[0],1);
			}
			wstring strValueSet = Project::Tools::AnsiToWideChar(cWarnig);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_POWERLINK:
		{
			if(vecValue.size() < 2)
				break;
			double dval = entry.GetMultipleReadValue((double)MergeTwoWORD(vecValue[1],vecValue[0],false));
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	default:
		break;
	}
}

void CModbusRTUCtrl::GetValueByDataTypeAndRange( DataPointEntry entry,double dValue,vector<WORD>& vecCmd,bool bOrder )
{
	vecCmd.clear();
	switch(entry.GetValueType())
	{
	case E_MODBUS_SIGNED:			//2016-03-18 ����
		{
			VARIANT vt;
			vt.lVal = dValue;
			WORD dwValue = vt.uintVal;
			vecCmd.push_back(dwValue);
		}
		break;
	case E_MODBUS_UNSIGNED:			//2016-03-18 ����
		{
			VARIANT vt;
			vt.lVal = dValue;
			WORD dwValue = vt.uintVal;
			vecCmd.push_back(dwValue);
		}
		break;
	case E_MODBUS_LONG:
		{
			VARIANT vt;
			vt.lVal = dValue;
			DWORD dwValue = vt.ulVal;
			WORD wLow = LOWORD(dwValue);
			WORD wHigh = HIWORD(dwValue);
			if(bOrder)
			{
				vecCmd.push_back(wHigh);
				vecCmd.push_back(wLow);
			}
			else
			{
				vecCmd.push_back(wLow);
				vecCmd.push_back(wHigh);
			}
		}
		break;
	case E_MODBUS_FLOAT:
		{
			VARIANT vt;
			vt.fltVal = dValue;
			DWORD dwValue = vt.ulVal;
			WORD wLow = LOWORD(dwValue);
			WORD wHigh = HIWORD(dwValue);
			if(bOrder)
			{
				vecCmd.push_back(wHigh);
				vecCmd.push_back(wLow);
			}
			else
			{
				vecCmd.push_back(wLow);
				vecCmd.push_back(wHigh);
			}
		}
		break;
	case E_MODBUS_FLOAT_INVERSE:
		{
			VARIANT vt;
			vt.fltVal = dValue;
			DWORD dwValue = vt.ulVal;
			WORD wLow = LOWORD(dwValue);
			WORD wHigh = HIWORD(dwValue);
			if(!bOrder)
			{
				vecCmd.push_back(wHigh);
				vecCmd.push_back(wLow);
			}
			else
			{
				vecCmd.push_back(wLow);
				vecCmd.push_back(wHigh);
			}
		}
		break;
	case E_MODBUS_LONG_INVERSE:
		{
			VARIANT vt;
			vt.lVal = dValue;
			DWORD dwValue = vt.ulVal;
			WORD wLow = LOWORD(dwValue);
			WORD wHigh = HIWORD(dwValue);
			if(!bOrder)
			{
				vecCmd.push_back(wHigh);
				vecCmd.push_back(wLow);
			}
			else
			{
				vecCmd.push_back(wLow);
				vecCmd.push_back(wHigh);
			}
		}
		break;
	case E_MODBUS_DOUBLE:
		{
			VARIANT vt;
			vt.dblVal = dValue;
			ULONGLONG ullVal = vt.ullVal;
			DWORD dwHigh =  ((DWORD)((((LONG64)(ullVal)) >> 32) & 0xffffffff));
			WORD wHLow = LOWORD(dwHigh);
			WORD wHHigh = HIWORD(dwHigh);
			DWORD dwLow =  ((DWORD)(((LONG64)(ullVal)) & 0xffffffff));
			WORD wLLow = LOWORD(dwLow);
			WORD wLHigh = HIWORD(dwLow);
			if(bOrder)
			{
				vecCmd.push_back(wHHigh);
				vecCmd.push_back(wHLow);
				vecCmd.push_back(wLHigh);
				vecCmd.push_back(wLLow);
			}
			else
			{
				vecCmd.push_back(wLLow);
				vecCmd.push_back(wLHigh);
				vecCmd.push_back(wHLow);
				vecCmd.push_back(wHHigh);
			}
		}
		break;
	case E_MODBUS_DOUBLE_INVERSE:
		{
			VARIANT vt;
			vt.dblVal = dValue;
			ULONGLONG ullVal = vt.ullVal;
			DWORD dwHigh =  ((DWORD)((((LONG64)(ullVal)) >> 32) & 0xffffffff));
			WORD wHLow = LOWORD(dwHigh);
			WORD wHHigh = HIWORD(dwHigh);
			DWORD dwLow =  ((DWORD)(((LONG64)(ullVal)) & 0xffffffff));
			WORD wLLow = LOWORD(dwLow);
			WORD wLHigh = HIWORD(dwLow);
			if(!bOrder)
			{
				vecCmd.push_back(wHHigh);
				vecCmd.push_back(wHLow);
				vecCmd.push_back(wLHigh);
				vecCmd.push_back(wLLow);
			}
			else
			{
				vecCmd.push_back(wLLow);
				vecCmd.push_back(wLHigh);
				vecCmd.push_back(wHLow);
				vecCmd.push_back(wHHigh);
			}
		}
		break;
	case E_MODBUS_POWERLINK:			//doubleֵ ��������Ϊ ���� С������Ϊ������  2λС��
		{
			DWORD nInt = dValue;
			CString strValue;
			strValue.Format(_T("%.2f"),dValue);
			WORD nDecimal = _wtof(strValue)*100-nInt*100;
			WORD wHLow = LOWORD(nInt);
			WORD wHHigh = HIWORD(nInt);
			vecCmd.push_back(wHLow);
			vecCmd.push_back(wHHigh);
			vecCmd.push_back(nDecimal);
		}
		break;
	default:
		break;
	}
}

bool CModbusRTUCtrl::GetValue( const wstring& pointname, double& result )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
			result =  entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	return false;
}

bool CModbusRTUCtrl::SetValue( const wstring& pointname, double fValue )
{
	if(SetValueByActive(pointname, fValue))		//дֵ�ɹ�
	{
		//дֵ��ȡ��ֵ


		return true;
	}
	return false;
}

bool CModbusRTUCtrl::SendWriteMutilCmd( DataPointEntry entry,double fValue )
{
	DWORD nSlaveId = entry.GetSlaveID();
	DWORD wAddress = entry.GetHeadAddress();
	DWORD wFunc = entry.GetFuncCode();
	if (nSlaveId == 0 || wAddress == 0 || wFunc == 0)
	{
		CString loginfo;
		loginfo.Format(L"ERROR: modbus write point params invalid(COM:%d)!\r\n",m_nPortnr);
		PrintLog(loginfo.GetString(),false);
		CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
		return false;
	}

	if (wFunc == 03)
	{
		wFunc = 0x10;
	}

	vector<WORD> vecCmd;
	GetValueByDataTypeAndRange(entry,fValue,vecCmd,false);
	if(vecCmd.size() <= 0)
	{
		CString loginfo;
		loginfo.Format(L"ERROR: modbus write point value invalid(COM:%d)!\r\n",m_nPortnr);
		PrintLog(loginfo.GetString(),false);
		return false;
	}

	//
	WORD wCRC = 0;
	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	BYTE byteToEx[2] = {0x00, 0x00};		//�ڴ�˳��
	cSendBuffer[0] = (BYTE)nSlaveId;
	cSendBuffer[1] = (BYTE)wFunc;
	cSendBuffer[2] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[3] = ((wAddress-1)&0x00FF);

	WORD wNum = vecCmd.size();
	WORD wRange = wNum*2;
	cSendBuffer[4] = (wNum&0xFF00)>>8;
	cSendBuffer[5] = (wNum&0x00FF);
	cSendBuffer[6] = (BYTE)wRange;
	int nCount = 7;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 7+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}
	nCount++;
	wCRC = m_Modbus.CRC16___((unsigned char *)cSendBuffer, nCount);
	memcpy(byteToEx, &wCRC, 2);
	cSendBuffer[nCount] = byteToEx[1];
	nCount++;
	cSendBuffer[nCount] = byteToEx[0];
	int bLength = nCount+1;
	SendCmd(cSendBuffer, bLength,wAddress,true);
	return true;
}

bool CModbusRTUCtrl::SendWriteBYTECmd( DataPointEntry entry,double fValue )
{
	//����ԭ����
	DWORD slaveid = entry.GetSlaveID();
	DWORD headaddress = entry.GetHeadAddress();
	DWORD funccode = entry.GetFuncCode();
	if (slaveid == 0 || headaddress == 0 || funccode == 0)
	{
		CString loginfo;
		loginfo.Format(L"ERROR: modbus write point params invalid(COM:%d)!\r\n",m_nPortnr);
		PrintLog(loginfo.GetString(),false);
		CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
		return false;
	}

	//golding: ע�������������, modbusдֵ���ܻ������
	if (funccode == 03){
		//��ֹ����Ŀ����û�����öԡ��������з�����
		funccode = 06;
	}

	//�ҵ��õ�ֵ
	DataPointEntry* entry_ = FindEntry_(slaveid, headaddress, 6);
	if (entry_)
	{
		WORD wValue = entry_->GetWORD();
		wValue = SetBitToWord(wValue,entry.GetValueRange(),(int)fValue);
		bool bSent = SendWriteCmd(slaveid, headaddress,wValue, funccode);
		if(bSent)
		{
			entry_->SetWORD(wValue);
		}
		return bSent;
	}
	return false;
}

bool CModbusRTUCtrl::SendWriteWORDCmd( DataPointEntry entry,double fValue )
{
	//����ԭ����
	DWORD slaveid = entry.GetSlaveID();
	DWORD headaddress = entry.GetHeadAddress();
	DWORD funccode = entry.GetFuncCode();
	if (slaveid == 0 || headaddress == 0 || funccode == 0)
	{
		CString loginfo;
		loginfo.Format(L"ERROR: modbus write point params invalid(COM:%d)!\r\n",m_nPortnr);
		PrintLog(loginfo.GetString(),false);
		CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
		return false;
	}

	//golding: ע�������������, modbusдֵ���ܻ������
	if (funccode == 03){
		//��ֹ����Ŀ����û�����öԡ��������з�����
		funccode = 06;
	}

	bool bSent = SendWriteCmd(slaveid, headaddress, (WORD)(fValue), funccode);
	if(bSent)
	{
		DataPointEntry* entry_ = FindEntry_(slaveid, headaddress, 6);
		if (entry_)
		{
			entry_->SetWORD((WORD)(fValue));
		}
	}
	return bSent;
}

int CModbusRTUCtrl::GetBitFromWord( WORD dwValue, int nIndex )
{
	WORD dwTemp = dwValue<<(16-nIndex-1);
	int nTemp = dwTemp>> 15;

	return nTemp;
}

WORD CModbusRTUCtrl::SetBitToWord( WORD nData, int nIndex,int nValue )
{
	nData &= ~(1<<nIndex); //��*num�ĵ�posλ����Ϊ0
	nData |= nValue<<nIndex;
	return nData;
}

DWORD CModbusRTUCtrl::MergeTwoWORD( WORD wGao,WORD wDi,bool bOrder /*= true*/ )
{
	if(!bOrder)
	{
		DWORD dwHourTotal;
		dwHourTotal = wGao;
		dwHourTotal = dwHourTotal<< 16;
		dwHourTotal = dwHourTotal | (wDi&0x0000FFFF);
		return dwHourTotal;
	}
	else
	{
		DWORD dwHourTotal;
		dwHourTotal = wDi;
		dwHourTotal = dwHourTotal<< 16;
		dwHourTotal = dwHourTotal | (wGao&0x0000FFFF);
		return dwHourTotal;
	}
}

double CModbusRTUCtrl::MergeFourDouble( WORD wGaoGao,WORD wGao,WORD wDi,WORD wDiDi,bool bOrder /*= true*/ )
{
	if(!bOrder)
	{
		wstring strValue;
		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wDiDi,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wDi,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wGao,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wGaoGao,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		return strtodbl(strValue.c_str());
	}
	else
	{
		wstring strValue;
		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wGaoGao,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wGao,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wDi,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wDiDi,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		return strtodbl(strValue.c_str());
	}
}

double CModbusRTUCtrl::strtodbl( const wchar_t * str )
{
	long long dbl = 0;  
	for(int i=0;i<63;i++)
	{  
		dbl += (str[i]-'0');  
		dbl <<= 1;  
	}  
	dbl +=(str[63]-'0');  
	double* db = (double*)&dbl;  
	return *db;
}

bool CModbusRTUCtrl::InitDataEntryValue( const wstring& pointname, double value )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
			entry.SetValue(value);
			return true;
		}
	}

	return false;
}

void CModbusRTUCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

void CModbusRTUCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

UINT WINAPI CModbusRTUCtrl::ThreadCheckAndRetryConnection( LPVOID lparam )
{
	CModbusRTUCtrl* pthis = (CModbusRTUCtrl*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			COleDateTimeSpan oleSpanUpdate =  COleDateTime::GetCurrentTime() - pthis->m_oleUpdateTime;
			if(oleSpanUpdate.GetTotalMinutes()>=5)
			{
				if(pthis->ReInitPort(3))
					pthis->ClearNetworkError();
			}

			//���´洢log
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - pthis->m_oleLastUpdateLog;
			if(oleSpan.GetTotalMinutes() >= 5)
			{
				pthis->SaveLog();
				pthis->m_oleLastUpdateLog = COleDateTime::GetCurrentTime();
			}

			int nPollSleep = 30;
			while (!pthis->m_bExitThread)
			{
				if(nPollSleep <= 0)
				{
					break;
				}
				nPollSleep--;
				Sleep(1000);
			}
		}
	}
	return 0;
}

bool CModbusRTUCtrl::ReInitPort(int nType)
{
	EnterCriticalSection(&m_CriticalSection);
	CString strLog;
	strLog.Format(_T("INFO : ReConnect Modbus RTU(Port:%d)...\r\n"),m_nPortNr);
	PrintLog(strLog.GetString(),true);
	m_bInitPortSuccess = ReConnect() && StartMonitoring();
	if(m_bInitPortSuccess)
	{
		strLog.Format(_T("INFO : ReConnect Modbus RTU(Port:%d) Success(%d).\r\n"),m_nPortNr,nType);
		PrintLog(strLog.GetString(),true);
	}
	else
	{
		strLog.Format(_T("INFO : ReConnect Modbus RTU(Port:%d) Failed(%d).\r\n"),m_nPortNr,nType);
		PrintLog(strLog.GetString(),true);
	}
	LeaveCriticalSection(&m_CriticalSection);
	return m_bInitPortSuccess;
}

bool CModbusRTUCtrl::OutPutModbusReceive( const unsigned char* pRevData, DWORD length ,WORD wStart)
{
	if(m_nOutPut>0)
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
				strLogPath.Format(_T("%s\\modbus_debug_%d_%02d_%02d.log"),strPath.c_str(),oleNow.GetYear(),oleNow.GetMonth(),oleNow.GetDay());

				CString strLog;
				std::ostringstream sqlstream;
				sqlstream << oleNow.GetHour() << ":" << oleNow.GetMinute() << ":" << oleNow.GetSecond() << "(COM" <<m_nPortNr  << ":"<< wStart  <<") [" << charToHexStr((unsigned char*)pRevData,length) << "]\n";
				strLog = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
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
		return true;
	}
	return false;
}

bool CModbusRTUCtrl::SetDebug( int nDebug )
{
	m_nOutPut = nDebug;
	return true;
}

string* CModbusRTUCtrl::byteToHexStr( unsigned char* byte_arr, int arr_len )
{
	string* hexstr=new string();  
	for (int i=0;i<arr_len;i++)  
	{  
		char hex1;  
		char hex2;  
		/*����C++֧�ֵ�unsigned��int��ǿ��ת������unsigned char��ֵ��int��ֵ����ôϵͳ�ͻ��Զ����ǿ��ת��*/  
		int value=byte_arr[i];  
		int S=value/16;  
		int Y=value % 16;  
		//��C++��unsigned char��int��ǿ��ת���õ�����ת����ĸ  
		if (S>=0&&S<=9)  
			hex1=(char)(48+S);  
		else  
			hex1=(char)(55+S);  
		//��C++��unsigned char��int��ǿ��ת���õ�������ת����ĸ  
		if (Y>=0&&Y<=9)  
			hex2=(char)(48+Y);  
		else  
			hex2=(char)(55+Y);  
		//���һ���Ĵ���ʵ�֣������õ���������ĸ���ӳ��ַ����ﵽĿ��  
		*hexstr=*hexstr+hex1+hex2;  
	}  
	return hexstr;  
}

string CModbusRTUCtrl::charToHexStr( unsigned char* byte_arr, int arr_len )
{
	std::ostringstream sqlstream;
	for (int i=0;i<arr_len;i++)  
	{  
		char hex1;  
		char hex2;  
		/*����C++֧�ֵ�unsigned��int��ǿ��ת������unsigned char��ֵ��int��ֵ����ôϵͳ�ͻ��Զ����ǿ��ת��*/  
		int value=byte_arr[i];  
		int S=value/16;  
		int Y=value % 16;  
		//��C++��unsigned char��int��ǿ��ת���õ�����ת����ĸ  
		if (S>=0&&S<=9)  
			hex1=(char)(48+S);  
		else  
			hex1=(char)(55+S);  
		//��C++��unsigned char��int��ǿ��ת���õ�������ת����ĸ  
		if (Y>=0&&Y<=9)  
			hex2=(char)(48+Y);  
		else  
			hex2=(char)(55+Y);  
		//���һ���Ĵ���ʵ�֣������õ���������ĸ���ӳ��ַ����ﵽĿ��  
		sqlstream << hex1 << hex2 << " ";
	}  
	return sqlstream.str().c_str();  
}

int CModbusRTUCtrl::GetMultiReadCount( DWORD slaveid, DWORD headaddress, DWORD funccode )
{
	for(int i=0; i<m_ReadUnitList.size(); ++i)
	{
		WORD wFunc = m_ReadUnitList[i].dwFuncCode;
		if(wFunc==0x06 || wFunc==0x10 || wFunc==0x17)
			wFunc = 0x03;
		else if(wFunc==0x05)
			wFunc = 0x01;
		else if(wFunc==0x0F)
			wFunc = 0x01;

		if(m_ReadUnitList[i].nSlaveId == slaveid && wFunc == funccode)
		{
			if(m_ReadUnitList[i].dwAddFrom <= headaddress && m_ReadUnitList[i].dwAddTo >= headaddress)
				return m_ReadUnitList[i].dwAddTo-m_ReadUnitList[i].dwAddFrom+1;
		}
	}
	return 0;
}

bool CModbusRTUCtrl::InsertLog( const wstring& loginfo, COleDateTime oleTime )
{
	if(m_logsession)
		return m_logsession->InsertLog(loginfo);
	return false;
}

bool CModbusRTUCtrl::SaveLog()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock_log);
	map<wstring,COleDateTime>::iterator iterErrLog = m_mapErrLog.begin();
	while(iterErrLog != m_mapErrLog.end())
	{
		InsertLog((*iterErrLog).first,(*iterErrLog).second);
		++iterErrLog;
	}
	m_mapErrLog.clear();
	return true;
}

DataPointEntry* CModbusRTUCtrl::FindEntryFromQueryCache_( DWORD slaveid, DWORD headaddress, DWORD funccode )
{
	EnterCriticalSection(&m_CriticalPointSection);
	DataPointEntry* entryFind = NULL;
	hash_map<wstring,DataPointEntry*>::iterator iterPoint = m_mapPointQuery.begin();
	while(iterPoint != m_mapPointQuery.end())
	{
		DataPointEntry* entry = (*iterPoint).second;
		if(entry)
		{
			if(entry->GetSlaveID() == slaveid && entry->GetHeadAddress() == headaddress)
			{
				WORD wFunc = entry->GetFuncCode();
				if(wFunc==0x06 || wFunc==0x10 || wFunc==0x17)
					wFunc = 0x03;
				else if(wFunc==0x05)
					wFunc = 0x01;
				else if(wFunc==0x0F)
					wFunc = 0x01;

				if(wFunc == funccode || entry->GetFuncCode() == funccode)
				{
					entryFind = entry;
					break;
				}
			}
		}
		++iterPoint;
	}
	LeaveCriticalSection(&m_CriticalPointSection);
	return entryFind;
}

void CModbusRTUCtrl::SumReadAndResponse()
{
	if(m_nCmdCount > 0)			//ͳ����Ϣ
	{
		std::ostringstream sqlstream;
		m_nResponseCount = (m_nResponseCount>m_nCmdCount)?m_nUpdatePointCount:m_nResponseCount;
		sqlstream << "Read(" << m_nCmdCount << ") Response(" << m_nResponseCount << ") UpdatePoint(" << m_nUpdatePointCount << ")";
		m_strUpdateLog = sqlstream.str();

		CString strOut;
		strOut.Format(_T("INFO : Modbus RTU(%d):%s.\r\n"),m_nEngineIndex,Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str()).c_str());
		PrintLog(strOut.GetString(),false);
	}
	m_nCmdCount = 0;
	m_nResponseCount = 0;
	m_nUpdatePointCount = 0;
}

void CModbusRTUCtrl::SendReadCommandsByActive()
{
	if(!m_bInitPortSuccess)
		return;

	if(m_vecPointList.size() <= 0)
		return;

	int i = 0;
	bool bConnectionExist = false; //�����Ƿ�����
	SumReadAndResponse();
	for(i=0;i<m_vecPointList.size();i++)
	{
		m_vecPointList[i].SetToUpdate();
	}

	for (i = 0; i < m_ReadUnitList.size(); i++)
	{
		_ModbusReadUnit & mmUnit = m_ReadUnitList[i];
		if(m_lastReadUnit.nSlaveId != mmUnit.nSlaveId)
		{
			Sleep(m_nIDInterval);			//����ID֮���л�ʱ
		}

		if(mmUnit.bHasErrPoint == false  || m_bDisSingleRead == true)
		{
			if(SendReadMutilByActive(mmUnit))
				bConnectionExist = true;
		}
		else
		{
			if(SendOneByOneByActive(mmUnit))
				bConnectionExist = true;
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}

	if(bConnectionExist)//ֻҪ�����ɹ���һ�����ݣ���ô����Ϊ������������
		ClearNetworkError();
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read modbus data Un-Recved(COM:%d).\r\n",m_nPortnr);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_UN_REC);
		PrintLog(loginfo.GetString(),false);
	}
}

void CModbusRTUCtrl::SetSendCmdEvent()
{
	SetEvent(m_sendCmdEvent);
}

HANDLE CModbusRTUCtrl::GetSendCmdEvent()
{
	return m_sendCmdEvent;
}

void CModbusRTUCtrl::SendOneByOneByActive()
{
	if(!m_bInitPortSuccess)
		return;

	if(m_vecPointList.size() <= 0)
		return;

	SumReadAndResponse();

	for(int i=0; i<m_vecPointList.size(); ++i)
	{
		DataPointEntry entry = m_vecPointList[i];
		m_vecPointList[i].SetToUpdate();
		
		m_lastReadEntry = entry;
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode());
		if(!bSent)
		{
			SetNetworkError();
		}
		else
		{
			//ʹ�ü���ģʽ
			DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
			if(dwObject == WAIT_TIMEOUT)		// ��ʱ
			{
				CString loginfo;
				loginfo.Format(L"ERROR: Modbus TimeOut(COM:%d ID:%d Fun:%02d From:%d To:%d)!\r\n", m_nPortnr,m_lastReadEntry.GetSlaveID(),m_lastReadEntry.GetFuncCode(),m_lastReadEntry.GetHeadAddress(),m_lastReadEntry.GetHeadAddress());
				PrintLog(loginfo.GetString(),false);
				CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
			}

			ClearNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
}

bool CModbusRTUCtrl::SendOneByOneByActive( _ModbusReadUnit& unit )
{
	unit.bHasErrPoint = false;
	unit.bMultiRead = false;
	bool bConnectionExist = false;

	for(int i=unit.dwAddFrom; i<= unit.dwAddTo; ++i)
	{
		Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock_send_log);		//������
		m_lastReadUnit = unit;
		bool bSent = SendReadCmd(unit.nSlaveId,i,i,unit.dwFuncCode);
		if(!bSent)
		{
			SetNetworkError();
		}
		else
		{
			//ʹ�ü���ģʽ
			DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
			if(dwObject == WAIT_TIMEOUT)		// ��ʱ
			{
				CString loginfo;
				loginfo.Format(L"ERROR: Modbus TimeOut(COM:%d ID:%d Fun:%02d From:%d To:%d)!\r\n", m_nPortnr,m_lastReadUnit.nSlaveId,m_lastReadUnit.dwFuncCode,m_lastReadUnit.dwAddFrom, m_lastReadUnit.dwAddTo);
				PrintLog(loginfo.GetString(),false);
				CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
			}

			bConnectionExist = true;
			ClearNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
	return bConnectionExist;
}

void CModbusRTUCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());

	if(bSaveLog)
	{
		Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock_log);
		m_mapErrLog[strLog] = COleDateTime::GetCurrentTime();
	}
}

bool CModbusRTUCtrl::SendReadMutilByActive( _ModbusReadUnit& mmUnit )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock_send_log);		//������
	m_lastReadUnit = mmUnit;

	if(SendReadMutil_(mmUnit))
	{
		//ʹ�ü���ģʽ
		DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
		if(dwObject == WAIT_TIMEOUT)		// ��ʱ
		{
			CString loginfo;
			loginfo.Format(L"ERROR: Modbus RTU(COM:%d) TimeOut(ID:%d Fun:%02d From:%d To:%d)!\r\n", m_nPortnr,m_lastReadUnit.nSlaveId,m_lastReadUnit.dwFuncCode,m_lastReadUnit.dwAddFrom, m_lastReadUnit.dwAddTo);
			PrintLog(loginfo.GetString(),false);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
			m_lastReadUnit.nReadSuccessCount = 0; //�����ζ�ʧ�ܣ��´�ֻ��һ�Ρ�
			m_lastReadUnit.bHasErrPoint = true;
		}
		else if(dwObject == WAIT_OBJECT_0)
		{
			return true;
		}
	}
	return false;
}

bool CModbusRTUCtrl::SetValueByActive( const wstring& pointname, double fValue )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock_send_log);		//������
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			fValue = entry.GetMultipleReadValue(fValue,false); 			//edit by robert
			bool bSent = SendWriteCmd(entry,fValue);
			if(bSent)
			{
				DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
				if(dwObject == WAIT_TIMEOUT)		// ��ʱ
				{
					return false;
				}
				else if(dwObject == WAIT_OBJECT_0)
				{
					//дֵ�ɹ����ȡ
					return true;
				}
			}
			else
			{
				CString loginfo;
				loginfo.Format(L"ERROR: modbus write point send package failed(COM:%d).\r\n",m_nPortnr);
				PrintLog(loginfo.GetString(),false);

				CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
				SetNetworkError();
			}
			return false;
		}
	}
	return false;
}
