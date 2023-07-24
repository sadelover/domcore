#pragma once
#include "StdAfx.h"
#include "ModbusTcpCtrl.h"
#include "atlstr.h"

#include "Tools/Maths/MathCalcs.h"
#include "Tools/EngineInfoDefine.h"
//#include "Tools/vld.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../LAN_WANComm/Tools/ToolsFunction/PingIcmp.h"
#include "../COMdll//DebugLog.h"

const int default_portnumber = 502;

CModbusTcpCtrl::CModbusTcpCtrl(void)
	:m_host("")
	,m_port(default_portnumber)
	,m_hsendthread(NULL)
	,m_nSendReadCmdTimeInterval(30)
	,m_nRecevieTimeOut(5000)
	,m_nRecvedSlave(-1)
	,m_hDataCheckRetryConnectionThread(NULL)
	,m_nModbusNetworkErrorCount(0)
	,m_bExitThread(false)
	,m_bSortPoint(false)
	,m_nSendCmdCount(0)
	,m_bReadOneByOneMode(false)
	,m_nMutilReadCount(99)
	,m_nIDInterval(500)
	,m_nPollSendInterval(2)
	,m_nOutPut(0)
	,m_nPrecision(6)
	,m_bDisSingleRead(false)
	,m_nCmdCount(0)
	,m_nResponseCount(0)
	,m_nUpdatePointCount(0)
	,m_strUpdateLog("")
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_strErrInfo = "";
	m_nEngineIndex = 0;
	m_vecPointList.clear();
	m_ModbusValue.clear();
	m_sendCmdEvent = ::CreateEvent(NULL, false, false, L"");
}

CModbusTcpCtrl::~CModbusTcpCtrl(void)
{
	if (m_hsendthread != NULL)
	{
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
}

//golding ,���÷��Ͷ�ȡ����ĺ�����
void    CModbusTcpCtrl::SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount,int nIDInterval,int nTimeOut,int nPollInterval,int nPrecision,bool bDisSingleRead)
{
	m_nSendReadCmdTimeInterval = nTimeMs;
	m_nMutilReadCount = nMutilReadCount;
	m_nIDInterval = nIDInterval;
	m_nRecevieTimeOut = nTimeOut;
	m_nPollSendInterval = nPollInterval;
	m_nPrecision = nPrecision;
	m_bDisSingleRead = bDisSingleRead;
}

/*
 * the modbus tcp protocal
	The request and response are prefixed by six bytes as follows

	byte 0:	transaction identifier - copied by server - usually 0
	byte 1:	transaction identifier - copied by server - usually 0
	byte 2:	protocol identifier = 0
	byte 3:	protocol identifier = 0
	byte 4:	length field (upper byte) = 0 (since all messages are smaller than 256)
	byte 5:	length field (lower byte) = number of bytes following
	byte 6:	unit identifier (previously ��slave address��)
	byte 7:	MODBUS function code
	byte 8 on:	data bytes.
	byte 9...  the real datas. each data is two bytes.

	So an example transaction ��read 1 register at offset 4 from UI 9�� returning a value of 5 would be
	request:  00 00 00 00 00 06 09 03 00 04 00 01
	response: 00 00 00 00 00 05 09 03 02 00 05
*/

// notify received package, use for sub-class derivativation
//bool CModbusTcpCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
//{
//	//20 00 00 00 00 2b 01 03 28 05 2a ba a4 00 00 03 01 03 00 00 00 00 00 00 00 00 00 00 00 00 
//	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
//	WORD wHeadAddr=0, wValue=0;
//    
//    unsigned int  slaveid = *(pRevData+6);
//
//	
//	m_nRecvedSlave = slaveid;  //�յ����ɱ��棬�յ��󼴿ɼ����·�.
//	m_oleUpdateTime = COleDateTime::GetCurrentTime();
//    memcpy(&wHeadAddr, pRevData, 2);
//	WORD funccode = pRevData[7];
//	int datanum = 0;
//	if (funccode == 1 || funccode == 2)		//����Ȧ  ����ɢ����
//	{
//		datanum = *(pRevData+8);
//		for(int nIndex=0; nIndex < datanum; ++nIndex)
//		{
//			wValue = pRevData[9+nIndex];
//			for(int i=0; i<8; ++i)
//			{
//				DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//				if (!entry){					
//					continue;
//				}
//				double dval =(wValue>>i) & 1 ;
//				dval = dval/entry->GetMultiple();
//
//				entry->SetValue(dval);
//				entry->SetUpdated();
//				wHeadAddr++;
//			}
//		}
//	}
//	else if(funccode == 3)					//�����ּĴ���
//	{
//		datanum = *(pRevData+8)/2;
//		for (int nIndex=0; nIndex < datanum; nIndex++)
//		{
//			DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//			if (!entry){
//				CString strTemp;
//				strTemp.Format(L"ERROR: modbus recv ErrorData: %d,%d,%d\r\n",slaveid, (int)wHeadAddr, (int)funccode);
//				PrintLog(strTemp.GetString());
//				continue;
//			}
//
//			wValue = pRevData[9+nIndex*2];
//			wValue = (wValue&0x00ff)<<8;
//			wValue |= (pRevData[9+nIndex*2+1]&0x00ff);
//
//			int nFlag = (pRevData[9+nIndex*2]>>7) & 1 ;			//��һλ�Ƿ�Ϊ1 ��Ϊ1��Ϊ����
//			int nValue = wValue;
//			if(nFlag >0 )		//����
//			{
//				nValue = nValue-65536;
//			}
//			double dval = nValue/entry->GetMultiple();
//			entry->SetValue(dval);
//			entry->SetUpdated();
//
//			wHeadAddr++;
//		}
//	}
//	else if(funccode == 4)			//������Ĵ���
//	{
//		datanum = *(pRevData+8)/2;
//		for (int nIndex=0; nIndex < datanum; nIndex++)
//		{
//			DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//			if (!entry){
//				CString strTemp;
//				strTemp.Format(L"ERROR: modbus recv ErrorData: %d,%d,%d\r\n",slaveid, (int)wHeadAddr, (int)funccode);
//				PrintLog(strTemp.GetString());
//				continue;
//			}
//
//			wValue = pRevData[9+nIndex*2];
//			wValue = (wValue&0x00ff)<<8;
//			wValue |= (pRevData[9+nIndex*2+1]&0x00ff);
//
//			int nFlag = (pRevData[9+nIndex*2]>>7) & 1 ;			//��һλ�Ƿ�Ϊ1 ��Ϊ1��Ϊ����
//			int nValue = wValue;
//			if(nFlag >0 )		//����
//			{
//				nValue = nValue-65536;
//			}
//			double dval = nValue/entry->GetMultiple();
//
//			entry->SetValue(dval);
//			entry->SetUpdated();
//
//			wHeadAddr++;
//		}
//	}
//	else if(funccode == 5)			//д������Ȧ ��Ӧ
//	{		
//		wHeadAddr = pRevData[8];
//		wHeadAddr = (wHeadAddr&0x00ff)<<8;
//		wHeadAddr |= (pRevData[9]&0x00ff);
//		wValue =(pRevData[10]>>7) & 1 ;
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if (!entry)
//		{
//			return false;
//		}
//		double dval = wValue/entry->GetMultiple();
//		entry->SetValue(dval);
//		entry->SetUpdated();
//	}
//	else if(funccode == 6)			//д�����Ĵ���
//	{
//		wHeadAddr = pRevData[8];
//		wHeadAddr = (wHeadAddr&0x00ff)<<8;
//		wHeadAddr |= (pRevData[9]&0x00ff);
//		
//		wValue = pRevData[10];
//		wValue = (wValue&0x00ff)<<8;
//		wValue |= (pRevData[11]&0x00ff);
//		
//
//		wHeadAddr++;		//��ַ���1
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, 6);
//		if (!entry)
//		{
//			return false;
//		}
//
//		int nFlag = (pRevData[10]>>7) & 1 ;			//��һλ�Ƿ�Ϊ1 ��Ϊ1��Ϊ����
//		int nValue = wValue;
//		if(nFlag >0 )		//����
//		{
//			nValue = nValue-65536;
//		}
//		double dval = nValue/entry->GetMultiple();
//
//		entry->SetValue(dval);
//		entry->SetUpdated();
//	}
//	else if(funccode == 0x81)		//������
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 01 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x82)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 02 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x83)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 03 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x84)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 04 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x85)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 05 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x86)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 06 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	return true;
//}

//edit by robert 20151113
bool CModbusTcpCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
{
	//20 00 00 00 00 2b 01 03 28 05 2a ba a4 00 00 03 01 03 00 00 00 00 00 00 00 00 00 00 00 00 
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(length <= 8)
		return false;

	CString strTemp;
	wstring wstrContent =  Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)pRevData,length).c_str());
	if(m_nOutPut>0)
	{
		strTemp.Format(L"DEBUG: Modbus Recv[%s]\r\n",wstrContent.c_str());
		PrintLog(strTemp.GetString(),false);
	}


	SetSendCmdEvent();//�յ����¼���ʾ�����յ�

	WORD wLenth = 0;
	wLenth = pRevData[4];
	wLenth = (wLenth&0x00ff)<<8;
	wLenth |= (pRevData[5]&0x00ff);

	if (length < wLenth+6)
		return false;


	//golding ����У��
	//unsigned int nCRCResult = CalcCrcFast((unsigned char*)pRevData,length-2);
	//unsigned int nCRCResult2 = CalcCrcFast((unsigned char*)(pRevData+6),length-2-6);

	OutPutModbusReceive(pRevData,length);

	WORD wHeadAddr=0, wValue=0;
	unsigned int  slaveid = *(pRevData+6);

	m_nRecvedSlave = slaveid;  //�յ����ɱ��棬�յ��󼴿ɼ����·�.
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	memcpy(&wHeadAddr, pRevData, 2);
	WORD funccode = pRevData[7];
	int datanum = 0;
	if (funccode == 1 || funccode == 2)		//����Ȧ  ����ɢ����
	{
		datanum = *(pRevData+8);
		int nUpdateNum = 0;
		if(!m_bReadOneByOneMode)
		{
			nUpdateNum = GetMultiReadCount(slaveid,wHeadAddr,funccode);
		}
		for(int nIndex=0; nIndex < datanum; ++nIndex)
		{
			wValue = pRevData[9+nIndex];
			if(m_bReadOneByOneMode)				//�����ȡģʽ
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
					WORD dval = wValue & 1 ;
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
					
					if (!entry)
					{	
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
		datanum = *(pRevData+8)/2;
		for (int nIndex=0; nIndex < datanum; nIndex++)
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
				wValue = pRevData[9+nIndex*2];
				wValue = (wValue&0x00ff)<<8;
				wValue |= (pRevData[9+nIndex*2+1]&0x00ff);

				entry->SetWORD(wValue);
				if(wValue==0 || wValue==1)
					int bDebug=1;
				entry->SetUpdated();
				wHeadAddr++;
				m_nUpdatePointCount++;
			}
		}
	}
	else if(funccode == 4)			//������Ĵ���
	{
		datanum = *(pRevData+8)/2;
		for (int nIndex=0; nIndex < datanum; nIndex++)
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
				wValue = pRevData[9+nIndex*2];
				wValue = (wValue&0x00ff)<<8;
				wValue |= (pRevData[9+nIndex*2+1]&0x00ff);

				entry->SetWORD(wValue);
				entry->SetUpdated();
				wHeadAddr++;
				m_nUpdatePointCount++;
			}
		}
	}
	else if(funccode == 5)			//д������Ȧ ��Ӧ
	{		
		wHeadAddr = pRevData[8];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[9]&0x00ff);
		wValue =(pRevData[10]>>7) & 1 ;
		wHeadAddr++;		//��ַ���1

	}
	else if(funccode == 6)			//д�����Ĵ���
	{
		wHeadAddr = pRevData[8];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[9]&0x00ff);
			
		wValue = pRevData[10];
		wValue = (wValue&0x00ff)<<8;
		wValue |= (pRevData[11]&0x00ff);
			
	}
	else if(funccode == 0x10)			//д����Ĵ���
	{
		wHeadAddr = pRevData[8];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[9]&0x00ff);

		wValue = pRevData[10];
		wValue = (wValue&0x00ff)<<8;
		wValue |= (pRevData[11]&0x00ff);

		wHeadAddr++;		//��ַ���1		
	}
	else if(funccode == 0x81)		//������
	{
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 01 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
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
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 02 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
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
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 03 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
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
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 04 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
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
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 05 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
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
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 06 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
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
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 16 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
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
*@author: ZHW 2011-02-28
****************************************************/

/***************************************************
*@brief:ModbusTcp�������Ӳ�ͨѶ
*@param: u_short uPort�˿�, string strIp�����ַ
*@return: bool,true���óɹ���false����ʧ�ܡ�
*@author: ZHW 2011-02-24
****************************************************/
bool CModbusTcpCtrl::Init()
{
	// if there is not modbus point, thread will not be created.
	if (m_pointlist.empty())
	{
		return false;
	}

	//Sort Point
	//SortPointByDevice_();

	if (!TcpIpConnectComm(m_host, m_port))
	{
		m_strErrInfo = Project::Tools::OutTimeInfo("connect fail");
		m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
		m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
		return false;
	}
	m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	
	return true;
}

void CModbusTcpCtrl::SortPointByDevice()
{
	//���豸�������豸��ַ������ַ��
	int i=0, j=0;
	for(i=0;(i+1)<m_pointlist.size();i++)
	{
		for(j=i+1; j<m_pointlist.size();j++)
		{
			DataPointEntry entryi = m_pointlist[i];
			DataPointEntry entryj = m_pointlist[j];
			int nDeviceAdd1 = _wtoi(entryi.GetParam(1).c_str());
			int nDeviceAdd2 = _wtoi(entryj.GetParam(1).c_str());

			if(nDeviceAdd1>nDeviceAdd2)
			{
				m_pointlist[i] = entryj;
				m_pointlist[j] = entryi;
			}
			else if(nDeviceAdd1==nDeviceAdd2)
			{
				int nPointAdd1 = _wtoi(entryi.GetParam(2).c_str());
				int nPointAdd2 = _wtoi(entryj.GetParam(2).c_str());
				if(nPointAdd1> nPointAdd2)
				{
					m_pointlist[i] = entryj;
					m_pointlist[j] = entryi;
				}
			}
		}
	}


	//���ɶ�ȡ����
	for(i=0;i<m_pointlist.size();i++)
	{
		_ModbusReadUnit munit;
		munit.strPointName = m_pointlist[i].GetShortName();
		munit.nSlaveId = _wtoi(m_pointlist[i].GetParam(1).c_str());
		munit.dwAddFrom = _wtoi(m_pointlist[i].GetParam(2).c_str());
		munit.dwAddTo = munit.dwAddFrom;
		munit.dwFuncCode = _wtoi(m_pointlist[i].GetParam(3).c_str());
		munit.nReadSuccessCount = 100;
		if(!CombineReadUnit(munit))
		{
			m_ReadUnitList.push_back(munit);
		}
	}
}

void    CModbusTcpCtrl::SetNetworkError(int nErrCountAdd)
{
	if(m_nModbusNetworkErrorCount<10000)
	{
		m_nModbusNetworkErrorCount+= nErrCountAdd;
	}
		
}
void    CModbusTcpCtrl::ClearNetworkError()
{
	m_nModbusNetworkErrorCount = 0;
}

bool CModbusTcpCtrl::CombineReadUnit(const _ModbusReadUnit &  mrUnit,bool bModbusEnd)
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

/***************************************************
*@brief:TCP/IPͨѶ��������[��ָ��]
*@param: unsigned int nSlaveId //�豸��Slave ID ��
, WORD wAddrFrom		//��ȡ���Ե���ʼ��ַ
, WORD wAddrTo			//��ȡ���Ե���ֹ��ַ
, WORD wFunc);			//�豸ͨѶ����
*@return: bool,true���ͳɹ���false����ʧ�ܡ�
*@author: ZHW 2011-01-07
****************************************************/
bool CModbusTcpCtrl:: SendReadCmd(unsigned int nSlaveId, WORD wAddrFrom
								  , WORD wAddrTo, WORD wFunc, int &nErrCode)

{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);

	if(wFunc==0x06 || wFunc==0x10 || wFunc==0x17)
		wFunc = 0x03;
	else if(wFunc==0x05)
		wFunc = 0x01;
	else if(wFunc==0x0F)
		wFunc = 0x01;

	m_nRecvedSlave = -1; //���������Slave
		
	ZeroMemory(m_SendBuffer, sizeof(m_SendBuffer));
	memcpy(m_SendBuffer, &wAddrFrom, sizeof(WORD));

	m_SendBuffer[5] = 0x06;
	m_SendBuffer[6] = (BYTE)nSlaveId;
	m_SendBuffer[7] = (BYTE)wFunc;
	m_SendBuffer[8] = ((wAddrFrom-1)&0xFF00)>>8;
	m_SendBuffer[9] = ((wAddrFrom-1)&0x00FF);
	m_SendBuffer[10] = ((wAddrTo-wAddrFrom+1)&0xFF00)>>8;
	m_SendBuffer[11] = ((wAddrTo-wAddrFrom+1)&0x00FF);

	//SendMessage
	wstring wstrBuffer = Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)m_SendBuffer,sizeof(m_SendBuffer)).c_str());
	if(m_nOutPut>0)
	{
		CString strTemp;
		strTemp.Format(L"DEBUG: Modbus Send[%s]\r\n",wstrBuffer.c_str());
		PrintLog(strTemp.GetString(),false);
	}
	return SendPackage(m_SendBuffer, sizeof(m_SendBuffer),nErrCode);

}

/**********************************************************************
*@brief:TCP/IPͨѶ��������[дָ��]
*@param: unsigned int nSlaveId   //�豸��Slave ID ��
, WORD wAddress			//д�����Եĵ�ַ
, WORD wValue			//����ֵ
, WORD wFunc);			//�豸ͨѶ����
*@return: bool,true�ɹ���falseʧ��
*@author: ZHW 2011-01-24
***********************************************************************/
bool CModbusTcpCtrl::SendWriteCmd(unsigned int nSlaveId, WORD wAddress
								  , WORD wValue, WORD wFunc)

{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);
	if(wFunc == 16)
	{
		char  cSendBuffer[256] = {0};
		ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
		memcpy(cSendBuffer, &wAddress, sizeof(WORD));

		//////////////////////////////////////////////////////////////////////////
		cSendBuffer[6] = (BYTE)nSlaveId;
		cSendBuffer[7] = (BYTE)wFunc;
		cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
		cSendBuffer[9] = ((wAddress-1)&0x00FF);
		WORD wNum = 1;
		WORD wRange = 2;
		cSendBuffer[10] = (wNum&0xFF00)>>8;
		cSendBuffer[11] = (wNum&0x00FF);
		cSendBuffer[12] = (BYTE)wRange;
		BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��
		int nCount = 13;
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress, 16);
		if (entry)
		{
			entry->SetWORD(wValue);
		}
		memcpy(byteToEx, &wValue, 2);
		nCount = 13;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
		WORD wCount = nCount-5;
		cSendBuffer[5] = (BYTE)wCount;

		int nErrCode = 0;
		if(m_nOutPut>0)
		{
			CString strTemp;
			strTemp.Format(L"DEBUG: Modbus SendWrite[%s]\r\n",Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)m_SendBuffer,sizeof(m_SendBuffer)).c_str()).c_str());
			PrintLog(strTemp.GetString(),false);
		}
		return SendPackage(cSendBuffer, nCount+1, nErrCode);
	}
	else
	{
		BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��
		ZeroMemory(m_SendBuffer, sizeof(m_SendBuffer));
		memcpy(m_SendBuffer, &wAddress, sizeof(WORD));

		if (wFunc == 06)
		{
			m_SendBuffer[5] = 0x06;
			m_SendBuffer[6] = (BYTE)nSlaveId;
			m_SendBuffer[7] = (BYTE)wFunc;
			m_SendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
			m_SendBuffer[9] = ((wAddress-1)&0x00FF);
			memcpy(byteToEx, &wValue, 2);
			m_SendBuffer[10] = byteToEx[1];
			m_SendBuffer[11] = byteToEx[0];
		}
		else
		{
			m_SendBuffer[5] = 0x06;
			m_SendBuffer[6] = (BYTE)nSlaveId;
			m_SendBuffer[7] = 0x05;
			m_SendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
			m_SendBuffer[9] = ((wAddress-1)&0x00FF);
			memcpy(byteToEx, &wValue, 2);

			if (wValue > 0)
			{
				m_SendBuffer[10] = 0xFF;
				m_SendBuffer[11] = 0;
			}
			else
			{
				m_SendBuffer[10] = 0;
				m_SendBuffer[11] = 0;
			}	
		}
		//SendMessage

		int nErrCode = 0;
		if(m_nOutPut>0)
		{
			CString strTemp;
			strTemp.Format(L"DEBUG: Modbus SendWrite[%s]\r\n",Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)m_SendBuffer,sizeof(m_SendBuffer)).c_str()).c_str());
			PrintLog(strTemp.GetString(),false);
		}
		return SendPackage(m_SendBuffer, sizeof(m_SendBuffer), nErrCode);
	}
	return false;
}

void CModbusTcpCtrl::SetHost( const string& strhost )
{
	m_host = strhost;
}

void CModbusTcpCtrl::SetPort( u_short portnumer )
{
	m_port = portnumer;
}

UINT CModbusTcpCtrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CModbusTcpCtrl* pthis = (CModbusTcpCtrl*)lparam;
	if (pthis != NULL)
	{
		pthis->SortPointByDevice_();

		while (!pthis->m_bExitThread)
		{
			if(!pthis->m_bReadOneByOneMode)
			{
				pthis->SendReadCommandsByActive();
			}
			else
			{
				pthis->SendOneByOneByActive();			//һ����һ���㷢��
			}

			int nPollSleep = pthis->m_nPollSendInterval;
			if(nPollSleep<1)
				nPollSleep = 1;

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

void CModbusTcpCtrl::TerminateSendThread()
{
	m_bExitThread = true;
	WaitForSingleObject(m_hsendthread, INFINITE);
	WaitForSingleObject(m_hDataCheckRetryConnectionThread,INFINITE);
}

void CModbusTcpCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CModbusTcpCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
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

DataPointEntry* CModbusTcpCtrl::FindEntry(DWORD slaveid, DWORD headaddress, DWORD funccode)
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if ((entry.GetSlaveID() == slaveid) && (entry.GetHeadAddress() == headaddress)){		//edit by robert 20140715 дֵʧ�� ȡ��������ƥ��
			return &m_pointlist[i];
		}
	}

	return NULL;
}

//bool	CModbusTcpCtrl::SetValue(const wstring& pointname, double fValue)
//{
//	for (unsigned int i = 0; i < m_pointlist.size(); i++)
//	{
//		const DataPointEntry& entry = m_pointlist[i];
//		if (entry.GetShortName() == pointname)
//		{
//
//			DWORD slaveid = entry.GetSlaveID();
//			DWORD headaddress = entry.GetHeadAddress();
//			DWORD funccode = entry.GetFuncCode();
//
//			if (slaveid == 0 || headaddress == 0 || funccode == 0)
//			{
//				PrintLog(L"ERROR: modbus write point params invalid!\r\n");
//
//				return false;
//			}
//
//			//golding: ע�������������, modbusдֵ���ܻ������
//			if (funccode == 03){
//				//��ֹ����Ŀ����û�����öԡ��������з�����
//				funccode = 06;
//			}
//
//			fValue = fValue*entry.GetMultiple();			//edit by robert
//
//			bool bSent = SendWriteCmd(slaveid, headaddress, (WORD)(fValue), funccode);
//			if(!bSent)//��������������
//			{
//				PrintLog(L"ERROR: modbus write point send package failed.\r\n");
//				SetNetworkError();
//			}
//			return true;
//		}
//	}
//
//	return false;
//}

bool	CModbusTcpCtrl::SetValue(const wstring& pointname, double fValue)
{
	if(!m_bConnectOK)
		return false;

	CString strTemp;

	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			fValue = entry.GetMultipleReadValue(fValue,false);			//edit by robert
			bool bSent = SendWriteCmd(entry,fValue);
			if(!bSent)//��������������
			{
				strTemp.Format(L"ERROR: modbus write point(%s->%.4f) send package failed.\r\n",pointname.c_str(), fValue);
				PrintLog(strTemp.GetString(),false);
				CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
				SetNetworkError();
			}
			Sleep(200);//���������Ϣ�����������������ᱻ�ϲ���һ����������ʧ��, 20211229 golding���³�ҫ���ֵ�bug
			return true;
		}
	}

	return false;
}

bool CModbusTcpCtrl::GetValue( const wstring& pointname, double& result )
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

bool CModbusTcpCtrl::InitDataEntryValue(const wstring& pointname, double value)
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

void CModbusTcpCtrl::AddLog( const wchar_t* loginfo )
{
	if (m_logsession){
		m_logsession->InsertLog(loginfo);
		Sleep(50);
	}
}

void CModbusTcpCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CModbusTcpCtrl::ReConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	CString strInfo;
	wstring wstrHost ;
	Project::Tools::UTF8ToWideChar(m_host, wstrHost);
	strInfo.Format(L"INFO : Modbus reconnecting(%s:%d)\r\n", wstrHost.c_str(), m_port);
	PrintLog( strInfo.GetString(), false);
	Disconnect(); //tcpip recv thread stop, and clear socket

	if (!TcpIpConnectComm(m_host, m_port))
	{
		PrintLog(L"ERROR: Modbus reconnecting failed!\r\n");
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_RECONNECT_MODBUS);
		return false;
	}
	m_strErrInfo = "";
	PrintLog(L"INFO : Modbus reconnecting success!\r\n",false);
	return true;
}

UINT WINAPI CModbusTcpCtrl::ThreadCheckAndRetryConnection(LPVOID lparam)
{
	CModbusTcpCtrl* pthis = (CModbusTcpCtrl*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(pthis->m_nModbusNetworkErrorCount>=60 || !pthis->m_bConnectOK)
			{
				if(pthis->ReConnect())
					pthis->ClearNetworkError();
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

bool CModbusTcpCtrl::Exit()
{
	m_bExitThread = true;
	Disconnect();		//�ر�TCP

	if(m_hDataCheckRetryConnectionThread)
	{
		WaitForSingleObject(m_hDataCheckRetryConnectionThread,5000);
		CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}

	if(m_hsendthread)
	{
		WaitForSingleObject(m_hsendthread, 5000);
		CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
	
	return true;
}

void CModbusTcpCtrl::SendReadCommandOneByOne()
{
	m_bSortPoint = true;
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		DataPointEntry entry = m_pointlist[i];

		int nErrCode = 0;
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode(), nErrCode);
		if(!bSent)
		{
			if(nErrCode==10065)
				SetNetworkError(10);
			else
				SetNetworkError();
			CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_SEND_ONE_MODBUS);
		}
		Sleep(10);
	}

	//5��
	Sleep(5000);

	m_backuppointlist.clear();
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		wstring strPointName = m_pointlist[i].GetShortName();
		map<wstring,DataPointEntry*>::iterator iter = m_mapErrPoint.find(strPointName);
		if(iter != m_mapErrPoint.end())
		{
			continue;
		}
		m_backuppointlist.push_back(m_pointlist[i]);
	}

	SortBackupPointByDevice();
	m_mapErrPoint.clear();
	m_bSortPoint = false;
}

void CModbusTcpCtrl::SortBackupPointByDevice()
{
	//���豸�������豸��ַ������ַ��
	int i=0, j=0;
	for(i=0;(i+1)<m_backuppointlist.size();i++)
	{
		for(j=i+1; j<m_backuppointlist.size();j++)
		{
			DataPointEntry entryi = m_backuppointlist[i];
			DataPointEntry entryj = m_backuppointlist[j];
			int nDeviceAdd1 = _wtoi(entryi.GetParam(1).c_str());
			int nDeviceAdd2 = _wtoi(entryj.GetParam(1).c_str());

			if(nDeviceAdd1>nDeviceAdd2)
			{
				m_backuppointlist[i] = entryj;
				m_backuppointlist[j] = entryi;
			}
			else if(nDeviceAdd1==nDeviceAdd2)
			{
				int nPointAdd1 = _wtoi(entryi.GetParam(2).c_str());
				int nPointAdd2 = _wtoi(entryj.GetParam(2).c_str());
				if(nPointAdd1> nPointAdd2)
				{
					m_backuppointlist[i] = entryj;
					m_backuppointlist[j] = entryi;
				}
			}
		}
	}


	//���ɶ�ȡ����
	m_ReadUnitList.clear();
	for(i=0;i<m_backuppointlist.size();i++)
	{
		_ModbusReadUnit munit;
		munit.strPointName = m_backuppointlist[i].GetShortName();
		munit.nSlaveId = _wtoi(m_backuppointlist[i].GetParam(1).c_str());
		munit.dwAddFrom = _wtoi(m_backuppointlist[i].GetParam(2).c_str());
		munit.dwAddTo = munit.dwAddFrom;
		munit.dwFuncCode = _wtoi(m_backuppointlist[i].GetParam(3).c_str());
		munit.nReadSuccessCount = 100;
		munit.bHasErrPoint = false;
		if(!CombineReadUnit(munit))
		{
			m_ReadUnitList.push_back(munit);
		}
	}
}

void CModbusTcpCtrl::SetBackupReadCommands()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	if(m_bSortPoint)		//���ŵ�� ��������
		return;

	if(m_nSendCmdCount >= 100)			//����100�κ���������� ������һ��
	{
		m_nSendCmdCount = 0;
		if(m_pointlist.size() != m_backuppointlist.size())
		{
			SendReadCommandOneByOne();		//����
			return;
		}
	}

	if(m_mapErrPoint.size()>0)
	{
		SendReadCommandOneByOne();		//����
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;

	int i = 0;
	bool bConnectionExist = false; //�����Ƿ�����
	//set command
	for(i=0;i<m_backuppointlist.size();i++)
	{
		m_backuppointlist[i].SetToUpdate();
	}

	for (i = 0; i < m_ReadUnitList.size(); i++)
	{
		_ModbusReadUnit & mmUnit = m_ReadUnitList[i];

		int slaveid = mmUnit.nSlaveId;
		DWORD headaddressFrom = mmUnit.dwAddFrom;
		DWORD headaddressTo = mmUnit.dwAddTo;
		DWORD funccode = mmUnit.dwFuncCode;
		if (slaveid < 0 || headaddressFrom < 0 || headaddressTo<headaddressFrom || funccode == 0 || funccode > 17)
		{
			// this point entry is not valid.
			CString loginfo;
			loginfo.Format(L"ERROR: modbus entry %s params not valid, skip!\r\n", mmUnit.strPointName.c_str());
			PrintLog(loginfo.GetString(),false);
			CDebugLog::GetInstance()->SetErrPoint(mmUnit.strPointName,POINTTYPE_MODBUS_,ERROR_CUSTOM_MODBUS_INVALID);
			continue;
		}

		int nErrCode = 0;
		bool bSent = SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode, nErrCode);
		if(!bSent)
		{
			if(nErrCode==10065)
				SetNetworkError(10);
			else
				SetNetworkError();
			continue;
		}

		//�ȴ���Ӧ�Ļ��ƣ��ǰ�˫������
		//10��
		int nWaitingCount = m_nRecevieTimeOut; 
		while(m_nRecvedSlave!= mmUnit.nSlaveId && nWaitingCount>0)
		{
			Sleep(1);
			nWaitingCount--;
		}
		//�ȴ�������Ӧ�����֮ǰ�й���Ӧ����ô����һ�Σ�
		if(nWaitingCount<=0)
		{
			if(mmUnit.nReadSuccessCount>0)
			{
				//�������ɹ����ģ����ٷ�һ��

				int nErrCode = 0;
				SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode, nErrCode);
				nWaitingCount = m_nRecevieTimeOut; 
				while(m_nRecvedSlave!= mmUnit.nSlaveId && nWaitingCount>0)
				{
					Sleep(1);
					nWaitingCount--;
				}

				if(nWaitingCount<=0)
				{
					CString loginfo;
					loginfo.Format(L"ERROR: modbus slave(ADDR: %d) no response!\r\n", mmUnit.nSlaveId);
					PrintLog(loginfo.GetString(),false);
					CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
					mmUnit.nReadSuccessCount = 0; //�����ζ�ʧ�ܣ��´�ֻ��һ�Ρ�
				}
			}
		}
		else
		{//ֻҪ�л�Ӧ������Ϊ�ɹ����������ݻز�������������������⣩
			if(mmUnit.nReadSuccessCount<10000)
				mmUnit.nReadSuccessCount++;

			bConnectionExist = true;
		}

		Sleep(100); 

	}

	if(bConnectionExist)//ֻҪ�����ɹ���һ�����ݣ���ô����Ϊ������������
		ClearNetworkError();
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read modbus data Un-Recved, error %d times.\r\n", m_nModbusNetworkErrorCount);
		PrintLog(loginfo.GetString(),false);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_UN_REC);
	}

	m_nSendCmdCount++;
}

void CModbusTcpCtrl::SetReadOneByOneMode( bool bReadOneByOneMode )
{
	m_bReadOneByOneMode = bReadOneByOneMode;
}

void CModbusTcpCtrl::SendOneByOne()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;

	for(int i=0; i<m_pointlist.size(); ++i)
	{
		DataPointEntry entry = m_pointlist[i];

		int nErrCode = 0;
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode(), nErrCode);
		if(!bSent)
		{
			if(nErrCode==10065)
				SetNetworkError(10);
			else
				SetNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
}

void CModbusTcpCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

void CModbusTcpCtrl::SortPointByDevice_()
{
	//�Ȱ����ͺ�λ���½�����
	m_vecPointList.clear();
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		DataPointEntry entry = m_pointlist[i];
		CreateEntryByDataTypeAndRange(entry,m_vecPointList);
	}

	//���豸�������豸��ַ������ַ��
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

void CModbusTcpCtrl::CreateEntryByDataTypeAndRange( DataPointEntry entry,vector<DataPointEntry>& vecModbus )
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

void CModbusTcpCtrl::SetReadCommands_()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;

	int i = 0;
	bool bConnectionExist = false; //�����Ƿ�����
	//set command
	for(i=0;i<m_vecPointList.size();i++)
	{
		m_vecPointList[i].SetToUpdate();
	}

	for (i = 0; i < m_ReadUnitList.size(); i++)
	{
		if(!m_bConnectOK)
			return;

		_ModbusReadUnit & mmUnit = m_ReadUnitList[i];
		if(m_nRecvedSlave != mmUnit.nSlaveId)
		{
			Sleep(m_nIDInterval);			//����ID֮���л�ʱ
		}

		if(mmUnit.bHasErrPoint == false || m_bDisSingleRead == true)
		{
			if(SendReadMutil_(mmUnit))
				bConnectionExist = true;
		}
		else
		{
			if(SendOneByOne_(mmUnit))
				bConnectionExist = true;
		}
	}

	if(bConnectionExist)//ֻҪ�����ɹ���һ�����ݣ���ô����Ϊ������������
		ClearNetworkError();
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read modbus data Un-Recved, error %d times.\r\n", m_nModbusNetworkErrorCount);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_UN_REC);
		PrintLog(loginfo.GetString(),false);
	}
}

DataPointEntry* CModbusTcpCtrl::FindEntry_( DWORD slaveid, DWORD headaddress, DWORD funccode )
{
	for (unsigned int i = 0; i < m_vecPointList.size(); i++)
	{
		const DataPointEntry& entry = m_vecPointList[i];
		if(entry.GetSlaveID() == slaveid && entry.GetHeadAddress() == headaddress)
		{
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

int CModbusTcpCtrl::GetBitFromWord( WORD dwValue, int nIndex )
{
	WORD dwTemp = dwValue<<(16-nIndex-1);
	int nTemp = dwTemp>> 15;

	return nTemp;
}

DWORD CModbusTcpCtrl::MergeTwoWORD( WORD wGao,WORD wDi,bool bOrder /*= true*/ )
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

void CModbusTcpCtrl::GetValueByDataTypeAndRange( DataPointEntry entry,vector< pair<wstring, wstring> >& valuelist )
{	
	if(entry.GetValueType() == E_MODBUS_BITE)		//BYTE
	{
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

void CModbusTcpCtrl::GetValueByDataTypeAndRange(DataPointEntry entry, E_MODBUS_VALUE_TYPE type,vector<WORD>& vecValue,vector< pair<wstring, wstring> >& valuelist )
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
	case E_MODBUS_UNSIGNED_LONG:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0],false);

			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue((double)vt.ulVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_UNSIGNED_LONG_INVERSE:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0]);

			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue((double)vt.ulVal);
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
			double dval = entry.GetMultipleReadValue(vt.fltVal);
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
			double dval = entry.GetMultipleReadValue(vt.fltVal);
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

void CModbusTcpCtrl::GetValueByDataTypeAndRange( DataPointEntry entry,double dValue,vector<WORD>& vecCmd ,bool bOrder)
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

double CModbusTcpCtrl::MergeFourDouble( WORD wGaoGao,WORD wGao,WORD wDi,WORD wDiDi,bool bOrder /*= true*/ )
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

double CModbusTcpCtrl::strtodbl( const wchar_t * str )
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

bool CModbusTcpCtrl::SendWriteCmd( DataPointEntry entry,double fValue )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);
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

bool CModbusTcpCtrl::SendWriteMutilCmd( DataPointEntry entry,double fValue )
{
	DWORD nSlaveId = entry.GetSlaveID();
	DWORD wAddress = entry.GetHeadAddress();
	DWORD wFunc = entry.GetFuncCode();
	if (nSlaveId == 0 || wAddress == 0 || wFunc == 0)
	{
		PrintLog(L"ERROR: modbus write point params invalid!\r\n",false);
		CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
		return false;
	}

	if (wFunc == 03){
		wFunc = 0x10;
	}

	vector<WORD> vecCmd;
	GetValueByDataTypeAndRange(entry,fValue,vecCmd,false);

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = entry.GetValueRange();
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��
	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;

	int nErrCode = 0;
	if(SendPackage(cSendBuffer, nCount+1, nErrCode))
	{
		//���ͳɹ�����ֵ
		for(int i=0; i<vecCmd.size(); ++i)
		{
			WORD value = vecCmd[i];
			DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
			if (entry)
			{
				entry->SetWORD(value);
			}
		}
	}
	return false;
}

void CModbusTcpCtrl::SendOneByOne_()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;
	
	SumReadAndResponse();
	for(int i=0; i<m_vecPointList.size(); ++i)
	{
		DataPointEntry entry = m_vecPointList[i];

		int nErrCode = 0;
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode(), nErrCode);
		if(!bSent)
		{
			if(nErrCode)
				SetNetworkError(10);
			else
				SetNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
}

bool CModbusTcpCtrl::SendOneByOne_( _ModbusReadUnit& unit )
{
	unit.bHasErrPoint = false;
	unit.bMultiRead = false;
	bool bConnectionExist = false;
	for(int i=unit.dwAddFrom; i<= unit.dwAddTo; ++i)
	{
		if(!m_bConnectOK)
			return false;

		int nErrCode = 0;
		bool bSent = SendReadCmd(unit.nSlaveId,i,i,unit.dwFuncCode, nErrCode);
		if(!bSent)
		{
			if(nErrCode==10065)
				SetNetworkError(10);
			else
				SetNetworkError();
		}
		else
		{
			bConnectionExist = true;
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
	return bConnectionExist;
}

bool CModbusTcpCtrl::MutilReadHasErr()
{
	if(m_mapErrPoint.size() <=0)
	{
		return false;
	}
	m_mapErrPoint.clear();
	return true;
}

void CModbusTcpCtrl::SetModbusReadUnitErrFlag( unsigned int nSlaveId,DWORD dwFuncCode,DWORD dwAdd,bool bHasErrPoint/*=true*/ )
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

//bool CModbusTcpCtrl::SendReadMutil_( _ModbusReadUnit& mmUnit )
//{
//	mmUnit.bHasErrPoint = false;
//	mmUnit.bMultiRead = true;
//	int slaveid = mmUnit.nSlaveId;
//	DWORD headaddressFrom = mmUnit.dwAddFrom;
//	DWORD headaddressTo = mmUnit.dwAddTo;
//	DWORD funccode = mmUnit.dwFuncCode;
//	if (slaveid < 0 || headaddressFrom < 0 || headaddressTo<headaddressFrom || funccode == 0 || funccode > 17)
//	{
//		// this point entry is not valid.
//		CString loginfo;
//		loginfo.Format(L"ERROR: modbus entry %s params not valid, skip!\r\n", mmUnit.strPointName.c_str());
//		PrintLog(loginfo.GetString());
//		CDebugLog::GetInstance()->SetErrPoint(mmUnit.strPointName,POINTTYPE_MODBUS_,ERROR_CUSTOM_MODBUS_INVALID);
//		return false;
//	}
//
//	bool bSent = SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode);
//	if(!bSent)
//	{
//		SetNetworkError();
//		//�ҵ����е�
//		for(int i= mmUnit.dwAddFrom; i<mmUnit.dwAddTo; ++i)
//		{
//			DataPointEntry* entry = FindEntry_(mmUnit.nSlaveId, i, mmUnit.dwFuncCode);
//			if(entry)
//			{		
//				CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_SEND_MODBUS);
//			}
//		}
//		CString loginfo;
//		loginfo.Format(L"ERROR: modbus entry %s send fail.\r\n", mmUnit.strPointName.c_str());
//		PrintLog(loginfo.GetString());
//		return false;
//	}
//
//	//�ȴ���Ӧ�Ļ��ƣ��ǰ�˫������
//	//10��
//	int nWaitingCount = m_nRecevieTimeOut; 
//	while(m_nRecvedSlave!= mmUnit.nSlaveId && nWaitingCount>0)
//	{
//		Sleep(1);
//		nWaitingCount--;
//	}
//	//�ȴ�������Ӧ�����֮ǰ�й���Ӧ����ô����һ�Σ�
//	if(nWaitingCount<=0)
//	{
//		if(mmUnit.nReadSuccessCount>0)
//		{
//			//�������ɹ����ģ����ٷ�һ��
//			SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode);
//			nWaitingCount = m_nRecevieTimeOut; 
//			while(m_nRecvedSlave!= mmUnit.nSlaveId && nWaitingCount>0)
//			{
//				Sleep(1);
//				nWaitingCount--;
//			}
//
//			if(nWaitingCount<=0)
//			{
//				CString loginfo;
//				loginfo.Format(L"ERROR: modbus slave(ADDR: %d) no response!\r\n", mmUnit.nSlaveId);
//				PrintLog(loginfo.GetString());
//				CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
//				mmUnit.nReadSuccessCount = 0; //�����ζ�ʧ�ܣ��´�ֻ��һ�Ρ�
//				mmUnit.bHasErrPoint = true;
//			}
//		}
//	}
//	else
//	{//ֻҪ�л�Ӧ������Ϊ�ɹ����������ݻز�������������������⣩
//		if(mmUnit.nReadSuccessCount<10000)
//			mmUnit.nReadSuccessCount++;
//		Sleep(m_nSendReadCmdTimeInterval); 
//		return true;
//	}
//
//	Sleep(m_nSendReadCmdTimeInterval); 
//	return false;
//}

bool CModbusTcpCtrl::SendReadMutil_( _ModbusReadUnit& mmUnit )
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
		loginfo.Format(L"ERROR: modbus entry %s params not valid, skip!\r\n", mmUnit.strPointName.c_str());
		PrintLog(loginfo.GetString(),false);
		//CDebugLog::GetInstance()->SetErrPoint(mmUnit.strPointName,POINTTYPE_MODBUS_,ERROR_CUSTOM_MODBUS_INVALID);
		return false;
	}

	int nErrCode = 0;
	bool bSent = SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode, nErrCode);
	if(!bSent)
	{
		if(nErrCode==10054)
		{
			SetNetworkError(10);
		}
		else
		{
			SetNetworkError();

		}
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
		loginfo.Format(L"ERROR: modbus entry %s send fail.\r\n", mmUnit.strPointName.c_str());
		PrintLog(loginfo.GetString(),false);
		return false;
	}
	return bSent;
}

unsigned short CModbusTcpCtrl::CalcCrcFast( unsigned char*puchMsg,unsigned short usDataLen )
{
	unsigned char uchCRCHi=0xFF ;
	unsigned char uchCRCLo=0xFF ;
	unsigned short uIndex ;

	while(usDataLen--)
	{
		uIndex=uchCRCHi^*puchMsg++;
		uchCRCHi=uchCRCLo^m_auchCRCHi[uIndex];
		uchCRCLo=m_auchCRCLo[uIndex];
	}
	return(uchCRCHi<<8|uchCRCLo);
}

bool CModbusTcpCtrl::SendWriteBYTECmd( DataPointEntry entry,double fValue )
{
	//����ԭ����
	DWORD slaveid = entry.GetSlaveID();
	DWORD headaddress = entry.GetHeadAddress();
	DWORD funccode = entry.GetFuncCode();
	if (slaveid == 0 || headaddress == 0 || funccode == 0)
	{
		PrintLog(L"ERROR: modbus write point params invalid!\r\n",false);
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

bool CModbusTcpCtrl::SendWriteWORDCmd( DataPointEntry entry,double fValue )
{
	//����ԭ����
	DWORD slaveid = entry.GetSlaveID();
	DWORD headaddress = entry.GetHeadAddress();
	DWORD funccode = entry.GetFuncCode();
	if (slaveid == 0 || headaddress == 0 || funccode == 0)
	{
		PrintLog(L"ERROR: modbus write point params invalid!\r\n",false);
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
		entry.SetWORD((WORD)(fValue));//golding 20211231
		//DataPointEntry* entry_ = FindEntry_(slaveid, headaddress, 6); //golding����ɾ��20211231
		//if (entry_)
		//{
		//	entry_->SetWORD((WORD)(fValue));
		//}
	}
	return bSent;
}

WORD CModbusTcpCtrl::SetBitToWord( WORD nData, int nIndex ,int nValue)
{
	nData &= ~(1<<nIndex); //��*num�ĵ�posλ����Ϊ0
	nData |= nValue<<nIndex;
	return nData;
}

bool CModbusTcpCtrl::SendTestWORDCmd( )
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��
	
	vector<WORD> vecCmd;
	WORD wwValue = 0x01FE;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0001;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	int nErrCode = 0;
	return SendPackage(cSendBuffer, nCount+1, nErrCode);
}

bool CModbusTcpCtrl::SendTestCloseWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��

	vector<WORD> vecCmd;
	WORD wwValue = 0x02FD;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0001;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	int nErrCode = 0;
	return SendPackage(cSendBuffer, nCount+1, nErrCode);
}

bool CModbusTcpCtrl::SendTestHornResetWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��

	vector<WORD> vecCmd;
	WORD wwValue = 0x04FB;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0001;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;

	int nErrCode = 0;
	return SendPackage(cSendBuffer, nCount+1, nErrCode);
}

bool CModbusTcpCtrl::SendTestFaultResetWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��

	vector<WORD> vecCmd;
	WORD wwValue = 0x08F7;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0001;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;

	int nErrCode = 0;
	return SendPackage(cSendBuffer, nCount+1, nErrCode);
}

bool CModbusTcpCtrl::SendTestOPenCloseWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��

	vector<WORD> vecCmd;
	WORD wwValue = 0x11EE;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0002;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;

	int nErrCode = 0;
	return SendPackage(cSendBuffer, nCount+1, nErrCode);
}

bool CModbusTcpCtrl::SendTestCloseOPenWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//�ڴ�˳��

	vector<WORD> vecCmd;
	WORD wwValue = 0x11EE;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0002;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;

	int nErrCode = 0;
	return SendPackage(cSendBuffer, nCount+1, nErrCode);
}

bool CModbusTcpCtrl::OutPutModbusReceive( const unsigned char* pRevData, DWORD length )
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
				sqlstream << oleNow.GetHour() << ":" << oleNow.GetMinute() << ":" << oleNow.GetSecond() << " [" << charToHexStr((unsigned char*)pRevData,length) << "]\n";
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

bool CModbusTcpCtrl::SetDebug( int nDebug )
{
	m_nOutPut = nDebug;
	return true;
}

string* CModbusTcpCtrl::byteToHexStr( unsigned char* byte_arr, int arr_len )
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

string CModbusTcpCtrl::charToHexStr( unsigned char* byte_arr, int arr_len )
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

int CModbusTcpCtrl::GetMultiReadCount( DWORD slaveid, DWORD headaddress, DWORD funccode )
{
	for(int i=0; i<m_ReadUnitList.size(); ++i)
	{
		if(m_ReadUnitList[i].nSlaveId == slaveid && m_ReadUnitList[i].dwFuncCode == funccode)
		{
			if(m_ReadUnitList[i].dwAddFrom <= headaddress && m_ReadUnitList[i].dwAddTo >= headaddress)
				return m_ReadUnitList[i].dwAddTo-m_ReadUnitList[i].dwAddFrom+1;
		}
	}
	return 0;
}

void CModbusTcpCtrl::SumReadAndResponse()
{
	if(m_nCmdCount > 0)			//ͳ����Ϣ
	{
		std::ostringstream sqlstream;
		m_nResponseCount = (m_nResponseCount>m_nCmdCount)?m_nUpdatePointCount:m_nResponseCount;
		sqlstream << "Read(" << m_nCmdCount << ") Response(" << m_nResponseCount << ") UpdatePoint(" << m_nUpdatePointCount << ")";
		m_strUpdateLog = sqlstream.str();

		CString strOut;
		strOut.Format(_T("INFO : Modbus TCP(%d):%s.\r\n"),m_nEngineIndex,Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str()).c_str());
		PrintLog(strOut.GetString(),false);
	}
	m_nCmdCount = 0;
	m_nResponseCount = 0;
	m_nUpdatePointCount = 0;
}

int CModbusTcpCtrl::SendReadCommandsByActive()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode())
	{
		return 0;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return 0;

	bool bConnectionExist = false; //�����Ƿ�����
	SumReadAndResponse();
	for(int i=0;i<m_vecPointList.size();i++)
	{
		m_vecPointList[i].SetToUpdate();
	}

	CString strTemp;
	int nContinousFailCount = 0;
	for (int i = 0; i < m_ReadUnitList.size(); i++)
	{
		if(!m_bConnectOK)
			return 0;

		_ModbusReadUnit & mmUnit = m_ReadUnitList[i];
		if(m_lastReadUnit.nSlaveId != mmUnit.nSlaveId)
		{
			Sleep(m_nIDInterval);			//����ID֮���л�ʱ
		}

		if(mmUnit.bHasErrPoint == false || m_bDisSingleRead == true)
		{
			strTemp.Format(_T("start SendReadMutil_(IP:%s:%d ID:%d)\r\n"), Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),m_port,m_lastReadUnit.nSlaveId);
			PrintLog(strTemp.GetString(), false);
			m_lastReadUnit = mmUnit;
			if(SendReadMutil_(mmUnit))
			{
				//ʹ�ü���ģʽ
				DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
				if(dwObject == WAIT_TIMEOUT)		// ��ʱ
				{
					CString loginfo;
					loginfo.Format(L"ERROR in SendReadMutil_: Modbus TimeOut(IP:%s:%d ID:%d Fun:%02d From:%d To:%d) Timeout:%d!\r\n", 
						Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),
						m_port,m_lastReadUnit.nSlaveId,m_lastReadUnit.dwFuncCode,m_lastReadUnit.dwAddFrom, m_lastReadUnit.dwAddTo,m_nRecevieTimeOut);
					PrintLog(loginfo.GetString(),false);
					//CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
					if(m_lastReadUnit.bHasErrPoint)
					{
						nContinousFailCount +=1;
					}

					if(nContinousFailCount>=3)
					{
						PrintLog(_T("Quit Once SendReadCommandsByActive for continuous timout failed counts>=3\r\n"), false);
						break;
					}

					m_lastReadUnit.nReadSuccessCount = 0; //�����ζ�ʧ�ܣ��´�ֻ��һ�Ρ�
					m_lastReadUnit.bHasErrPoint = true;
				}
				else if(dwObject == WAIT_OBJECT_0)
				{
					bConnectionExist = true;
					nContinousFailCount = 0;

					m_lastReadUnit.bHasErrPoint = false;
				}
			}
		}
		else
		{
			strTemp.Format(_T("start SendOneByOneByActive(IP:%s:%d ID:%d)\r\n"), Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),m_port,m_lastReadUnit.nSlaveId);
			PrintLog(strTemp.GetString(), false);
			bool bConnectionLost = false;
			if(SendOneByOneByActive(mmUnit, bConnectionLost))
				bConnectionExist = true;

			if(bConnectionLost)
			{
				//�������ӵ���ʱ���⴦��
				m_bConnectOK = false;
				return 0;
			}
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}

	if(bConnectionExist)//ֻҪ�����ɹ���һ�����ݣ���ô����Ϊ������������
		ClearNetworkError();
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read modbus data Un-Recved(IP:%s:%d).\r\n",Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),m_port);
		//CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_UN_REC);
		PrintLog(loginfo.GetString(),false);

	}

	return 1;
}

void CModbusTcpCtrl::SetSendCmdEvent()
{
	SetEvent(m_sendCmdEvent);
}

HANDLE CModbusTcpCtrl::GetSendCmdEvent()
{
	return m_sendCmdEvent;
}

void CModbusTcpCtrl::SendOneByOneByActive()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode())
	{
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;

	SumReadAndResponse();
	for(int i=0; i<m_vecPointList.size(); ++i)
	{
		DataPointEntry entry = m_vecPointList[i];
		m_vecPointList[i].SetToUpdate();

		int nErrCode = 0;
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode(), nErrCode);
		if(!bSent)
		{
			if(nErrCode==10065)
				SetNetworkError(10);
			else
				SetNetworkError();
		}
		else
		{
			//ʹ�ü���ģʽ
			DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
			if(dwObject == WAIT_TIMEOUT)		// ��ʱ
			{
				CString loginfo;
				loginfo.Format(L"ERROR in SendOneByOneByActive: Modbus TimeOut(IP:%s:%d ID:%d Fun:%02d From:%d To:%d) Timeout:%d!\r\n",
					Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),m_port,m_lastReadEntry.GetSlaveID(),
					m_lastReadEntry.GetFuncCode(),m_lastReadEntry.GetHeadAddress(),m_lastReadEntry.GetHeadAddress(), m_nRecevieTimeOut);
				PrintLog(loginfo.GetString(),false);
				//CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
				SetNetworkError();
			}
			else
			{
				ClearNetworkError();
			}
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
}

bool CModbusTcpCtrl::SendOneByOneByActive( _ModbusReadUnit& unit , bool &bConnectionStop)
{
	unit.bHasErrPoint = false;
	unit.bMultiRead = false;
	bool bConnectionExist = false;
	int nContinuousTimeoutCount = 0;
	int nContinuous10054Count = 0;
	bConnectionStop = false;
	for(int i=unit.dwAddFrom; i<= unit.dwAddTo; ++i)
	{
		if(!m_bConnectOK)
			return false;

		m_lastReadUnit = unit;
		int nErrCode = 0;
		bool bSent = SendReadCmd(unit.nSlaveId,i,i,unit.dwFuncCode, nErrCode);
		if(!bSent)
		{
			if(nErrCode==10065)
			{
				SetNetworkError(10);
				nContinuous10054Count = 0;
			}
			else if(nErrCode==10054)
			{
				SetNetworkError(10);
				nContinuous10054Count+=1;
				if(nContinuous10054Count>=3)
				{
					PrintLog(_T("Continuous 10054 error counts match and quit read\r\n"),false);
					bConnectionStop = true;
					return false;
				}
			}
			else
			{
				SetNetworkError();
				nContinuous10054Count = 0;
			}


			
		}
		else
		{
			//ʹ�ü���ģʽ
			DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
			if(dwObject == WAIT_TIMEOUT)		// ��ʱ
			{
				CString loginfo;
				loginfo.Format(L"ERROR in SendOneByOneByActive: Modbus TimeOut(IP:%s:%d ID:%d Fun:%02d From:%d To:%d) Timeout:%d!\r\n", 
					Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),m_port,m_lastReadEntry.GetSlaveID(),m_lastReadEntry.GetFuncCode(),
					m_lastReadEntry.GetHeadAddress(),m_lastReadEntry.GetHeadAddress(), m_nRecevieTimeOut);
				PrintLog(loginfo.GetString(),false);
				//CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);

				nContinuousTimeoutCount+=1;
				if(nContinuousTimeoutCount>=3)
				{
					PrintLog(_T("ERROR in SendOneByOneByActive: Continuous Timeout Count>=5, Quit.\r\n"), false);
					SetNetworkError(10);
					return false;
				}
			}
			else
			{
				nContinuousTimeoutCount = 0;
				bConnectionExist = true;
				ClearNetworkError();
			}
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
	return bConnectionExist;
}

void CModbusTcpCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}
