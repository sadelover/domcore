#include "StdAfx.h"
#include "BEOPDataLinkManager.h"
#include "OPCLink.h"
#include "ModbusLink.h"
#include "MemoryLink.h"
#include "BacnetLink.h"
#include "BacnetServerEngine.h"
#include "Protocol104Link.h"
#include "MysqlLink.h"
#include "SqlServerLink.h"
#include "SqliteLink.h"
#include "SiemensLink.h"
#include "FCBusLink.h"
#include "CO3PLink.h"
#include "WebLink.h"
#include "DiagnoseLink.h"
#include "BEOPDataPoint/DataPointManager.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "Tools/Zip/zip.h"
#include "Tools/Zip/unzip.h"
#include <iostream>
#include <sstream>
#include "../COMdll/DebugLog.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../COMdll/ModbusTcp/DogTcpCtrl.h"
#include "Tools/json/value.h"
#include "Tools/json/reader.h"
#include "RedisManager.h"
#include "../BEOPDataEngineCore/HttpOperation.h"

using Beopdatalink::CRealTimeDataAccess;
using Beopdatalink::CRealTimeDataEntry;
using Beopdatalink::CRealTimeDataStringEntry;
using namespace Json;

const int update_input_internal = 2*1000;	//2秒钟更新一次数据库。
const int update_outputparams_internal = 500;	//2秒钟刷新一下输出表。
const int update_historydata_internal = 120*1000;	//2分钟记录一次历史数据库
const int scan_warning_interval = 60;			//1分钟扫描一次报警及操作记录

const string DTU_UPDATE_ALL = "DTU_Update_ALL_Num";		//全部更新点
const string DTU_UPDATE_S5 = "DTU_Update_S5_Num";		//5s中更新点
const string DTU_UPDATE_M1 = "DTU_Update_M1_Num";		//1m中更新点
const string DTU_UPDATE_M5 = "DTU_Update_M5_Num";		//5m中更新点
const string DTU_UPDATE_H1 = "DTU_Update_H1_Num";		//1h中更新点
const string DTU_UPDATE_D1 = "DTU_Update_D1_Num";		//1d中更新点
const string DTU_UPDATE_MONTH1 = "DTU_Update_MONTH1_Num";	//1m中更新点
const string DTU_CSQ = "DTU_CSQ";	//DTU信号

const CString g_NAME_Logic = _T("domlogic.exe");
const CString g_NAME_Master = _T("domdog.exe");
const CString g_NAME_Core = _T("domcore.exe");
const CString g_NAME_Update = _T("domwatch.exe");

#define Beop_MAX_VAL 1E15
#define Beop_MIN_VAL -1e+15

CBEOPDataLinkManager::CBEOPDataLinkManager(void)
	:m_opcengine(NULL),
	m_bacnetengine(NULL),
	m_memoryengine(NULL),
	m_pDiagnoseEngine(NULL),
	m_pointmanager(NULL),
	m_dbsession_realtime(NULL),
	m_dbsession_history(NULL),
	m_dbsession_Redundancy(NULL),
	m_dbsession_log(NULL),
	m_hupdateinput(NULL),
	m_hupdateoutput(NULL),
	m_hhistorydata(NULL),
	m_hhistoryS5data(NULL),
	m_bexitthread_history(false),
	m_bexitthread_input(false),
	m_bexitthread_output(false),
	m_pdtusender(NULL),
	m_bdtuSuccess(FALSE),
	m_hsenddtuhandle(NULL),
	m_hReceivedtuhandle(NULL),
	m_hscanwarninghandle(NULL),
	m_msgwnd(NULL),
	m_bFirstInitRedundancyBuffer(false),
	m_bDTUEnabled(false),
	m_nDTUDataType(0),
	m_nDTUSendMinType(3),
	m_bVecCopying(false),
	m_bDTUChecked(false),
	m_bDTURecCmd(false),
	m_bModbusreadonebyone(false),
	m_bDisSingleRead(false),
	m_bSaveErrLog(false),
	m_nMobusInterval(100),
	m_nCO3PInterval(2000),
	m_nCO3PRecevieTimeOut(2000),
	m_nCO3PRollInterval(2),
	m_nMutilReadCount(99),
	m_nIDInterval(500),
	m_nModbusTimeOut(5000),
	m_nModbusPollInterval(2),
	m_bFirstStart(true),
	m_bDTUDisableSendAllData(false),
	m_bTCPEnable(false),
	m_strTCPServerIP(L""),
	m_nTCPServerPort(9500),
	m_strTCPName(L""),
	m_strAck(""),
	m_nStoreZip(0),
	m_nSendAll(0),
	m_strSendRealFilePath(""),
	m_nSendRealFileCount(0),
	m_strLastSendOperation(L""),
	m_nOPCClientMaxPoint(10000),
	m_bMutilOPCClientMode(false),
	m_bMutilOPCClientUseThread(false),
	m_bStoreHistory(false),
	m_nOPCCmdSleep(5),
	m_nMutilCount(20),
	m_nOPCCmdSleepFromDevice(50),
	m_nOPCPollSleep(5),
	m_nOPCPollSleepFromDevice(60),
	m_nRemoteSetOPC(0),
	m_nRemoteSetModbus(0),
	m_nRemoteSetBacnet(0),
	m_nRemoteSetSiemens(0),
	m_nRemoteSetMysql(0),
	m_nRemoteSetSqlServer(0),
	m_nRemoteSetSqlite(0),
	m_nOPCCheckReconnectInterval(5),
	m_nOPCReconnectInertval(30),
	m_nDebugOPC(0),
	m_nReconnectMinute(0),
	m_nSendRemoteSet(20),
	m_bUpdateBacnetInfo(false),
	m_nOPCMainThreadOPCPollSleep(2),
	m_bEnableSecurity(false),
	m_bDisableCheckQuality(false),
	m_nPrecision(6),
	m_bAsync20Mode(false),
	m_nRefresh20Interval(60),
	m_nLanguageID(0),
	m_nMultiAddItems(1000),
	m_nUpdateRate(500),
	m_nMaxInsertLimit(10000)
{
	 m_enginemode = BeopDataEngineMode::Mode_NULL;
	 InitializeCriticalSection(&m_RealTimeData_CriticalSection);

	 m_tLastHistorySaved = COleDateTime::GetCurrentTime();
	 m_oleCoreStart = COleDateTime::GetCurrentTime();
	 m_oleLastRemoteSetting = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0,1,0,0); 
	 m_nUpdateInputCount = 0;
	 m_nUpdateOutputCount = 0;

	 m_nMinTCycle = (POINT_STORE_CYCLE)m_nDTUSendMinType;

	 GetLocalTime(&m_tSendDTUTime);
	 GetLocalTime(&m_ackTime);
	 m_mapSendLostHistory.clear();
	 m_mapOPCEngine.clear();
	 m_RealTimeDataMap.clear();
	 m_vecModbusSer.clear();
	 m_vecCO3PSer.clear();
	 m_vecProtocol104Ser.clear();
	 m_vecMysqlSer.clear();
	 m_vecWebSer.clear();
	 m_vecSqlSer.clear();
	 m_vecSqliteSer.clear();
	 m_vecFCSer.clear();
	 m_vecOPCSer.clear();
	 m_vecOutputEntry.clear();
	 m_vecMemoryPointChangeList.clear();
	 m_RealTimeDataEntryList.clear();
	 m_RealTimeDataEntryListBufferMap_S5.clear();
	 m_RealTimeDataEntryListBufferMap_M1.clear();
	 m_RealTimeDataEntryListBufferMap_M5.clear();
	 m_RealTimeDataEntryListBufferMap_H1.clear();
	 m_RealTimeDataEntryListBufferMap_D1.clear();
	 m_RealTimeDataEntryListBufferMap_Month1.clear();
	 m_mapSendLostHistory.clear();
	 m_vecSendBuffer.clear();


	 m_bModbusEquipmentEngineEnable = false;
	 m_bThirdPartyEngineEnable = false;
	 m_bPersagyControllerEngineEnable = false;
	 m_bOPCUAEngineEnable = false;
	 m_bObixEngineEnable = false;
	 m_bCoreStationEngineEnable = false;
	 m_bKNXEngineEnable = false;

	 m_bLogixEngineEnable = false;
	 m_bMoxaTCPServerEngineEnable = false;

	 m_bABSLCEngineEnable = false;
}

CBEOPDataLinkManager::CBEOPDataLinkManager( CDataPointManager* pointmanager, Beopdatalink::CRealTimeDataAccess* pdbcon, bool bRealSite, bool bDTUEnabled,bool bDTUChecked,bool bDTURecCmd,int nDTUSendMinType,int nDTUPort,bool bModbusReadOne,int nModbusInterval,bool bDTUDisableSendAllData,bool bTCPEnable,wstring strTCPName,wstring strTCPServerIP,int nTCPServerPort,int nSendAllData,bool bRunDTUEngine)
	:m_opcengine(NULL),
	m_bacnetengine(NULL),
	m_memoryengine(NULL),
	m_pDiagnoseEngine(NULL),
	m_hupdateinput(NULL),
	m_hupdateoutput(NULL),
	m_hhistorydata(NULL),
	m_hhistoryS5data(NULL),
	m_dbsession_history(NULL),
	m_dbsession_Redundancy(NULL),
	m_bexitthread_history(false),
	m_bexitthread_input(false),
	m_bexitthread_output(false),
	m_pointmanager(pointmanager),
	m_dbsession_realtime(pdbcon),
	m_dbsession_log(NULL),
	m_msgwnd(NULL),
	m_pdtusender(NULL),
	m_bdtuSuccess(FALSE),
	m_hsenddtuhandle(NULL),
	m_hReceivedtuhandle(NULL),
	m_hscanwarninghandle(NULL),
	m_bRealSite(bRealSite),
	m_bFirstInitRedundancyBuffer(false),
	m_bDTUEnabled(bDTUEnabled),
	m_nDTUPort(nDTUPort),
	m_nDTUSendMinType(nDTUSendMinType),
	m_nDTUDataType(0),
	m_bVecCopying(false),
	m_bDTUChecked(bDTUChecked),
	m_bDTURecCmd(bDTURecCmd),
	m_bModbusreadonebyone(bModbusReadOne),
	m_bDisSingleRead(false),
	m_bSaveErrLog(false),
	m_nMobusInterval(nModbusInterval),
	m_nCO3PInterval(2000),
	m_nCO3PRecevieTimeOut(2000),
	m_nCO3PRollInterval(2),
	m_nMutilReadCount(99),
	m_nIDInterval(500),
	m_nModbusTimeOut(5000),
	m_nModbusPollInterval(2),
	m_bFirstStart(true),
	m_bDTUDisableSendAllData(bDTUDisableSendAllData),
	m_bTCPEnable(bTCPEnable),
	m_strTCPServerIP(strTCPServerIP),
	m_nTCPServerPort(nTCPServerPort),
	m_strTCPName(strTCPName),
	m_strAck(""),
	m_nStoreZip(0),
	m_nSendAll(nSendAllData),
	m_strLastSendOperation(L""),
	m_nOPCClientMaxPoint(10000),
	m_bMutilOPCClientMode(false),
	m_bMutilOPCClientUseThread(false),
	m_bStoreHistory(false),
	m_nOPCCmdSleep(5),
	m_nMutilCount(20),
	m_nOPCCmdSleepFromDevice(50),
	m_nOPCPollSleep(5),
	m_nOPCPollSleepFromDevice(60),
	m_nRemoteSetOPC(0),
	m_nRemoteSetModbus(0),
	m_nRemoteSetBacnet(0),
	m_nRemoteSetSiemens(0),
	m_nRemoteSetMysql(0),
	m_nRemoteSetSqlServer(0),
	m_nRemoteSetSqlite(0),
	m_nOPCCheckReconnectInterval(5),
	m_nOPCReconnectInertval(10),
	m_nDebugOPC(0),
	m_nReconnectMinute(0),
	m_nSendRemoteSet(20),
	m_bUpdateBacnetInfo(false),
	m_nOPCMainThreadOPCPollSleep(2),
	m_bEnableSecurity(false),
	m_bDisableCheckQuality(false),
	m_nPrecision(6),
	m_bAsync20Mode(false),
	m_nRefresh20Interval(60),
	m_nLanguageID(0),
	m_nMultiAddItems(1000),
	m_nMaxInsertLimit(10000),
	m_bRunDTUEngine(bRunDTUEngine)
{
	 m_enginemode = BeopDataEngineMode::Mode_Master;
	 m_vecModbusSer.clear();
	 m_vecCO3PSer.clear();
	 m_mapMdobusEngine.clear();
	 m_mapCO3PEngine.clear();
	 m_mapProtocol104Engine.clear();
	 m_mapFCBusEngine.clear();
	 m_mapOPCEngine.clear();

	 InitializeCriticalSection(&m_RealTimeData_CriticalSection);
	  m_oleCoreStart = COleDateTime::GetCurrentTime();
	 m_tLastHistorySaved = COleDateTime::GetCurrentTime();
	 m_nUpdateInputCount = 0;
	 m_nUpdateOutputCount = 0;

	 m_nMinTCycle = (POINT_STORE_CYCLE)m_nDTUSendMinType;
	 m_oleLastRemoteSetting = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0,1,0,0); 
	 m_senddtuevent = ::CreateEvent(NULL, false, false, L"");
	  GetLocalTime(&m_tSendDTUTime);
	  GetLocalTime(&m_ackTime);


	  m_bModbusEquipmentEngineEnable = false;
	  m_bThirdPartyEngineEnable = false;
	  m_bPersagyControllerEngineEnable = false;
	  m_bOPCUAEngineEnable = false;
	  m_bObixEngineEnable = false;
	  m_bLogixEngineEnable = false;
	  m_bABSLCEngineEnable = false;
	  m_bCoreStationEngineEnable = false;
	  m_bKNXEngineEnable = false;
	  m_bDLT645EngineEnable = false;

	  m_bMoxaTCPServerEngineEnable = false;
}

CBEOPDataLinkManager::~CBEOPDataLinkManager(void)
{
	map<wstring,CModbusEngine*>::iterator iter = m_mapMdobusEngine.begin();
	while(iter != m_mapMdobusEngine.end())
	{
		delete iter->second;
		iter->second = NULL;
		iter++;
	}

	map<wstring,CCO3PEngine*>::iterator iterCO3P = m_mapCO3PEngine.begin();
	while(iterCO3P != m_mapCO3PEngine.end())
	{
		delete iterCO3P->second;
		iterCO3P->second = NULL;
		iterCO3P++;
	}

	map<wstring,CFCBusEngine*>::iterator iterFC = m_mapFCBusEngine.begin();
	while(iterFC != m_mapFCBusEngine.end())
	{
		delete iterFC->second;
		iterFC->second = NULL;
		iterFC++;
	}

	map<wstring,CProtocol104Engine*>::iterator iter1 = m_mapProtocol104Engine.begin();
	while(iter1 != m_mapProtocol104Engine.end())
	{
		iter1->second->Exit();

		delete iter1->second;
		iter1->second = NULL;
		iter1++;
	}

	map<wstring,CMysqlEngine*>::iterator iter2 = m_mapMysqlEngine.begin();
	while(iter2 != m_mapMysqlEngine.end())
	{
		iter2->second->Exit();

		delete iter2->second;
		iter2->second = NULL;
		iter2++;
	}

	map<wstring,CWebEngine*>::iterator iterWeb = m_mapWebEngine.begin();
	while(iterWeb != m_mapWebEngine.end())
	{
		iterWeb->second->Exit();
		delete iterWeb->second;
		iterWeb->second = NULL;
		iterWeb++;
	}

	map<wstring,CSqlServerEngine*>::iterator iterSqlServer = m_mapSqlServerEngine.begin();
	while(iterSqlServer != m_mapSqlServerEngine.end())
	{
		iterSqlServer->second->Exit();
		delete iterSqlServer->second;
		iterSqlServer->second = NULL;
		iterSqlServer++;
	}

	map<int,COPCEngine*>::iterator iter3 = m_mapOPCEngine.begin();
	while(iter3 != m_mapOPCEngine.end())
	{
		iter3->second->Exit();

		delete iter3->second;
		iter3->second = NULL;
		iter3++;
	}

	map<wstring,CSqliteEngine*>::iterator iterSqlite = m_mapSqliteEngine.begin();
	while(iterSqlite != m_mapSqliteEngine.end())
	{
		iterSqlite->second->Exit();
		delete iterSqlite->second;
		iterSqlite->second = NULL;
		iterSqlite++;
	}


	if (m_opcengine){
		delete m_opcengine;
		m_opcengine = NULL;
	}

	if (m_memoryengine){
		delete m_memoryengine;
		m_memoryengine = NULL;
	}

	if(m_pDiagnoseEngine)
	{
		delete m_pDiagnoseEngine;
		m_pDiagnoseEngine = NULL;
	}

	if (m_bacnetengine){
		m_bacnetengine->ExitThread();
		delete m_bacnetengine;
		m_bacnetengine = NULL;
	}

	if (m_hupdateinput){
		::CloseHandle(m_hupdateinput);
		m_hupdateinput = NULL;
	}

	if (m_hupdateoutput){
		::CloseHandle(m_hupdateoutput);
		m_hupdateoutput = NULL;
	}

	if(m_hsenddtuhandle){
		::CloseHandle(m_hsenddtuhandle);
		m_hsenddtuhandle = NULL;
	}

	if(m_hReceivedtuhandle)
	{
		::CloseHandle(m_hReceivedtuhandle);
		m_hReceivedtuhandle = NULL;
	}

	if(m_hscanwarninghandle){
		::CloseHandle(m_hscanwarninghandle);
		m_hscanwarninghandle = NULL;
	}

	if (m_pdtusender)
	{
		m_pdtusender->ExitThreads();
		m_bdtuSuccess = false;
		delete m_pdtusender;
		m_pdtusender = NULL;
	}

	if (m_hhistorydata){
		::CloseHandle(m_hhistorydata);
		m_hhistorydata = NULL;
	}
	if (m_hhistoryS5data){
		::CloseHandle(m_hhistoryS5data);
		m_hhistoryS5data = NULL;
	}

	m_RealTimeDataEntryList.clear();
	m_RealTimeDataEntryListBufferMap_S5.clear();
	m_RealTimeDataEntryListBufferMap_M1.clear();
	m_RealTimeDataEntryListBufferMap_M5.clear();
	m_RealTimeDataEntryListBufferMap_H1.clear();
	m_RealTimeDataEntryListBufferMap_D1.clear();
	m_RealTimeDataEntryListBufferMap_Month1.clear();
	m_RealTimeDataMap.clear();
}

bool CBEOPDataLinkManager::InitOPC()
{
	vector<DataPointEntry> opcpointlist;
	m_pointmanager->GetOpcPointList(opcpointlist);
	if (!opcpointlist.empty())
	{
		SendLogMsg(L"OPC", L"INFO : Init OPC Engine...\r\n");

		wstring strOPCServerIP = m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"OPCServerIP");
		if(strOPCServerIP.length()<=7)
			strOPCServerIP = L"localhost";

		if(opcpointlist[0].GetSourceType() == L"OPC")		//OPC类型的点 若opcpointlist[0].GetOPCServerIP()不为空 需要根据ip创建连接 否则使用默认配置的IP
		{
			m_bMutilOPCClientMode = true;
			vector<vector<DataPointEntry>> vecServerList = GetOPCServer(opcpointlist,m_nOPCClientMaxPoint,strOPCServerIP);
			for(int i=0; i<vecServerList.size(); i++)
			{
				vector<DataPointEntry> opcpointlist_ = vecServerList[i];
				int nOPCIndex = i;
				if(!opcpointlist_.empty())
				{
					//////////////////////////////////////////////////////////////////////////
					wstring strOPCSIP = opcpointlist_[0].GetOPCServerIP();
					if(strOPCSIP.length() <= 0)
						strOPCSIP = strOPCServerIP;
					COPCEngine* opcengine = new COPCEngine(this,m_dbsession_history, strOPCSIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality,m_nPrecision,m_bAsync20Mode,m_nRefresh20Interval,m_nLanguageID,m_nMultiAddItems,m_nDebugOPC,m_nUpdateRate);
					opcengine->SetOpcPointList(opcpointlist_);
					opcengine->SetLogSession(m_dbsession_log);
					opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
					m_mapOPCEngine[nOPCIndex] = opcengine;
					for(int j=0; j<opcpointlist_.size(); ++j)
					{
						m_pointmanager->SetOPClientIndex(opcpointlist_[j].GetShortName(),nOPCIndex);
					}
					if (!InitOPC(opcengine,nOPCIndex))
					{
						CString strErr;
						strErr.Format(_T("ERROR: OPC Engine %d(Mutil:%s-%s) Start Failed!\r\n"),nOPCIndex,strOPCSIP.c_str(),opcpointlist_[0].GetParam(3).c_str());
						SendLogMsg(L"OPC",strErr);
					}
					//////////////////////////////////////////////////////////////////////////
				}
			}
		}
		else
		{
			//增加大批量OPC时分连接读取
			int nSize = opcpointlist.size();
			if(nSize <= m_nOPCClientMaxPoint)
			{
				m_opcengine = new COPCEngine(this,m_dbsession_history, strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality,m_nPrecision,m_bAsync20Mode,m_nRefresh20Interval,m_nLanguageID,m_nMultiAddItems,m_nDebugOPC,m_nUpdateRate);
				m_opcengine->SetOpcPointList(opcpointlist);
				m_opcengine->SetLogSession(m_dbsession_log);
				m_opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
				if (!InitOPC(m_opcengine,0))
				{
					SendLogMsg(L"OPC",L"ERROR: OPC Engine Start Failed!\r\n");
					return false;
				}

				SendLogMsg(L"OPC",L"INFO : OPC Engine Connected Success.\r\n");
				return true;
			}
			else
			{
				m_bMutilOPCClientMode = true;
				int nOPCIndex = 0;
				vector<DataPointEntry> opcpointlist_;		//单个引擎
				for(int i=0; i< opcpointlist.size(); ++i)
				{
					if(i%m_nOPCClientMaxPoint == 0 && i != 0)
					{
						COPCEngine* opcengine = new COPCEngine(this,m_dbsession_history, strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality,m_nPrecision,m_bAsync20Mode,m_nRefresh20Interval,m_nLanguageID,m_nMultiAddItems,m_nDebugOPC,m_nUpdateRate);
						opcengine->SetOpcPointList(opcpointlist_);
						opcengine->SetLogSession(m_dbsession_log);
						opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
						m_mapOPCEngine[nOPCIndex] = opcengine;
						if (!InitOPC(opcengine,nOPCIndex))
						{
							CString strErr;
							strErr.Format(_T("ERROR: OPC Engine %d(Mutil) Start Failed!\r\n"),nOPCIndex);
							SendLogMsg(L"OPC",strErr);
							return false;
						}
						nOPCIndex++;
						opcpointlist_.clear();
					}

					m_pointmanager->SetOPClientIndex(opcpointlist[i].GetShortName(),nOPCIndex);
					opcpointlist_.push_back(opcpointlist[i]);
				}

				if(opcpointlist_.size() > 0)
				{
					COPCEngine* opcengine = new COPCEngine(this, m_dbsession_history,strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality,m_nPrecision,m_bAsync20Mode,m_nRefresh20Interval,m_nLanguageID,m_nMultiAddItems,m_nDebugOPC,m_nUpdateRate);
					opcengine->SetOpcPointList(opcpointlist_);
					opcengine->SetLogSession(m_dbsession_log);
					opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
					m_mapOPCEngine[nOPCIndex] = opcengine;
					if (!InitOPC(opcengine,nOPCIndex))
					{
						CString strErr;
						strErr.Format(_T("ERROR: OPC Engine %d(Mutil) Start Failed!\r\n"),nOPCIndex);
						SendLogMsg(L"OPC",strErr);
						return false;
					}
				}
			}
			SendLogMsg(L"OPC",L"INFO : OPC Engine(Mutil) Connected Success.\r\n");
			return true;
		}
	}
	return false;
}

bool CBEOPDataLinkManager::Init()
{
	ASSERT(m_pointmanager != NULL);
	if (!m_pointmanager){
		return false;
	}

	SendLogMsg(L"", L"INFO : Init Realtime DB Input Datas...\r\n");
	if(!InitInputTable())
	{
		SendLogMsg(L"", L"ERROR: Init Realtime DB Input Datas Failed, Please Check Buffer Size of MySQL.");
		return false;
	}

	if(!m_Redis.is_valid())
	{
		m_Redis.init(L"127.0.0.1", 6379);
	}
	
	//Init Param
	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		wstring wstrOPCClientMaxPoint,wstrOPCClientThread,wstrStoreHistory,wstrOPCSleep,wstrOPCMutilCount,wstrMutilReadCount,wstrAsync20Mode,wstrRefresh20Interval,wstrUpdateRate;
		wstrOPCClientMaxPoint = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcclientmaxpoint",L"10000");
		if(wstrOPCClientMaxPoint == L"" || wstrOPCClientMaxPoint == L"0")
			wstrOPCClientMaxPoint = L"10000";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcclientmaxpoint",wstrOPCClientMaxPoint);
		m_nOPCClientMaxPoint = _wtoi(wstrOPCClientMaxPoint.c_str());

		//OPC Thread
		wstrOPCClientThread = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcclientthread",L"0");
		if(wstrOPCClientThread == L"")
			wstrOPCClientThread = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcclientthread",wstrOPCClientThread);
		m_bMutilOPCClientUseThread = _wtoi(wstrOPCClientThread.c_str());

		//
		wstrOPCMutilCount = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcmutilcount",L"1");
		if(wstrOPCMutilCount == L"")
			wstrOPCMutilCount = L"1";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcmutilcount",wstrOPCMutilCount);
		m_nMutilCount = _wtoi(wstrOPCMutilCount.c_str());

		//
		wstrOPCSleep = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opccmdsleep",L"500");
		if(wstrOPCSleep == L"")
			wstrOPCSleep = L"500";
		m_dbsession_history->WriteCoreDebugItemValue(L"opccmdsleep",wstrOPCSleep);
		m_nOPCCmdSleep = _wtoi(wstrOPCSleep.c_str());

		wstrUpdateRate = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcupdaterate",L"500");
		if(wstrUpdateRate == L"")
			wstrUpdateRate = L"500";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcupdaterate",wstrUpdateRate);
		m_nUpdateRate = _wtoi(wstrUpdateRate.c_str());

		wstrAsync20Mode = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcasync20mode",L"0");
		if(wstrAsync20Mode == L"")
			wstrAsync20Mode = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcasync20mode",wstrAsync20Mode);
		m_bAsync20Mode = _wtoi(wstrAsync20Mode.c_str());

		wstrRefresh20Interval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcasync20interval",L"60");
		if(wstrRefresh20Interval == L"")
			wstrRefresh20Interval = L"60";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcasync20interval",wstrRefresh20Interval);
		m_nRefresh20Interval = _wtoi(wstrRefresh20Interval.c_str());

		wstring wstrLanguageID,wstrMultiAddItems;
		wstrLanguageID = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opclanguageid",L"0");
		if(wstrLanguageID == L"")
			wstrLanguageID = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"opclanguageid",wstrLanguageID);
		m_nLanguageID = _wtoi(wstrLanguageID.c_str());

		wstrMultiAddItems = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcmultiadd",L"1000");
		if(wstrMultiAddItems == L"")
			wstrMultiAddItems = L"1000";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcmultiadd",wstrMultiAddItems);
		m_nMultiAddItems = _wtoi(wstrMultiAddItems.c_str());

		//////////////////////////////////////////////////////////////////////////
		wstring wstrOPCCmdSleepFromDevice,wstrOPCPollSleep,wstrOPCPollSleepFromDevice,wstrReconnectMinute,wstrIDInterval,wstrEnableSecurity,wstrOPCMainThreadOPCPollSleep,wstrDisableCheckQuality,wstrModbusTimeout;
		wstrOPCCmdSleepFromDevice = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opccmdsleepfromdevice",L"500");
		if(wstrOPCCmdSleepFromDevice == L"")
			wstrOPCCmdSleepFromDevice = L"500";
		m_dbsession_history->WriteCoreDebugItemValue(L"opccmdsleepfromdevice",wstrOPCCmdSleepFromDevice);
		m_nOPCCmdSleepFromDevice = _wtoi(wstrOPCCmdSleepFromDevice.c_str());

		wstrOPCPollSleep = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcpollsleep",L"60");
		if(wstrOPCPollSleep == L"")
			wstrOPCPollSleep = L"60";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcpollsleep",wstrOPCPollSleep);
		m_nOPCPollSleep = _wtoi(wstrOPCPollSleep.c_str());

		wstrOPCPollSleepFromDevice = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcpollsleepfromdevice",L"60");
		if(wstrOPCPollSleepFromDevice == L"")
			wstrOPCPollSleepFromDevice = L"60";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcpollsleepfromdevice",wstrOPCPollSleepFromDevice);
		m_nOPCPollSleepFromDevice = _wtoi(wstrOPCPollSleepFromDevice.c_str());

		wstrReconnectMinute = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcreconnectignore",L"0");
		if(wstrReconnectMinute == L"")
			wstrReconnectMinute = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcreconnectignore",wstrReconnectMinute);
		m_nReconnectMinute = _wtoi(wstrReconnectMinute.c_str());

		wstrEnableSecurity = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"enableSecurity",L"0");
		if(wstrEnableSecurity == L"")
			wstrEnableSecurity = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"enableSecurity",wstrEnableSecurity);
		m_bEnableSecurity = _wtoi(wstrEnableSecurity.c_str());

		wstrDisableCheckQuality = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"disableCheckQuality",L"0");
		if(wstrDisableCheckQuality == L"")
			wstrDisableCheckQuality = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"disableCheckQuality",wstrDisableCheckQuality);
		m_bDisableCheckQuality = _wtoi(wstrDisableCheckQuality.c_str());

		wstrOPCMainThreadOPCPollSleep = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcmainpollsleep",L"10");
		if(wstrOPCMainThreadOPCPollSleep == L"")
			wstrOPCMainThreadOPCPollSleep = L"10";
		m_dbsession_history->WriteCoreDebugItemValue(L"opcmainpollsleep",wstrOPCMainThreadOPCPollSleep);
		m_nOPCMainThreadOPCPollSleep = _wtoi(wstrOPCMainThreadOPCPollSleep.c_str());

		//////////////////////////////////////////////////////////////////////////

		//是否存储历史数据
		//wstrStoreHistory = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"storehistory",L"0");
		//if(wstrStoreHistory == L"")
		//	wstrStoreHistory = L"0";
		//m_dbsession_history->WriteCoreDebugItemValue(L"storehistory",wstrStoreHistory);
		//m_bStoreHistory = _wtoi(wstrStoreHistory.c_str());
		m_bStoreHistory = true;

		//数据精度
		wstring wstrPrecision = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"precision",L"2");
		if(wstrPrecision == L"")
			wstrPrecision = L"2";
		m_dbsession_history->WriteCoreDebugItemValue(L"precision",wstrPrecision);
		m_nPrecision = _wtoi(wstrPrecision.c_str());

		wstring wstrMaxInsertLimit = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"maxinsertlimit",L"10000");
		if(wstrMaxInsertLimit == L"")
			wstrMaxInsertLimit = L"10000";
		m_dbsession_history->WriteCoreDebugItemValue(L"maxinsertlimit",wstrMaxInsertLimit);
		m_nMaxInsertLimit = _wtoi(wstrMaxInsertLimit.c_str());

		//是否开放远程设置硬点
		wstring wstrRemoteSetOPC,wstrRemoteSetModbus,wstrRemoteSetBacnet,wstrRemoteSetSiemens,wstrRemoteSetMysql,wstrRemoteSetSqlite,wstrRemoteSetSqlServer;
		wstrRemoteSetOPC = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"remotesetopc",L"0");
		if(wstrRemoteSetOPC == L"")
			wstrRemoteSetOPC = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"remotesetopc",wstrRemoteSetOPC);
		m_nRemoteSetOPC = _wtoi(wstrRemoteSetOPC.c_str());

		wstrRemoteSetModbus = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"remotesetmodbus",L"0");
		if(wstrRemoteSetModbus == L"")
			wstrRemoteSetModbus = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"remotesetmodbus",wstrRemoteSetModbus);
		m_nRemoteSetModbus = _wtoi(wstrRemoteSetModbus.c_str());

		wstrRemoteSetBacnet = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"remotesetbacnet",L"0");
		if(wstrRemoteSetBacnet == L"")
			wstrRemoteSetBacnet = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"remotesetbacnet",wstrRemoteSetBacnet);
		m_nRemoteSetBacnet = _wtoi(wstrRemoteSetBacnet.c_str());

		wstrRemoteSetSiemens = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"remotesetsimens",L"0");
		if(wstrRemoteSetSiemens == L"")
			wstrRemoteSetSiemens = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"remotesetsimens",wstrRemoteSetSiemens);
		m_nRemoteSetSiemens = _wtoi(wstrRemoteSetSiemens.c_str());

		wstrRemoteSetMysql = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"remotesetmysql",L"0");
		if(wstrRemoteSetMysql == L"")
			wstrRemoteSetMysql = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"remotesetmysql",wstrRemoteSetMysql);
		m_nRemoteSetMysql = _wtoi(wstrRemoteSetMysql.c_str());

		wstrRemoteSetSqlServer = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"remotesetsqlserver",L"0");
		if(wstrRemoteSetSqlServer == L"")
			wstrRemoteSetSqlServer = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"remotesetsqlserver",wstrRemoteSetSqlServer);
		m_nRemoteSetSqlServer = _wtoi(wstrRemoteSetSqlServer.c_str());

		wstrRemoteSetSqlite = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"remotesetsqlite",L"0");
		if(wstrRemoteSetSqlite == L"")
			wstrRemoteSetSqlite = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"remotesetsqlite",wstrRemoteSetSqlite);
		m_nRemoteSetSqlite = _wtoi(wstrRemoteSetSqlite.c_str());

		//
		wstring strDebugOPC = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debugopc",L"0");
		if(strDebugOPC==L"")
			strDebugOPC = L"0";
		m_nDebugOPC = _wtoi(strDebugOPC.c_str());

		wstring strOPCReconnectInterval= m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcreconnect",L"30");
		if(strOPCReconnectInterval==L"")
			strOPCReconnectInterval = L"30";
		m_nOPCReconnectInertval = _wtoi(strOPCReconnectInterval.c_str());

		wstring strOPCCheckReconnect= m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opccheckreconnect",L"5");
		if(strOPCCheckReconnect==L"")
			strOPCCheckReconnect = L"5";
		m_nOPCCheckReconnectInterval = _wtoi(strOPCCheckReconnect.c_str());

		//modbus
		wstrMutilReadCount = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"modbusmutilcount",L"99");
		if(wstrMutilReadCount == L"")
			wstrMutilReadCount = L"99";
		m_dbsession_history->WriteCoreDebugItemValue(L"modbusmutilcount",wstrMutilReadCount);
		m_nMutilReadCount = _wtoi(wstrMutilReadCount.c_str());

		wstrIDInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"modbusidinterval",L"500");
		if(wstrIDInterval == L"")
			wstrIDInterval = L"500";
		m_dbsession_history->WriteCoreDebugItemValue(L"modbusidinterval",wstrIDInterval);
		m_nIDInterval = _wtoi(wstrIDInterval.c_str());

		wstrModbusTimeout = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"modbustimeout",L"5000");
		if(wstrModbusTimeout == L"")
			wstrModbusTimeout = L"5000";
		m_dbsession_history->WriteCoreDebugItemValue(L"modbustimeout",wstrModbusTimeout);
		m_nModbusTimeOut = _wtoi(wstrModbusTimeout.c_str());

		wstring wstrModbusPollInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"modbuspollinterval",L"5");
		if(wstrModbusPollInterval == L"")
			wstrModbusPollInterval = L"5";
		m_dbsession_history->WriteCoreDebugItemValue(L"modbuspollinterval",wstrModbusPollInterval);
		m_nModbusPollInterval = _wtoi(wstrModbusPollInterval.c_str());

		wstring wstrSendRemoteSet = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"remotesetinterval",L"20");
		if(wstrSendRemoteSet == L"")
			wstrSendRemoteSet = L"20";
		m_dbsession_history->WriteCoreDebugItemValue(L"remotesetinterval",wstrSendRemoteSet);
		m_nSendRemoteSet = _wtoi(wstrSendRemoteSet.c_str());

		wstring wstrDisSingleRead = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"modbusdissingleread",L"0");
		if(wstrDisSingleRead == L"")
			wstrDisSingleRead = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"modbusdissingleread",wstrDisSingleRead);
		m_bDisSingleRead = _wtoi(wstrDisSingleRead.c_str());
		
		wstring wstrSaveErrLog = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"modbussavelog",L"0");
		if(wstrSaveErrLog == L"")
			wstrSaveErrLog = L"0";
		m_dbsession_history->WriteCoreDebugItemValue(L"modbussavelog",wstrSaveErrLog);
		m_bSaveErrLog = _wtoi(wstrSaveErrLog.c_str());

		//co3p
		wstring wstrCO3PInterval,wstrCO3PRollInterval,wstrCO3PRecevieTimeOut;
		wstrCO3PInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"co3pcmdinterval",L"500");
		if(wstrCO3PInterval == L"")
			wstrCO3PInterval = L"500";
		m_dbsession_history->WriteCoreDebugItemValue(L"co3pcmdinterval",wstrCO3PInterval);
		m_nCO3PInterval = _wtoi(wstrCO3PInterval.c_str());

		wstrCO3PRecevieTimeOut = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"co3ptimeout",L"5000");
		if(wstrCO3PRecevieTimeOut == L"")
			wstrCO3PRecevieTimeOut = L"5000";
		m_dbsession_history->WriteCoreDebugItemValue(L"co3ptimeout",wstrCO3PRecevieTimeOut);
		m_nCO3PRecevieTimeOut = _wtoi(wstrCO3PRecevieTimeOut.c_str());

		wstrCO3PRollInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"co3prollinterval",L"60");
		if(wstrCO3PRollInterval == L"")
			wstrCO3PRollInterval = L"60";
		m_dbsession_history->WriteCoreDebugItemValue(L"co3prollinterval",wstrCO3PRollInterval);
		m_nCO3PRollInterval = _wtoi(wstrCO3PRollInterval.c_str());

		//读取deeplogic_redis定义是否启用
		wstring wstrDeepRedisAddress = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"deep_logic_redis_address",L"");//形如:192.168.1.10,6379,rpwd
		if(wstrDeepRedisAddress.length()>0)
		{
			vector<wstring> wstrList;
			Project::Tools::SplitStringByChar(wstrDeepRedisAddress.c_str(), ',', wstrList);
			if(wstrList.size()==1)
			{
				wstrList.push_back(L"6379");
			}
			if(wstrList.size()>=2)
			{
				m_Redis_To_DeepLogic.init(wstrList[0], _wtoi(wstrList[1].c_str()));

				if(wstrList.size()>=3)
				{
					Project::Tools::WideCharToUtf8(wstrList[2], m_Redis_To_DeepLogic.m_strPassword);
				}
			}
		}
	}

	///初始化Modbus引擎///////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> modbuspointlist;
	m_pointmanager->GetModbusPointList(modbuspointlist);
	if(modbuspointlist.size() > 0)
	{
		SendLogMsg(L"", L"INFO : Init Modbus...\r\n");
		vector<DataPointEntry> modbuspointlist_valid;
		for (unsigned int i = 0; i < modbuspointlist.size(); i++)
		{
			const DataPointEntry& entry = modbuspointlist[i];
			if (entry.GetParam(DataPointEntry::INDEX_SLAVEID).empty()
				|| entry.GetParam(DataPointEntry::INDEX_FUNCODE).empty()
				|| entry.GetParam(DataPointEntry::INDEX_HEADADDRESS).empty()
				|| entry.GetParam(DataPointEntry::INDEX_SERVERIP).empty())
			{
				CString Errorinfo;
				Errorinfo.Format(_T("ERROR: Modbus Point : %s Invalid\r\n") , entry.GetShortName());
				SendLogMsg(L"Modbus", Errorinfo.GetString());
				continue;
			}else
				modbuspointlist_valid.push_back(entry);
		}
		InitModbusEngine(modbuspointlist_valid, m_nMobusInterval,m_nMutilReadCount,m_nIDInterval,m_nModbusTimeOut,m_nModbusPollInterval,m_nPrecision,m_bDisSingleRead,m_bSaveErrLog);//
	}
	
	///初始化FCBus引擎///////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> fcbuspointlist;
	m_pointmanager->GetFCBusPointList(fcbuspointlist);
	if(fcbuspointlist.size() >0)
	{
		SendLogMsg(L"", L"INFO : Init FCbus...\r\n");
		InitFCbusEngine(fcbuspointlist);//
	}

	///初始化Bacnet引擎///////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> bacnetpointlist;
	m_pointmanager->GetBacnetPointList(bacnetpointlist);
	if(bacnetpointlist.size() > 0)
	{
		SendLogMsg(L"", L"INFO : Init Bacnet Engine...\r\n");
		if(!InitBacnetEngine(bacnetpointlist))
		{
			SendLogMsg(L"", L"ERROR: Init Bacnet Engine Failed...\r\n");
		}
	}

	//初始化104规约引擎///////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> protocol104entrylist;
	m_pointmanager->GetProtocol104PointList(protocol104entrylist);
	if(protocol104entrylist.size() > 0)
	{
		SendLogMsg(L"", L"INFO : Init Protocol-104 Engine...\r\n");
		InitProtocol104Engine(protocol104entrylist);
	}

	////初始化WebEngine引擎//////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> webentrylist;
	m_pointmanager->GetWebPointList(webentrylist);
	if(webentrylist.size() > 0)
	{
		SendLogMsg(L"", L"INFO : Init Web Engine...\r\n");
		InitWebEngine(webentrylist);
	}

	///初始化Mysql引擎///////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> mysqlentrylist;
	m_pointmanager->GetMysqlPointList(mysqlentrylist);
	if(mysqlentrylist.size() > 0)
	{
		SendLogMsg(L"", L"INFO : Init MySQL-Data Engine...\r\n");
		InitMysqlEngine(mysqlentrylist);
	}

	////初始化SqlServerEngine引擎//////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> sqlentrylist;
	m_pointmanager->GetSqlServerPointList(sqlentrylist);
	if(sqlentrylist.size() > 0)
	{
		SendLogMsg(L"", L"INFO : Init SqlServer-Data Engine...\r\n");
		InitSqlServerEngine(sqlentrylist);
	}

	///初始化Sqlite引擎///////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> sqliteentrylist;
	m_pointmanager->GetSqlitePointList(sqliteentrylist);
	if(sqliteentrylist.size() > 0)
	{
		SendLogMsg(L"", L"INFO : Init Sqlite-Data Engine...\r\n");
		InitSqliteEngine(sqliteentrylist);
	}

	///初始化CO3P引擎///////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> co3ppointlist;
	m_pointmanager->GetCO3PPointList(co3ppointlist);
	if(co3ppointlist.size() > 0)
	{		
		SendLogMsg(L"", L"INFO : Init CO3P Engine...\r\n");
		InitCO3PEngine(co3ppointlist, m_nCO3PInterval,m_nCO3PRollInterval,m_nCO3PRecevieTimeOut,m_nPrecision);
	}

	///初始化软点引擎///////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> memoryentrylist;
	m_pointmanager->GetMemoryPointList(memoryentrylist);
	if(memoryentrylist.size() > 0)
	{
		SendLogMsg(L"", L"INFO : Init Memory Engine...\r\n");
		if (!memoryentrylist.empty()){
			m_memoryengine = new CMemoryLink(this);
			m_memoryengine->SetPointList(memoryentrylist);
			m_memoryengine->Init();
		}
	}
	
	////初始化S7西门子PLC引擎//////////////////////////////////////////////////////////////////////
	vector<DataPointEntry> siemenslist;
	m_pointmanager->GetSiemensPointList(siemenslist);
	if(siemenslist.size() > 0)
	{
		vector<_SiemensPLCProp> sCPUPropList;
		for(int i=0;i<siemenslist.size();i++)
		{
			_SiemensPLCProp pp;
			pp.strIP = siemenslist[i].GetParam(3);
			pp.nSlack = _wtoi(siemenslist[i].GetParam(4).c_str());
			pp.nSlot = _wtoi(siemenslist[i].GetParam(5).c_str());
			if(!FindS7CPUInfo(sCPUPropList,pp))
			{
				sCPUPropList.push_back(pp);
			}

			//support configure warnings
			siemenslist[i].SetParam(1, AutoSupportSimense1200Address(siemenslist[i].GetParam(1)));
		}

	}

	//初始化modbusEquipment
	vector<DataPointEntry> modbusEquipmentPointList;
	m_pointmanager->GetModbusEquipmentPointList(modbusEquipmentPointList);
	if(modbusEquipmentPointList.size() > 0)
	{
		m_bModbusEquipmentEngineEnable = true;
	}
	else
	{
		m_bModbusEquipmentEngineEnable = false;
	}


	//初始化CoreStation点
	vector<DataPointEntry> coreStationPointList;
	m_pointmanager->GetCoreStationPointList(coreStationPointList);
	if(coreStationPointList.size() > 0)
	{
		m_bCoreStationEngineEnable = true;
	}
	else
	{
		m_bCoreStationEngineEnable = false;
	}

	//初始化KNX点
	vector<DataPointEntry> knxPointList;
	m_pointmanager->GetKNXPointList(knxPointList);
	if(knxPointList.size() > 0)
	{
		m_bKNXEngineEnable = true;
	}
	else
	{
		m_bKNXEngineEnable = false;
	}


	//初始化DLT645点
	vector<DataPointEntry> dlt645PointList;
	m_pointmanager->GetDLT645PointList(dlt645PointList);
	if(dlt645PointList.size() > 0)
	{
		m_bDLT645EngineEnable = true;
	}
	else
	{
		m_bDLT645EngineEnable = false;
	}

	//初始化thirdparty
	vector<DataPointEntry> thirdPartyPointList;
	m_pointmanager->GetThirdPartyPointList(thirdPartyPointList);
	if(thirdPartyPointList.size() > 0)
	{
		m_bThirdPartyEngineEnable = true;
	}
	else
	{
		m_bThirdPartyEngineEnable = false;
	}

	//初始化persagy controller engine
	vector<DataPointEntry> persagyPointList;
	m_pointmanager->GetPersagyControllerPointList(persagyPointList);
	if(persagyPointList.size() > 0)
	{
		m_bPersagyControllerEngineEnable = true;
	}
	else
	{
		m_bPersagyControllerEngineEnable = false;
	}



	//初始化obix engine
	vector<DataPointEntry> obixPointList;
	m_pointmanager->GetObixPointList(obixPointList);
	if(obixPointList.size() > 0)
	{
		m_bObixEngineEnable = true;
	}
	else
	{
		m_bObixEngineEnable = false;
	}

	//初始化logix engine
	vector<DataPointEntry> logixPointList;
	m_pointmanager->GetLogixPointList(logixPointList);
	if(logixPointList.size() > 0)
	{
		m_bLogixEngineEnable = true;
	}
	else
	{
		m_bLogixEngineEnable = false;
	}


	//初始化ABSLC engine
	vector<DataPointEntry> abslcPointList;
	m_pointmanager->GetABSLCPointList(abslcPointList);
	if(abslcPointList.size() > 0)
	{
		m_bABSLCEngineEnable = true;
	}
	else
	{
		m_bABSLCEngineEnable = false;
	}

	//初始化moxa engine
	vector<DataPointEntry> moxaPointList;
	m_pointmanager->GetMoxaTCPServerPointList(moxaPointList);
	if(moxaPointList.size() > 0)
	{
		m_bMoxaTCPServerEngineEnable = true;
	}
	else
	{
		m_bMoxaTCPServerEngineEnable = false;
	}

	//初始化opc ua engine
	vector<DataPointEntry> opcuaPointList;
	m_pointmanager->GetOPCUAControllerPointList(opcuaPointList);
	if(opcuaPointList.size() > 0)
	{
		m_bOPCUAEngineEnable = true;
	}
	else
	{
		m_bOPCUAEngineEnable = false;
	}

	//初始化bacnetpy
	vector<DataPointEntry> bacnetpyPointList;
	m_pointmanager->GetBacnetpyPointList(bacnetpyPointList);
	if(bacnetpyPointList.size() > 0)
	{
		m_bBacnetPyEngineEnable = true;
	}
	else
	{
		m_bBacnetPyEngineEnable = false;
	}

	////初始化诊断引擎//////////////////////////////////////////////////////////////////////
	SendLogMsg(L"", L"INFO : Init Diagnose Engine...\r\n");
	m_pDiagnoseEngine = new CDiagnoseLink(this,this->m_dbsession_history);
	m_pDiagnoseEngine->Init();
	
	if(m_bRealSite)					//仿真模式下不init dtu
	{
		if(m_bRunDTUEngine)
		{
			//扫描生成报警事件
			if(m_hscanwarninghandle == NULL)
				m_hscanwarninghandle = (HANDLE)_beginthreadex(NULL, 0, ScanWarningAndOperation, this, 0, NULL);
		}
		else
		{
			InitDTUEngine(m_bDTUEnabled,m_bTCPEnable);
		}
	}
	InitPointsValueFromDB();

	m_hhistorydata = (HANDLE)_beginthreadex(NULL, 0, ThreadFuncHistoryData, this, NORMAL_PRIORITY_CLASS,NULL);
	m_hhistoryS5data = (HANDLE)_beginthreadex(NULL, 0, ThreadFuncHistoryData_S5, this, NORMAL_PRIORITY_CLASS,NULL);
	
	return true;
}

/************************************************************************/
/* 
自动将点名里的大小写兼容
PLC引擎代码里需要的是大写的，所以最终读取时都需要大写
*/
/************************************************************************/
wstring CBEOPDataLinkManager::AutoSupportSimense1200Address(wstring strAddress)
{
	CString strTemp = strAddress.c_str();
	if(strTemp.Find(L",REAL")>=0 && strTemp.Find(L"DB")>=0)
		return strAddress;

	if(strTemp.Find(L",INT")>=0 && strTemp.Find(L"DB")>=0)
		return strAddress;

	if(strTemp.Find(L",X")>=0  && strTemp.Find(L"DB")>=0)
		return strAddress;

	strTemp.MakeLower();
	int nFind = strTemp.Find(_T(",real"));
	if(nFind >=0)
	{
		if(strTemp.Find(_T("db"))>=0)
		{
			strTemp.Replace(_T("db"), _T("DB"));
			strTemp.Replace(_T(",real"), _T(",REAL"));
			wstring wstrNew = strTemp.GetString();
			return wstrNew;
		}
	}

	nFind = strTemp.Find(_T(",int"));
	if(nFind >=0)
	{
		if(strTemp.Find(_T("db"))>=0)
		{
			strTemp.Replace(_T("db"), _T("DB"));
			strTemp.Replace(_T(",int"), _T(",INT"));
			wstring wstrNew = strTemp.GetString();
			return wstrNew;
		}
	}

	//
	nFind = strTemp.Find(_T(",x"));
	if(nFind >=0)
	{
		if(strTemp.Find(_T("db"))>=0)
		{
			strTemp.Replace(_T("db"), _T("DB"));
			strTemp.Replace(_T(",x"), _T(",X"));
			wstring wstrNew = strTemp.GetString();
			return wstrNew;
		}
	}

	return strAddress;
}

bool CBEOPDataLinkManager::InitBacnetEngine( vector<DataPointEntry>& bacnetpoint )
{
	if (!bacnetpoint.empty())
	{
		m_bacnetengine = new CBacnetEngine(this,this->m_dbsession_history);
		m_bacnetengine->SetPointList(bacnetpoint);
		m_bacnetengine->SetLogSession(m_dbsession_log);
		return m_bacnetengine->Init();
	}
	return true;
}

bool CBEOPDataLinkManager::FindS7CPUInfo(const vector<_SiemensPLCProp> & ppList, const _SiemensPLCProp &pp)
{
	for(int i=0;i<ppList.size();i++)
	{
		if(ppList[i].strIP == pp.strIP &&  ppList[i].nSlack==pp.nSlack && ppList[i].nSlot==pp.nSlot)
			return true;
	}

	return false;
}

CBacnetEngine * CBEOPDataLinkManager::GetBacnetEngine()
{
	return m_bacnetengine;
}

CBacnetServerEngine * CBEOPDataLinkManager::GetBacnetServerEngine()
{
	return m_bacnetServerEngine;
}

CMemoryLink *  CBEOPDataLinkManager::GetMemoryEngine()
{
	return m_memoryengine;
}

bool CBEOPDataLinkManager::InitModbusEngine( vector<DataPointEntry> modbuspoint , int nReadCmdMs,int nMutilReadCount,int nIDInterval,int nTimeOut,int nPollInterval,int nPrecision,bool bDisSingleRead,bool bSaveErrLog)
{
	if(modbuspoint.empty())
		return false;

	vector<vector<DataPointEntry>> vecServerList = GetModbusServer(modbuspoint);

	for(int i=0; i<vecServerList.size(); i++)
	{
		if(!vecServerList[i].empty())
		{
			CModbusEngine* pModbusengine = new CModbusEngine(this);
			wstring strSer = vecServerList[i][0].GetServerAddress();
			pModbusengine->SetPointList(vecServerList[i]);
			pModbusengine->SetLogSession(m_dbsession_log);
			pModbusengine->SetSendCmdTimeInterval(nReadCmdMs,nMutilReadCount,nIDInterval,nTimeOut,nPollInterval,nPrecision,bDisSingleRead,bSaveErrLog);
			pModbusengine->SetReadOneByOneMode(m_bModbusreadonebyone);
			pModbusengine->SetIndex(i);
			if(!pModbusengine->Init())
			{
				CString strFailedInfo;
				strFailedInfo.Format(L"ERROR: Init Modbus Engine %d(%s) Failed.\r\n", i,strSer.c_str());
				SendLogMsg(L"", strFailedInfo);
			}
			m_mapMdobusEngine[strSer] = pModbusengine;
		}
	}
	return true;
}

int CBEOPDataLinkManager::GetModbusEngineCount()
{
	return m_mapMdobusEngine.size();
}

int CBEOPDataLinkManager::GetCO3PEngineCount()
{
	return m_mapCO3PEngine.size();
}

int CBEOPDataLinkManager::GetProtocol104EngineCount()
{
	return m_mapProtocol104Engine.size();
}

vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetModbusServer( vector<DataPointEntry> modbuspoint )
{
	vector<vector<DataPointEntry>> vecModbusPList;
	m_vecModbusSer.clear();
	for(int i=0; i<modbuspoint.size(); i++)
	{
		//
		wstring strSer = modbuspoint[i].GetServerAddress();
		if(strSer.empty())
			continue;

		int nPos = IsExistInVec(strSer,m_vecModbusSer);
		if(nPos==m_vecModbusSer.size())	//不存在
		{
			m_vecModbusSer.push_back(strSer);
			vector<DataPointEntry> vecPList;
			vecModbusPList.push_back(vecPList);
		}
		vecModbusPList[nPos].push_back(modbuspoint[i]);		
	}
	return vecModbusPList;
}

bool CBEOPDataLinkManager::InitProtocol104Engine( vector<DataPointEntry> protocol04point )
{
	if(protocol04point.empty())
		return false;

	vector<vector<DataPointEntry>> vecServerList = GetProtocol104Server(protocol04point);
	for(int i=0; i<vecServerList.size(); i++)
	{
		if(!vecServerList[i].empty())
		{
			CProtocol104Engine* p104engine = new CProtocol104Engine(this);
			p104engine->SetPointList(vecServerList[i]);
			p104engine->SetLogSession(m_dbsession_log);
			wstring strSer = vecServerList[i][0].GetParam(1);
			if(!p104engine->Init())
			{
				CString strFailedInfo;
				strFailedInfo.Format(L"ERROR: Init Protocol-104 Engine(%s) Failed.\r\n", strSer.c_str());
				SendLogMsg(L"", strFailedInfo);
			}
			m_mapProtocol104Engine[strSer] = p104engine;
		}
	}
	return true;
}

vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetProtocol104Server( vector<DataPointEntry> protocol04point )
{
	vector<vector<DataPointEntry>> vecpro104PList;
	vecpro104PList.clear();
	for(int i=0; i<protocol04point.size(); i++)
	{
		//
		wstring strSer = protocol04point[i].GetParam(1);
		if(strSer.empty())
			continue;

		int nPos = IsExistInVec(strSer,m_vecProtocol104Ser);
		if(nPos==m_vecProtocol104Ser.size())	//不存在
		{
			m_vecProtocol104Ser.push_back(strSer);
			vector<DataPointEntry> vecPList;
			vecpro104PList.push_back(vecPList);
		}
		vecpro104PList[nPos].push_back(protocol04point[i]);		
	}
	return vecpro104PList;
}

int CBEOPDataLinkManager::IsExistInVec( wstring strSer, vector<wstring> vecSer )
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strSer == vecSer[i])
		{
			return i;
		}
	}
	return i;
}

bool CBEOPDataLinkManager::InitCO3PEngine( vector<DataPointEntry> co3ppoint, int nReadCmdMs /*= 50*/ ,int nRollInterval,int nTimeOut,int nPrecision)
{
	if(co3ppoint.empty())
		return false;

	vector<vector<DataPointEntry>> vecServerList = GetCO3PServer(co3ppoint);

	for(int i=0; i<vecServerList.size(); i++)
	{
		if(!vecServerList[i].empty())
		{
			CCO3PEngine* pCO3Pengine = new CCO3PEngine(this);
			wstring strSer = vecServerList[i][0].GetParam(1);
			pCO3Pengine->SetPointList(vecServerList[i]);
			pCO3Pengine->SetLogSession(m_dbsession_log);
			pCO3Pengine->SetSendCmdTimeInterval(nReadCmdMs,nRollInterval,nTimeOut,nPrecision);
			pCO3Pengine->SetIndex(i);
			if(!pCO3Pengine->Init())
			{
				CString strFailedInfo;
				strFailedInfo.Format(L"ERROR: Init CO3P Engine %d(%s) Failed.\r\n", i,strSer.c_str());
				SendLogMsg(L"", strFailedInfo);
			}
			m_mapCO3PEngine[strSer] = pCO3Pengine;
		}
	}
	return true;
}

vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetCO3PServer( vector<DataPointEntry> co3ppoint )
{
	vector<vector<DataPointEntry>> vecCO3PPList;
	m_vecCO3PSer.clear();
	for(int i=0; i<co3ppoint.size(); i++)
	{
		//
		wstring strSer = co3ppoint[i].GetParam(1);
		if(strSer.empty())
			continue;

		int nPos = IsExistInVec(strSer,m_vecCO3PSer);
		if(nPos==m_vecCO3PSer.size())	//不存在
		{
			m_vecCO3PSer.push_back(strSer);
			vector<DataPointEntry> vecPList;
			vecCO3PPList.push_back(vecPList);
		}
		vecCO3PPList[nPos].push_back(co3ppoint[i]);		
	}
	return vecCO3PPList;
}

bool CBEOPDataLinkManager::InitMysqlEngine( vector<DataPointEntry> mysqlpoint )
{
	if(mysqlpoint.empty())
		return false;

	vector<vector<DataPointEntry>> vecServerList = GetMysqlServer(mysqlpoint);
	for(int i=0; i<vecServerList.size(); i++)
	{
		if(!vecServerList[i].empty())
		{
			CMysqlEngine* pMysqlengine = new CMysqlEngine(this);
			pMysqlengine->SetPointList(vecServerList[i]);
			pMysqlengine->SetLogSession(m_dbsession_log);
			pMysqlengine->SetIndex(i);
			wstring strSer = vecServerList[i][0].GetParam(3) + vecServerList[i][0].GetParam(8);
			if(!pMysqlengine->Init())
			{
				CString strFailedInfo;
				strFailedInfo.Format(L"ERROR: Init Mysql Engine(%s) Failed.\r\n", strSer.c_str());
				SendLogMsg(L"", strFailedInfo);
			}
			m_mapMysqlEngine[strSer] = pMysqlengine;
		}
	}
	return true;
}

bool CBEOPDataLinkManager::InitSqlServerEngine( vector<DataPointEntry> sqlpoint )
{
	if(sqlpoint.empty())
		return false;

	vector<vector<DataPointEntry>> vecServerList = GetSqlServer(sqlpoint);
	for(int i=0; i<vecServerList.size(); i++)
	{
		if(!vecServerList[i].empty())
		{
			CSqlServerEngine* pSqlServerengine = new CSqlServerEngine(this);
			pSqlServerengine->SetPointList(vecServerList[i]);
			pSqlServerengine->SetLogSession(m_dbsession_log);
			pSqlServerengine->SetIndex(i);
			wstring strSer = vecServerList[i][0].GetParam(3) + vecServerList[i][0].GetParam(8);
			if(!pSqlServerengine->Init())
			{
				CString strFailedInfo;
				strFailedInfo.Format(L"ERROR: Init SqlServer Engine(%s) Failed.\r\n", strSer.c_str());
				SendLogMsg(L"", strFailedInfo);
			}
			m_mapSqlServerEngine[strSer] = pSqlServerengine;
		}
	}
	return true;
}

vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetMysqlServer( vector<DataPointEntry> mysqlpoint )
{
	vector<vector<DataPointEntry>> vecMysqlPList;
	m_vecMysqlSer.clear();
	for(int i=0; i<mysqlpoint.size(); i++)
	{
		//
		wstring strIP = mysqlpoint[i].GetParam(3);
		wstring strSchema = mysqlpoint[i].GetParam(8);
		if(strIP.empty() || strSchema.empty())
			continue;

		int nPos = IsExistInVec(strIP,strSchema,m_vecMysqlSer);
		if(nPos==m_vecMysqlSer.size())	//不存在
		{
			MysqlServer pServer;
			pServer.strIP = strIP;
			pServer.strSchema = strSchema;
			m_vecMysqlSer.push_back(pServer);

			vector<DataPointEntry> vecPList;
			vecMysqlPList.push_back(vecPList);
		}
		vecMysqlPList[nPos].push_back(mysqlpoint[i]);		
	}
	return vecMysqlPList;
}

int CBEOPDataLinkManager::IsExistInVec( wstring strIP,wstring strSechema, vector<MysqlServer> vecSer )
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strIP == vecSer[i].strIP && strSechema == vecSer[i].strSchema)
		{
			return i;
		}
	}
	return i;
}

int CBEOPDataLinkManager::IsExistInVec( wstring strSer, wstring strPort,vector<FCServer> vecSer )
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strSer == vecSer[i].strIP && strPort == vecSer[i].strPort)
		{
			return i;
		}
	}
	return i;
}

int CBEOPDataLinkManager::IsExistInVec( wstring strSer, wstring strProgram,vector<OPCServer>& vecSer ,int nMaxSize)
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strSer == vecSer[i].strIP && strProgram == vecSer[i].strServerName && vecSer[i].nCount < nMaxSize)
		{
			vecSer[i].nCount++;
			return i;
		}
	}
	return i;
}

int CBEOPDataLinkManager::IsExistInVec(wstring strPath,wstring strTName, vector<SqliteServer> vecSer)
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strPath == vecSer[i].strFilePath && strTName == vecSer[i].strFileTable)
		{
			return i;
		}
	}
	return i;
}

int CBEOPDataLinkManager::IsExistInVec( wstring strIP,wstring strSechema, vector<SqlServer> vecSer )
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strIP == vecSer[i].strIP && strSechema == vecSer[i].strSchema)
		{
			return i;
		}
	}
	return i;
}

int CBEOPDataLinkManager::IsExistInVec( wstring strIP,wstring strUser, wstring strPsw,vector<WebServer> vecSer )
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strIP == vecSer[i].strIP && strUser == vecSer[i].strUser && strPsw == vecSer[i].strPsw)
		{
			return i;
		}
	}
	return i;
}

//初始化input表
//如果历史表的点数量与读点表得到的点数量不一致时，则认为点表发生变化，则用新点表，老点的值要逐一赋给
//如果数量一致，则用历史点表数据覆盖
//bool CBEOPDataLinkManager::InitInputTable()
//{
//	/**读取input表，若为空，则将buffer表内容复制进realtime表；
//	   若不为空，则不作处理，认为数据库尚未重启，数据依然有效 */
//	vector<CRealTimeDataEntry> resultList;
//	if(!m_dbsession_history->ReadPointBufferData(resultList))
//	{
//		SendLogMsg(L"RealtimeInput",L"ERROR: Read Point Buffer Data Failed.\r\n");
//		return false;
//	}
//
//	CString strTemp;
//	strTemp.Format(L"Read Numbers of Point:%d from point value buffer.\r\n", resultList.size());
//	SendLogMsg(L"RealtimeInput",strTemp.GetString());
//
//	vector<DataPointEntry> entryList;
//	m_pointmanager->GetMemoryPointList(entryList);
//	/** 差异比较  */
//	
//	SYSTEMTIME st;
//	GetLocalTime(&st);
//	vector<CRealTimeDataEntry> resultFinalList; //最终插入的点和值清单
//	vector<CRealTimeDataEntry> newVPoingList; //最终插入的点和值清单
//	for(int i=0;i<entryList.size();i++)
//	{
//		POINT_STORE_CYCLE storeCycle =  entryList[i].GetStoreCycle();
//		if(storeCycle < m_nMinTCycle && storeCycle!=E_STORE_NULL)
//			m_nMinTCycle = entryList[i].GetStoreCycle();
//
//		if(entryList[i].GetSourceType()!=L"vpoint")
//		{
//			continue;
//		}
//
//		bool bFind = false;
//		wstring strPointName = entryList[i].GetShortName();
//		string ansiNameTemp = Project::Tools::WideCharToAnsi(strPointName.c_str());
//		for(int j=0;j<resultList.size();j++)
//		{
//			if(resultList[j].GetName()==ansiNameTemp) //若找到
//			{
//				resultFinalList.push_back(resultList[j]);
//				resultList.erase(resultList.begin()+j);
//				bFind = true;
//				break;
//			}
//		}
//
//		if(!bFind) //新点,只有新的软点才进入数据库实时表，否则opc点值不对会带来误解
//		{
//			if(entryList[i].GetSourceType()==L"vpoint")
//			{
//				Beopdatalink::CRealTimeDataEntry entryNew;
//				entryNew.SetName(ansiNameTemp);
//				entryNew.SetValue(L"0");
//				entryNew.SetTimestamp(st);
//				resultFinalList.push_back(entryNew);
//				newVPoingList.push_back(entryNew);
//				
//			}
//		}
//	}
//	//刷realtime_input表
//	bool bSuccess = m_dbsession_realtime->InsertRealTimeDatas_Input(resultFinalList);
//	if(!bSuccess)
//	{
//		SendLogMsg(L"RealtimeInput",L"ERROR: Insert Realtime Datas Failed.\r\n");
//	}
//
//	//新增加的软点必须在buffer中加入   
//	if(newVPoingList.size()>0)
//		bSuccess = bSuccess && m_dbsession_history->InsertPointBuffer(newVPoingList);
//
//	//清除不必要的点（遗留）
//	if(resultList.size()>0)
//	{
//
//	}
//
//	return bSuccess;
//}

bool CBEOPDataLinkManager::InitInputTable()
{
	/**读取input表，若为空，则将buffer表内容复制进realtime表；
	   若不为空，则不作处理，认为数据库尚未重启，数据依然有效 */
	hash_map<string,CRealTimeDataEntry> resultList;
	if(!m_dbsession_history->ReadPointBufferData(resultList))
	{
		SendLogMsg(L"RealtimeInput",L"ERROR: Read Point Buffer Data Failed.\r\n");
		return false;
	}

	CString strTemp;
	strTemp.Format(L"INFO : Read Numbers of Point:%d from point value buffer.\r\n", resultList.size());
	SendLogMsg(L"RealtimeInput",strTemp.GetString());

	vector<DataPointEntry> entryList;
	m_pointmanager->GetMemoryPointList(entryList);
	/** 差异比较  */
	
	m_nMinTCycle = m_pointmanager->GetMinCycle();
	if(m_nMinTCycle > E_STORE_FIVE_SECOND)
	{
		m_nMinTCycle = E_STORE_ONE_MINUTE;				//
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	vector<CRealTimeDataEntry> resultFinalList; //最终插入的点和值清单
	vector<CRealTimeDataEntry> newVPoingList; //最终插入的点和值清单
	for(int i=0;i<entryList.size();i++)
	{
		if(entryList[i].GetSourceType()!=L"vpoint" 
			&& entryList[i].GetSourceType()!=L"ModbusDirecctReadInServer"
			&& entryList[i].GetSourceType()!=L"ModbusDirectReadInServer")
		{
			continue;
		}

		bool bFind = false;
		wstring strPointName = entryList[i].GetShortName();
		string ansiNameTemp = Project::Tools::WideCharToAnsi(strPointName.c_str());
		hash_map<string,CRealTimeDataEntry>::iterator findEntry = resultList.find(ansiNameTemp);
		if(findEntry != resultList.end())		//找到
		{
			resultFinalList.push_back((*findEntry).second);
		}
		else
		{
			Beopdatalink::CRealTimeDataEntry entryNew;
			entryNew.SetName(ansiNameTemp);
			entryNew.SetValue(L"0");
			entryNew.SetTimestamp(st);
			resultFinalList.push_back(entryNew);
			newVPoingList.push_back(entryNew);
		}
	}
	//刷realtime_input表
	bool bSuccess = m_dbsession_realtime->InsertRealTimeDatas_Input(resultFinalList,m_nMaxInsertLimit);
	if(!bSuccess)
	{
		SendLogMsg(L"RealtimeInput",L"ERROR: Insert Realtime Datas Failed.\r\n");
	}

	//新增加的软点必须在buffer中加入   
	if(newVPoingList.size()>0)
		bSuccess = bSuccess && m_dbsession_history->InsertPointBuffer(newVPoingList);

	//清除不必要的点（遗留）
	if(resultList.size()>0)
	{

	}

	return bSuccess;
}

bool   CBEOPDataLinkManager::UpdateRedis(COleDateTime tUpdate, vector<CRealTimeDataEntry > &entrylist)
{
	if(entrylist.size()==0)
		return false;

	Json::Value pvAllRoot;
	if(m_Redis.is_valid())
	{
		for(int i=0;i<entrylist.size();i++)
		{
			string strPointName_utf8 = entrylist[i].GetName();
			string strPointValue_utf8 ;
			Project::Tools::WideCharToUtf8(entrylist[i].GetValue(), strPointValue_utf8);
			pvAllRoot[strPointName_utf8] = strPointValue_utf8;
		}

		string strDict = pvAllRoot.toStyledString();
		wstring wstrRootAll;
		Project::Tools::UTF8ToWideChar(strDict, wstrRootAll);
		m_Redis.set_value(L"realtime_data",wstrRootAll );

		CString strTimeUpdate = tUpdate.Format(L"%Y-%m-%d %H:%M:%S");
		string strTimeUpdate_utf8;
		Project::Tools::WideCharToUtf8(strTimeUpdate.GetString(), strTimeUpdate_utf8);
		Json::Value pvTime;
		pvTime["time"] = strTimeUpdate_utf8;


		wstring wstrRootAll_UpdateTime;
		Project::Tools::UTF8ToWideChar(pvTime.toStyledString(), wstrRootAll_UpdateTime);

		m_Redis.set_value(L"realtime_data_update_time",wstrRootAll_UpdateTime );
	}

	if(m_Redis_To_DeepLogic.is_valid() && entrylist.size()>0)
	{

		vector<string> strPNList;
		vector<string> strPVList;
		for(int i=0;i<entrylist.size();i++)
		{
			string strTime = Project::Tools::SystemTimeToString(entrylist[i].GetTimestamp());
			string strPointName_write = "ACQDATA_R_" + entrylist[i].GetName();
			string strPointValue_utf8 ;
			Project::Tools::WideCharToUtf8(entrylist[i].GetValue(), strPointValue_utf8);

			string strPointValue_write = entrylist[i].GetName()+",0," + strTime +"," + strPointValue_utf8+ ",0";

			strPNList.push_back(strPointName_write);
			strPVList.push_back(strPointValue_write);
		}

		m_Redis_To_DeepLogic.set_value_mul_utf8(strPNList, strPVList);
	}

	return true;
}

bool CBEOPDataLinkManager::UpdateInputTable()
{
	if (!IsMasterMode())
	{
		SendLogMsg(L"RealtimeInput", L"Not In Master Mode, no update input table!");
		return false;
	}

	


	// first, get all the data
	vector<pair<wstring, wstring> >	valuesets;
	COleDateTime oleStart;
	COleDateTimeSpan oleSpan;
	CString strOut;


	if(m_mapOPCEngine.size()>0 || m_opcengine!=NULL)
	{
		oleStart = COleDateTime::GetCurrentTime();
		GetOPCValueSets(valuesets);	
		oleSpan = COleDateTime::GetCurrentTime() - oleStart;
		
		//strOut.Format(_T("TimeLog: GetOPCValueSets num:%d time:%f \r\n"), valuesets.size(),oleSpan.GetTotalSeconds());
		//_tprintf(strOut);
	}

	GetModbusValueSets(valuesets);
	GetFCbusValueSets(valuesets);
	GetProtocol104ValueSets(valuesets);
	GetWebValueSets(valuesets);
	GetMysqlValueSets(valuesets);
	GetSqlServerValueSets(valuesets);
	GetSqliteValueSets(valuesets);
	GetBacnetValueSets(valuesets);
	GetCO3PValueSets(valuesets);
	if(m_bModbusEquipmentEngineEnable)
	{
		GetModbusEquipmentValueSets(valuesets);
	}

	if(m_bCoreStationEngineEnable)
	{
		GetCoreStationValueSets(valuesets);
	}

	if(m_bKNXEngineEnable)
	{
		GetKNXValueSets(valuesets);
	}

	if(m_bDLT645EngineEnable)
	{
		GetDLT645ValueSets(valuesets);
	}

	if(m_bOPCUAEngineEnable)
	{
		GetOPCUAValueSets(valuesets);
	}

	if(m_bBacnetPyEngineEnable)
	{
		GetBacnetPyValueSets(valuesets);
	}

	if(m_bPersagyControllerEngineEnable)
	{
		GetPersagyControllerValueSets(valuesets);
	}

	if(m_bObixEngineEnable)
	{
		GetObixValueSets(valuesets);
	}

	if(m_bLogixEngineEnable)
	{
		GetLogixValueSets(valuesets);
	}


	if(m_bABSLCEngineEnable)
	{
		GetABSLCValueSets(valuesets);
	}

	if(m_bMoxaTCPServerEngineEnable)
	{
		GetMoxaTCPServerValueSets(valuesets);
	}

	if(m_bThirdPartyEngineEnable)
	{
		GetThirdPartyValueSets(valuesets);
	}

	if (m_memoryengine){
		m_memoryengine->GetValueSet(valuesets);
	}

	GetSiemensTCPValueSets(valuesets);

	if(m_pDiagnoseEngine)
	{
		m_pDiagnoseEngine->GetValueSet(valuesets);
	}
		
	vector<CRealTimeDataEntry> entrylist;

	SYSTEMTIME st;
	GetLocalTime(&st);
	CRealTimeDataEntry entry;
	for (unsigned int i = 0; i < valuesets.size(); i++)
	{
		const pair<wstring, wstring>& pval = valuesets[i];
		entry.SetTimestamp(st);
		string ansiname = Project::Tools::WideCharToAnsi(pval.first.c_str());
		entry.SetName(ansiname);
		entry.SetValue(pval.second);
		entrylist.push_back(entry);
	}

	SetRealTimeDataEntryList(entrylist);
	
	oleStart = COleDateTime::GetCurrentTime();
	bool bresult = m_dbsession_realtime->InsertRealTimeDatas_Input(entrylist,m_nMaxInsertLimit);
	oleSpan = COleDateTime::GetCurrentTime() - oleStart;
	strOut.Format(_T("Time: InsertRealTimeDatas_Input num:%d  time:%f \r\n"), entrylist.size(),oleSpan.GetTotalSeconds());
	//_tprintf(strOut);
	//loginfoxx.Format(_T(""));

	//redis update

	UpdateRedis(oleStart, entrylist);

	CString loginfo;
	
	//失败时记载
	if(!bresult)
	{
		loginfo.Format(_T("ERROR: DataEngineLog Update points(total nums: %d) To Table Failed!\r\n"), valuesets.size());
		SendLogMsg(L"RealtimeInput", loginfo.GetString());

		if(m_bLog)
			AddLog(loginfo.GetString());

		return false;
	}
	else
	{

		return true;

	}

	valuesets.clear();//golding add
	entrylist.clear();//golding add

	return true;
	
}

// the update params functions is a bit complicated.
bool	CBEOPDataLinkManager::UpdateOutputParams(wstring &strChangedValues)
{
	// read the output params
	if (!m_dbsession_realtime){
		return false;
	}

	if (!IsMasterMode())
	{
		SendLogMsg(L"RealtimeOutput",L"ERROR: Not In Master Mode, no update output table!");
		return false;
	}

	m_vecOutputEntry.clear();
	bool bSuccess = m_dbsession_realtime->ReadRealTimeData_Output(m_vecOutputEntry);
	if(!bSuccess)
	{
		CString loginfo;
		loginfo.Format(_T("ERROR: ReadRealTimeData_Output Failed!"));
		SendLogMsg(L"RealtimeOutput", loginfo);

		return false;
	}
	int nChangeCount = m_vecOutputEntry.size();
	if(nChangeCount>0)
	{
		ChangeValue(strChangedValues);
	}

	return true;
}

 void CBEOPDataLinkManager::SetPointManager( CDataPointManager* pointmanager )
 {
	 m_pointmanager = pointmanager;
 }

 void CBEOPDataLinkManager::SetDbCon( CRealTimeDataAccess* pdbcon )
 {
	 m_dbsession_realtime = pdbcon;
 }

 void CBEOPDataLinkManager::ChangeValue(wstring &strChangedValues)
 {
	 strChangedValues = L"";
	 if (!IsMasterMode()){
		 return;
	 }

	 vector<CRealTimeDataEntry> memoryPointChangeList;

	 int nchangecount = m_vecOutputEntry.size();

	 wstring strChanged_DLT645, strChanged_KNX, strChanged_OPC, strChanged_Modbus, strChanged_Bacnet, strChanged_VPoint, 
		 strChanged_PLC, strChanged_Mysql, strChanged_Web, strChanged_Sqlite,strChanged_PersagyController, strChanged_SqlServer, strChanged_Unknown,
		 strChanged_Obix, strChanged_Logix, strChanged_Moxa, strChanged_ABSLC;
	 if(m_bRealSite)
	 {
		 for (unsigned int i = 0; i < nchangecount; i++)
		 {
			 const CRealTimeDataEntry& entry = m_vecOutputEntry[i];
			 bool bwriteresult = false;
			 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());

			 DataPointEntry pointentry = m_pointmanager->GetEntry(pointname);
			 Beopdatalink::CRealTimeDataEntry tempentry = entry;

			 if (pointentry.IsOpcPoint())
			 {
				 bwriteresult = WriteOPCValue(tempentry);
				 if(strChanged_OPC.length()<=0)
					 strChanged_OPC+=L"--------Changed OPC Points-------\r\n";

				 strChanged_OPC +=L"    (OPC) ";
				 strChanged_OPC += pointname;
				 strChanged_OPC += L"->";
				 strChanged_OPC += tempentry.GetValue();
				 strChanged_OPC += L";\r\n";
			 }
			 else if (pointentry.IsModbusPoint())
			 {
				 bwriteresult =  WriteModbusValue(tempentry);
				 if(strChanged_Modbus.length()<=0)
					 strChanged_Modbus+=L"--------Changed Modbus Points--------\r\n";

				 strChanged_Modbus +=L"    (modbus) ";
				 strChanged_Modbus += pointname;
				 strChanged_Modbus += L"->";
				 strChanged_Modbus += tempentry.GetValue();
				 strChanged_Modbus += L";\r\n";
			 }
			 else if (pointentry.IsVPoint())
			 {
				 //更新内存
				 bwriteresult = WriteMemoryPoint(tempentry);
				 if(bwriteresult)
					 memoryPointChangeList.push_back(tempentry);
				 //软点不直接修改内存表，因为下次会由内存更新到realtime_input
				 //bwriteresult = m_dbsession_realtime->UpdatePointValue_Input(entry);
			 }
			 else if (pointentry.IsBacnetPoint())
			 {
				 bwriteresult = WriteBacnetPoint(tempentry);
				 if(strChanged_Bacnet.length()<=0)
					 strChanged_Bacnet+=L"--------Changed Bacnet Points--------\r\n";

				 strChanged_Bacnet +=L"    (bacnet) ";
				 strChanged_Bacnet += pointname;
				 strChanged_Bacnet += L"->";
				 strChanged_Bacnet += tempentry.GetValue();
				 strChanged_Bacnet += L";\r\n";
			 }
			 else if(pointentry.IsSiemens1200Point() || pointentry.IsSiemens300Point())
			 {
				 bwriteresult = WriteSiemensTCPPoint(pointentry.GetParam(3), tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_PLC.length()<=0)
						 strChanged_PLC+=L"--------Changed SiemensTCP Points--------\r\n";

					 strChanged_PLC +=L"    (SiemensTCP) ";
					 strChanged_PLC += pointname;
					 strChanged_PLC += L"->";
					 strChanged_PLC += tempentry.GetValue();
					 strChanged_PLC += L";\r\n";
				 }

			 }
			 else if(pointentry.IsWebPoint())
			 {
				 bwriteresult = WriteWebPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Web.length()<=0)
						 strChanged_Web+=L"--------Changed Web Points--------\r\n";

					 strChanged_Web +=L"    (Web) ";
					 strChanged_Web += pointname;
					 strChanged_Web += L"->";
					 strChanged_Web += tempentry.GetValue();
					 strChanged_Web += L";\r\n";
				 }
				 else
				 {
					 strChanged_Web +=L"ERROR: Web Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsMysqlPoint())
			 {
				 bwriteresult = WriteMysqlPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Mysql.length()<=0)
						 strChanged_Mysql+=L"--------Changed Mysql Points--------\r\n";

					 strChanged_Mysql +=L"    (Mysql) ";
					 strChanged_Mysql += pointname;
					 strChanged_Mysql += L"->";
					 strChanged_Mysql += tempentry.GetValue();
					 strChanged_Mysql += L";\r\n";
				 }
				 else
				 {
					 strChanged_Mysql +=L"ERROR: MySQL Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsSqlServerPoint())
			 {
				 bwriteresult = WriteSqlServerPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_SqlServer.length()<=0)
						 strChanged_SqlServer+=L"--------Changed SqlServer Points--------\r\n";

					 strChanged_SqlServer +=L"    (SqlServer) ";
					 strChanged_SqlServer += pointname;
					 strChanged_SqlServer += L"->";
					 strChanged_SqlServer += tempentry.GetValue();
					 strChanged_SqlServer += L";\r\n";
				 }
				 else
				 {
					 strChanged_SqlServer +=L"ERROR: SqlServer Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsSqlitePoint())
			 {
				 bwriteresult = WriteSqlitePoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Sqlite.length()<=0)
						 strChanged_Sqlite+=L"--------Changed Sqlite Points--------\r\n";

					 strChanged_Sqlite +=L"    (Sqlite) ";
					 strChanged_Sqlite += pointname;
					 strChanged_Sqlite += L"->";
					 strChanged_Sqlite += tempentry.GetValue();
					 strChanged_Sqlite += L";\r\n";
				 }
				 else
				 {
					 strChanged_Sqlite +=L"ERROR: Sqlite Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsWirelessPoint())
			 {
				 bwriteresult = WriteWirelessPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Sqlite.length()<=0)
						 strChanged_Sqlite+=L"--------Changed Wiress Points--------\r\n";

					 strChanged_Sqlite +=L"    (Wireless) ";
					 strChanged_Sqlite += pointname;
					 strChanged_Sqlite += L"->";
					 strChanged_Sqlite += tempentry.GetValue();
					 strChanged_Sqlite += L";\r\n";
				 }
				 else
				 {
					 strChanged_Sqlite +=L"ERROR: Wireless Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsDLT645Point())
			 {
				 bwriteresult = WriteDLT645Point(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_DLT645.length()<=0)
						 strChanged_DLT645+=L"--------Changed DLT645 Points--------\r\n";

					 strChanged_DLT645 +=L"    (DLT645) ";
					 strChanged_DLT645 += pointname;
					 strChanged_DLT645 += L"->";
					 strChanged_DLT645 += tempentry.GetValue();
					 strChanged_DLT645 += L";\r\n";
				 }
				 else
				 {
					 strChanged_DLT645 +=L"ERROR: DLT645 Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsKNXPoint())
			 {
				 bwriteresult = WriteKNXPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_KNX.length()<=0)
						 strChanged_KNX+=L"--------Changed KNX Points--------\r\n";

					 strChanged_KNX +=L"    (KNX) ";
					 strChanged_KNX += pointname;
					 strChanged_KNX += L"->";
					 strChanged_KNX += tempentry.GetValue();
					 strChanged_KNX += L";\r\n";
				 }
				 else
				 {
					 strChanged_KNX +=L"ERROR: KNX Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsPersagyControllerPoint())
			 {
				 bwriteresult = WritePersagyControllerPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_PersagyController.length()<=0)
						 strChanged_PersagyController+=L"--------Changed PersagyController Points--------\r\n";

					 strChanged_PersagyController +=L"    (PersagyController) ";
					 strChanged_PersagyController += pointname;
					 strChanged_PersagyController += L"->";
					 strChanged_PersagyController += tempentry.GetValue();
					 strChanged_PersagyController += L";\r\n";
				 }
				 else
				 {
					 strChanged_PersagyController +=L"ERROR: PersagyController Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsModbusEquipmentPoint())
			 {
				 bwriteresult = WriteModbusEquipmentPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Sqlite.length()<=0)
						 strChanged_Sqlite+=L"--------Changed ModbusEquipment Points--------\r\n";

					 strChanged_Sqlite +=L"    (ModbusEquipment) ";
					 strChanged_Sqlite += pointname;
					 strChanged_Sqlite += L"->";
					 strChanged_Sqlite += tempentry.GetValue();
					 strChanged_Sqlite += L";\r\n";
				 }
				 else
				 {
					 strChanged_Sqlite +=L"ERROR: ModbusEquipment Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsOPCUAPoint())
			 {
				 bwriteresult = WriteOPCUAPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Sqlite.length()<=0)
						 strChanged_Sqlite+=L"--------Changed OPCUA Points--------\r\n";

					 strChanged_Sqlite +=L"    (OPCUA) ";
					 strChanged_Sqlite += pointname;
					 strChanged_Sqlite += L"->";
					 strChanged_Sqlite += tempentry.GetValue();
					 strChanged_Sqlite += L";\r\n";
				 }
				 else
				 {
					 strChanged_Sqlite +=L"ERROR: OPCUA Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsBacnetPyPoint())
			 {
				 bwriteresult = WriteBacnetPyPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Sqlite.length()<=0)
						 strChanged_Sqlite+=L"--------Changed BacnetPy Points--------\r\n";

					 strChanged_Sqlite +=L"    (BacnetPy) ";
					 strChanged_Sqlite += pointname;
					 strChanged_Sqlite += L"->";
					 strChanged_Sqlite += tempentry.GetValue();
					 strChanged_Sqlite += L";\r\n";
				 }
				 else
				 {
					 strChanged_Sqlite +=L"ERROR: BacnetPy Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsThirdPartyPoint())
			 {
				 bwriteresult = WriteThirdPartyPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Sqlite.length()<=0)
						 strChanged_Sqlite+=L"--------Changed ThirdParty Points--------\r\n";

					 strChanged_Sqlite +=L"    (ThirdParty) ";
					 strChanged_Sqlite += pointname;
					 strChanged_Sqlite += L"->";
					 strChanged_Sqlite += tempentry.GetValue();
					 strChanged_Sqlite += L";\r\n";
				 }
				 else
				 {
					 strChanged_Sqlite +=L"ERROR: ThirdParty Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsObixPoint())
			 {
				 bwriteresult = WriteObixPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Obix.length()<=0)
						 strChanged_Obix+=L"--------Changed Obix Points--------\r\n";

					 strChanged_Obix +=L"    (Obix) ";
					 strChanged_Obix += pointname;
					 strChanged_Obix += L"->";
					 strChanged_Obix += tempentry.GetValue();
					 strChanged_Obix += L";\r\n";
				 }
				 else
				 {
					 strChanged_Obix +=L"ERROR: Obix Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsLogixPoint())
			 {
				 bwriteresult = WriteLogixPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Logix.length()<=0)
						 strChanged_Logix+=L"--------Changed Logix Points--------\r\n";

					 strChanged_Logix +=L"    (Logix) ";
					 strChanged_Logix += pointname;
					 strChanged_Logix += L"->";
					 strChanged_Logix += tempentry.GetValue();
					 strChanged_Logix += L";\r\n";
				 }
				 else
				 {
					 strChanged_Logix +=L"ERROR: Logix Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsABSLCPoint())
			 {
				 bwriteresult = WriteABSLCPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_ABSLC.length()<=0)
						 strChanged_ABSLC+=L"--------Changed ABSLC Points--------\r\n";

					 strChanged_ABSLC +=L"    (ABSLC) ";
					 strChanged_ABSLC += pointname;
					 strChanged_ABSLC += L"->";
					 strChanged_ABSLC += tempentry.GetValue();
					 strChanged_ABSLC += L";\r\n";
				 }
				 else
				 {
					 strChanged_ABSLC +=L"ERROR: ABSLC Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsMoxaTCPServerPoint())
			 {
				 bwriteresult = WriteMoxaTCPServerPoint(tempentry);
				 if(bwriteresult)
				 {
					 if(strChanged_Moxa.length()<=0)
						 strChanged_Moxa+=L"--------Changed MoxaTCPServer Points--------\r\n";

					 strChanged_Moxa +=L"    (MoxaTCPServer) ";
					 strChanged_Moxa += pointname;
					 strChanged_Moxa += L"->";
					 strChanged_Moxa += tempentry.GetValue();
					 strChanged_Moxa += L";\r\n";
				 }
				 else
				 {
					 strChanged_Moxa +=L"ERROR: MoxaTCPServer Point Value Set Failed:";

				 }
			 }
			 else if(pointentry.IsUnknownPoint())
			 {
				
				 if(strChanged_Unknown.length()<=0)
					 strChanged_Unknown+=L"--------ERROR: Unknown Points--------\r\n";

				 strChanged_Unknown +=L"    (Unkown) ";
				 strChanged_Unknown += pointname;
				 strChanged_Unknown += L"->";
				 strChanged_Unknown += tempentry.GetValue();
				 strChanged_Unknown += L";\r\n";

			 }

			 m_dbsession_realtime->DeleteEntryInOutput(entry.GetName());
		}
	 }
	 else
	 {
			//不与现场连接时的处理
			m_dbsession_realtime->UpdatePointValue_InputArray(m_vecOutputEntry);


			for (unsigned int i = 0; i < nchangecount; i++)
			{
				const CRealTimeDataEntry& entry = m_vecOutputEntry[i];
				wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
				DataPointEntry pointentry = m_pointmanager->GetEntry(pointname);
				if (pointentry.IsVPoint())
				{
					bool bwriteresult = WriteMemoryPoint(entry);
					if(bwriteresult)
						memoryPointChangeList.push_back(entry);
				}

				m_dbsession_realtime->DeleteEntryInOutput(entry.GetName());
			}

	 }


	 if(memoryPointChangeList.size()>0)
	 {
		 Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock_vecMemoryPointChangeList);
		 for(int nIndex =0;nIndex<memoryPointChangeList.size();nIndex++)
			 m_vecMemoryPointChangeList.push_back(memoryPointChangeList[nIndex]);

		 strChanged_VPoint = L"--------Changed Memory Points--------\r\n";
		 strChanged_VPoint += L"Numbers: ";
		 strChanged_VPoint += to_wstring((long long) memoryPointChangeList.size());
	 }

	 if(strChanged_OPC.length()>0)
	 {
		strChangedValues+= strChanged_OPC;
		strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Modbus.length()>0)
	 {
		 strChangedValues+= strChanged_Modbus;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Bacnet.length()>0)
	 {
		 strChangedValues+= strChanged_Bacnet;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_VPoint.length()>0)
	 {
		 strChangedValues+= strChanged_VPoint;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_PLC.length()>0)
	 {
		 strChangedValues+= strChanged_PLC;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Web.length()>0)
	 {
		 strChangedValues+= strChanged_Web;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Mysql.length()>0)
	 {
		 strChangedValues+= strChanged_Mysql;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_SqlServer.length()>0)
	 {
		 strChangedValues+= strChanged_SqlServer;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Sqlite.length()>0)
	 {
		 strChangedValues+= strChanged_Sqlite;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Unknown.length()>0)
	 {
		 strChangedValues+= strChanged_Unknown;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_PersagyController.length()>0)
	 {
		 strChangedValues+= strChanged_PersagyController;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Obix.length()>0)
	 {
		 strChangedValues+= strChanged_Obix;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Logix.length()>0)
	 {
		 strChangedValues+= strChanged_Logix;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_Moxa.length()>0)
	 {
		 strChangedValues+= strChanged_Moxa;
		 strChangedValues +=L"\r\n";
	 }
	 if(strChanged_KNX.length()>0)
	 {
		 strChangedValues+= strChanged_KNX;
		 strChangedValues +=L"\r\n";

	 }

	 if(strChanged_DLT645.length()>0)
	 {
		 strChangedValues+= strChanged_DLT645;
		 strChangedValues +=L"\r\n";

	 }
	 if(strChanged_ABSLC.length()>0)
	 {
		 strChangedValues+= strChanged_ABSLC;
		 strChangedValues +=L"\r\n";

	 }
 }


 void CBEOPDataLinkManager::SetMsgWnd( CWnd* msgwnd )
 {
	 m_msgwnd = msgwnd;
 }

 void CBEOPDataLinkManager::SendLogMsg(LPCTSTR logFrom, LPCTSTR loginfo )
 {
	 if(m_dbsession_log)
	 {
		 m_dbsession_log->InsertLog(loginfo);
	 }
	 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DATALINK,E_DEBUG_LOG_LEVEL_BASE,loginfo);
 }

 void CBEOPDataLinkManager::CleatOutputData()
 {
	 if (m_dbsession_realtime){
		 m_dbsession_realtime->DeleteRealTimeData_Output();
	 }
 }

 unsigned int WINAPI CBEOPDataLinkManager::ThreadFuncHistoryData_S5( LPVOID lparam )
 {
	 CBEOPDataLinkManager* pthis = static_cast<CBEOPDataLinkManager*>(lparam);
	 if (pthis != NULL)
	 {
		 int nSecondCount = 0;

		 //string strHostIP = pthis->GetHostIP();
		 COleDateTime tLastSave = COleDateTime::GetCurrentTime();
		 while (!pthis->GetExitThread_History())
		 {
			 COleDateTime tnow = COleDateTime::GetCurrentTime();
			 COleDateTimeSpan tspan = tnow - tLastSave;
			 if(tspan.GetTotalSeconds()>=5)
			 {
				 tLastSave = tnow;

				 SYSTEMTIME stNow;
				 tnow.GetAsSystemTime(stNow);
				 pthis->UpdatePointBuffer(); //更新缓存表
				 pthis->UpdateHistoryStatusTable();

				 if(pthis->m_nMinTCycle <=E_STORE_FIVE_SECOND && pthis->m_bStoreHistory)
				 {
					 vector<Beopdatalink::CRealTimeDataEntry> tempInsertHisEntrylist;
					 pthis->GetRealTimeDataEntryList(tempInsertHisEntrylist, E_STORE_FIVE_SECOND);

					 bool bTableCreated = pthis->CheckCreateHistoryTable(stNow, E_STORE_FIVE_SECOND);
					 
					 if(tempInsertHisEntrylist.size()>0)
					 {
						 pthis->UpdateHistoryTable(stNow, E_STORE_FIVE_SECOND, tempInsertHisEntrylist,tempInsertHisEntrylist.size());
					 }
				 }

				 COleDateTimeSpan tspan1 = tnow - pthis->m_oleLastRemoteSetting;
				 if(pthis->m_nDTUSendMinType == E_STORE_FIVE_SECOND || (tspan1.GetTotalSeconds() <= pthis->m_nSendRemoteSet && pthis->m_nDTUSendMinType >= (int)E_STORE_FIVE_SECOND))
				 {
					 //pthis->SendRealData(E_STORE_FIVE_SECOND);
					 stNow.wSecond = stNow.wSecond - stNow.wSecond%5;
					 pthis->GenerateDTURealDataS5(stNow,E_STORE_FIVE_SECOND,pthis->m_nSendAll);
					 pthis->SetSendDTUEventAndType(0,stNow);
				 }

				 //冗余数据库 存储 buffer input
				 if(pthis->m_bRealSite)
					 pthis->InsertRedundancyInput();

			 }

			 Sleep(1*1000);
			 nSecondCount++;
		 }
	 }

	 return 0;
 }

 //edit dtu发送逻辑 启动时发送全部数据 每天零点也发送全部数据
 unsigned int WINAPI CBEOPDataLinkManager::ThreadFuncHistoryData( LPVOID lparam )
 {
	 CBEOPDataLinkManager* pthis = static_cast<CBEOPDataLinkManager*>(lparam);


	 if (pthis != NULL)
	 {
		 int nSecondCount = 0;	
		 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DATALINK,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Run ThreadFuncHistoryData.\n");
		 pthis->SendCoreStart();
		 while (!pthis->GetExitThread_History())
		 {
			 COleDateTime tnow = COleDateTime::GetCurrentTime();
			 SYSTEMTIME stNow;
			 tnow.GetAsSystemTime(stNow);
			 stNow.wSecond = 0;					//时间精度确定00.000
			 stNow.wMilliseconds = 0;	

			 //第一次启动时候发送全部数据
			 if(pthis->m_bFirstStart && pthis->m_nSendAll == 0)
			 {
				 pthis->SendAllRealData(stNow);
			 }

			 if (tnow.GetMinute()!= pthis->m_tLastHistorySaved.GetMinute())
			 {	
				 pthis->CompareAndUpdateRealTimeDataEntryList(stNow);
				 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DATALINK,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Run ThreadFuncHistoryData->SendCoreStart\n");
				 pthis->SendCoreStart();			//维护心跳包
				 pthis->UpdateDTUSuccess();			//主要更新TCP发送引擎重连
				 pthis->UpdateDebug();				//更新调试设置
				 pthis->SaveBacnetInfo();			//更新Bacnet信息到Unit01
				 CDebugLog::GetInstance()->UpdateDebugSetting();		//主要维护调试log配置信息的更新
				 pthis->UpdateHistoryStatusTable();
				
				 if(tnow.GetMonth()!=pthis->m_tLastHistorySaved.GetMonth()  && pthis->m_nMinTCycle <=E_STORE_ONE_MONTH)
				 {
					 if(pthis->m_bStoreHistory)
					 {
						 vector<Beopdatalink::CRealTimeDataEntry> tempInsertHisEntrylist;
						 pthis->GetRealTimeDataEntryList(tempInsertHisEntrylist, E_STORE_ONE_MONTH);
						 bool bTableCreated = pthis->CheckCreateHistoryTable(stNow, E_STORE_ONE_MONTH);
						
					     pthis->UpdateHistoryTable(stNow, E_STORE_ONE_MONTH, tempInsertHisEntrylist,tempInsertHisEntrylist.size());
					 }

					 if(pthis->m_nDTUSendMinType == (int)E_STORE_ONE_MONTH)
					 {
						 pthis->GenerateDTURealData(stNow,E_STORE_ONE_MONTH,pthis->m_nSendAll);
						 pthis->SetSendDTUEventAndType(0,stNow);
					 }
				 }

				 bool bNewDaySave = tnow.GetDay()!=pthis->m_tLastHistorySaved.GetDay() && pthis->m_nMinTCycle <=E_STORE_ONE_DAY;
				 if(bNewDaySave)
				 {
					 if(pthis->m_bStoreHistory)
					 {
						 vector<Beopdatalink::CRealTimeDataEntry> tempInsertHisEntrylist;
						 pthis->GetRealTimeDataEntryList(tempInsertHisEntrylist, E_STORE_ONE_DAY);
						 bool bTableCreated = pthis->CheckCreateHistoryTable(stNow, E_STORE_ONE_DAY);

						 vector<Beopdatalink::CRealTimeDataEntry> tempNewInsertHisEntrylist;
						 pthis->CombineRealTimeDataEntryList(tempInsertHisEntrylist, pthis->m_RealTimeDataEntryListBufferMap_M1, tempNewInsertHisEntrylist);//合并，避免少了点

						 pthis->UpdateHistoryTable(stNow, E_STORE_ONE_DAY, tempNewInsertHisEntrylist,tempNewInsertHisEntrylist.size());
						 
						 //vector<Beopdatalink::CRealTimeDataEntry> tempInsertHisEntrylistCopy = tempInsertHisEntrylist;
						 //day data all save
						 
						 //save into redis
						/* DWORD tick1 = GetTickCount();
						 if(pthis->m_Redis.is_valid() && tempInsertHisEntrylistCopy.size()>0)
						 {
							 CString strNow = tnow.Format(_T("%Y-%m-%d 00:00:00"));
							 string strNow_utf8=  Project::Tools::WideCharToAnsi(strNow.GetString());
							 Json::Value pvAllRoot;
							 for(int i=0;i<tempInsertHisEntrylistCopy.size();i++)
							 {
								 wstring wstrVV = tempInsertHisEntrylistCopy[i].GetValue();
								 string strVV = Project::Tools::WideCharToAnsi(wstrVV.c_str());

								 string strPointName_utf8 = tempInsertHisEntrylistCopy[i].GetName();

								 CString strKeyName;
								 strKeyName.Format(_T("HIS##%s"),Project::Tools::AnsiToWideChar(strPointName_utf8.data()).c_str());

								 string strCurValue;
								 pthis->m_Redis.get_value_string(strKeyName.GetString(), strCurValue);

								 Json::Reader jsonReader;
								 Json::Value jsonRoot;
								 if(strCurValue=="")
								 {
									  jsonRoot[strNow_utf8] = strVV;
								 }
								 if(jsonReader.parse(strCurValue, jsonRoot))
								 {
									 if(jsonRoot.isObject())
									 {
										 jsonRoot[strNow_utf8] = strVV;
									 }
								 }
								 else
								 {
									 jsonRoot[strNow_utf8] = strVV;
								 }

								 string strAllValueNew = jsonRoot.toStyledString();
								 wstring wstrAllValueNew = Project::Tools::AnsiToWideChar(strAllValueNew.data());

								 pthis->m_Redis.set_value(strKeyName.GetString(), wstrAllValueNew);
							 }

						 }
						 int nCostTick = GetTickCount() - tick1;
						 */
					 }

					

					 //每天零点发送全部数据
					  if(!pthis->m_bFirstStart && !pthis->m_bDTUDisableSendAllData  && pthis->m_nSendAll == 0)
					 {					
						pthis->SendAllRealData(stNow);
					 }

					 if(pthis->m_nDTUSendMinType == (int)E_STORE_ONE_DAY)
					 {
						 pthis->GenerateDTURealData(stNow,E_STORE_ONE_DAY,pthis->m_nSendAll);
						 pthis->SetSendDTUEventAndType(0,stNow);
					 }
				 }

				 if(tnow.GetHour()!=pthis->m_tLastHistorySaved.GetHour() && pthis->m_nMinTCycle <=E_STORE_ONE_HOUR)
				 {			
					 if(pthis->m_bStoreHistory)
					 {
						 vector<Beopdatalink::CRealTimeDataEntry> tempInsertHisEntrylist;
						 pthis->GetRealTimeDataEntryList(tempInsertHisEntrylist, E_STORE_ONE_HOUR);
						 bool bTableCreated = pthis->CheckCreateHistoryTable(stNow, E_STORE_ONE_HOUR);
						 bool bNewMonth = (pthis->m_tLastHistorySaved.GetMonth()!=tnow.GetMonth());
						 if(!bNewMonth && !bTableCreated)
						 {
							 vector<Beopdatalink::CRealTimeDataEntry> tempNewInsertHisEntrylist;
							 pthis->CompareModifiedRealTimeDataEntryList(tempInsertHisEntrylist, pthis->m_RealTimeDataEntryListBufferMap_H1, tempNewInsertHisEntrylist);

							 pthis->UpdateHistoryTable(stNow, E_STORE_ONE_HOUR, tempNewInsertHisEntrylist,tempInsertHisEntrylist.size());
							 pthis->UpdateHistoryRedisTable(stNow, tempInsertHisEntrylist);//全集
							 //单独增加分钟数据存储!!!!!
							 //pthis->UpdateHistoryTable(stNow, E_STORE_ONE_MINUTE, tempInsertHisEntrylist,tempInsertHisEntrylist.size());
						 }
						 else
						 {
							 vector<Beopdatalink::CRealTimeDataEntry> tempNewInsertHisEntrylist;
							 pthis->CombineRealTimeDataEntryList(tempInsertHisEntrylist, pthis->m_RealTimeDataEntryListBufferMap_H1, tempNewInsertHisEntrylist);//合并，避免少了点


							 pthis->UpdateHistoryTable(stNow, E_STORE_ONE_HOUR, tempNewInsertHisEntrylist,tempNewInsertHisEntrylist.size());
							 pthis->UpdateHistoryRedisTable(stNow, tempNewInsertHisEntrylist);//全集

						 }

						 
					 }

					 if(pthis->m_nDTUSendMinType == (int)E_STORE_ONE_HOUR)
					 {
						 pthis->GenerateDTURealData(stNow,E_STORE_ONE_HOUR,pthis->m_nSendAll);
						 pthis->SetSendDTUEventAndType(0,stNow);
					 }
				 }

				 if (tnow.GetMinute()%5==0 && pthis->m_nMinTCycle <=E_STORE_FIVE_MINUTE)
				 {		
					 if(pthis->m_bStoreHistory)
					 {
						 vector<Beopdatalink::CRealTimeDataEntry> tempInsertHisEntrylist;
						 pthis->GetRealTimeDataEntryList(tempInsertHisEntrylist, E_STORE_FIVE_MINUTE);
						 bool bTableCreated = pthis->CheckCreateHistoryTable(stNow, E_STORE_FIVE_MINUTE);

						 bool bNewDay = (pthis->m_tLastHistorySaved.GetDay()!=tnow.GetDay());
						 if(!bNewDay && !bTableCreated)
						 {
							 vector<Beopdatalink::CRealTimeDataEntry> tempNewInsertHisEntrylist;
							 pthis->CompareModifiedRealTimeDataEntryList(tempInsertHisEntrylist, pthis->m_RealTimeDataEntryListBufferMap_M5, tempNewInsertHisEntrylist);
							 pthis->UpdateHistoryTable(stNow, E_STORE_FIVE_MINUTE, tempNewInsertHisEntrylist,tempInsertHisEntrylist.size());

						 }
						 else
						 {

							 vector<Beopdatalink::CRealTimeDataEntry> tempNewInsertHisEntrylist;
							 pthis->CombineRealTimeDataEntryList(tempInsertHisEntrylist, pthis->m_RealTimeDataEntryListBufferMap_M5, tempNewInsertHisEntrylist);//合并，避免少了点

							 pthis->UpdateHistoryTable(stNow, E_STORE_FIVE_MINUTE, tempNewInsertHisEntrylist,tempNewInsertHisEntrylist.size());
						 }

					 }

					 if(pthis->m_nDTUSendMinType == (int)E_STORE_FIVE_MINUTE)
					 {
						 pthis->GenerateDTURealData(stNow,E_STORE_FIVE_MINUTE,pthis->m_nSendAll);
						 pthis->SetSendDTUEventAndType(0,stNow);
					 }
				 }

				 if(pthis->m_nMinTCycle <= E_STORE_ONE_MINUTE)
				 {
					 if(pthis->m_bStoreHistory)
					 {
						 vector<Beopdatalink::CRealTimeDataEntry> tempInsertHisEntrylist;
						 pthis->GetRealTimeDataEntryList(tempInsertHisEntrylist, E_STORE_ONE_MINUTE);
						 bool bTableCreated = pthis->CheckCreateHistoryTable(stNow, E_STORE_ONE_MINUTE);
						 bool bNewDay = (pthis->m_tLastHistorySaved.GetDay()!=tnow.GetDay());
						 if(!bNewDay && !bTableCreated)
						 {
							 

							 vector<Beopdatalink::CRealTimeDataEntry> tempNewInsertHisEntrylist;
							 pthis->CompareModifiedRealTimeDataEntryList(tempInsertHisEntrylist, pthis->m_RealTimeDataEntryListBufferMap_M1, tempNewInsertHisEntrylist);
							 int nInsertCount = tempNewInsertHisEntrylist.size();
							 pthis->UpdateHistoryTable(stNow, E_STORE_ONE_MINUTE, tempNewInsertHisEntrylist, nInsertCount);
							 pthis->UpdateHistoryRedisTableRecentMinutes(stNow,tempInsertHisEntrylist );

							// pthis->UpdateHistoryRedisTable(stNow, tempInsertHisEntrylist);//全集测试，注意一定要删除

							 CString strTemp;
						     strTemp.Format(_T("[SYSTEM]Insert m1 count %d/Real:%d/Buffer:%d point data when running.\r\n"), nInsertCount, tempInsertHisEntrylist.size(), pthis->m_RealTimeDataEntryListBufferMap_M1.size());
							 pthis->SendLogMsg(_T("SYSTEM"), strTemp.GetString());
						 }
						 else
						 {

							 vector<Beopdatalink::CRealTimeDataEntry> tempNewInsertHisEntrylist;
							 pthis->CombineRealTimeDataEntryList(tempInsertHisEntrylist, pthis->m_RealTimeDataEntryListBufferMap_M1, tempNewInsertHisEntrylist);//合并，避免少了点

							 CString strTemp;
							 strTemp.Format(_T("[SYSTEM]Insert m1 count %d/Real:%d/Buffer:%d point data when day begin!!!!\r\n"), tempNewInsertHisEntrylist.size(),tempInsertHisEntrylist.size(), pthis->m_RealTimeDataEntryListBufferMap_M1.size());
							 pthis->SendLogMsg(_T("SYSTEM"), strTemp.GetString());
							 pthis->UpdateHistoryTable(stNow, E_STORE_ONE_MINUTE, tempNewInsertHisEntrylist,tempNewInsertHisEntrylist.size());
							 pthis->UpdateHistoryRedisTableRecentMinutes(stNow,tempNewInsertHisEntrylist );
						 }
					 }

					 if(pthis->m_nDTUSendMinType == (int)E_STORE_ONE_MINUTE)
					 {
						 pthis->GenerateDTURealData(stNow,E_STORE_ONE_MINUTE,pthis->m_nSendAll);
						 pthis->SetSendDTUEventAndType(0,stNow);
					 }
				 }

				pthis->m_tLastHistorySaved = tnow;
			 }
			 Sleep(1*1000);
			 nSecondCount++;
		 }
	 }

	 return 0;
 }

 bool CBEOPDataLinkManager::UpdatePointBuffer()
 {
	 if(!m_bFirstInitRedundancyBuffer)
	 {
		 if(m_dbsession_Redundancy != NULL && m_dbsession_Redundancy->IsConnected())
		 {

				 //初始同步PointBuffer
				 //vector<CRealTimeDataEntry> entrybufferlist;
				 //m_dbsession_history->ReadPointBufferData(entrybufferlist);

				 //m_dbsession_Redundancy->InsertPointBuffer(entrybufferlist);

				 //同步unit01

				 //同步unit01				
				 //string strHostIP = GetHostIP();
				 //InsertRedundancyUnit01(strHostIP);

				 m_bFirstInitRedundancyBuffer = true;
			 
		 }
	 }

	 vector<CRealTimeDataEntry> entrylist;
	 if(true)
	 {
		 Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock_vecMemoryPointChangeList);
		 if(m_vecMemoryPointChangeList.size()<=0)
			 return true;
		 entrylist = m_vecMemoryPointChangeList;
		 m_vecMemoryPointChangeList.clear();
	 }

	 bool bresult = m_dbsession_history->UpdatePointBuffer(entrylist);
	 if(m_dbsession_Redundancy != NULL)
	 {
		 if(m_dbsession_Redundancy->IsConnected())
		 {
			  m_dbsession_Redundancy->UpdatePointBuffer(entrylist);
		 }
	 }
	 return bresult;
 }

 bool CBEOPDataLinkManager::CheckCreateHistoryTable(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle)
 {
	return m_dbsession_history->CheckCreateHistoryTable(st, tCycle);
 }


 bool	CBEOPDataLinkManager::UpdateHistoryRedisTableRecentMinutes(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist)
 {
	 //	 GenerateEntryList(entrylist);
	 if (entrylist.empty()){
		 return true;
	 }


		//update redis
		CHttpOperation http(_T("127.0.0.1"), 5000, _T("redis/updateHistoryDataMinutes"));

		Json::Value myRoot,jsonPointNameList, jsonPointValueList;
		string strTime_utf8 = Project::Tools::SystemTimeToString(st);
		for(int i=0;i<entrylist.size();i++)
		{
			jsonPointNameList.append(Json::Value(entrylist[i].GetName()));
			string strValue_utf8 = Project::Tools::WideCharToUtf8(entrylist[i].GetValue().c_str());
			jsonPointValueList.append(Json::Value(strValue_utf8));
		}

		myRoot["time"]= strTime_utf8;
		myRoot["pointNameList"] = jsonPointNameList;
		myRoot["pointValueList"] = jsonPointValueList;

		string szBuf = myRoot.toStyledString();
		CString strResult;
		bool bRet = http.HttpPostGetRequestEx(0, szBuf.data(), strResult,  _T("Content-Type: application/json"));
		if(bRet)
		{
			/*Json::Reader jsReader;
			string strResult_utf8;
			Project::Tools::WideCharToUtf8(strResult.GetString(), strResult_utf8);
			Json::Value myReturnRoot;
			if (!jsReader.parse(strResult_utf8, myReturnRoot))
			{
			PrintLog(_T("updateHealthLog by dompysite failed\r\n"), true);
			}*/

			return true;
		}
		else
		{
			return false;
		}
	


	 return true;
 }

 bool	CBEOPDataLinkManager::UpdateHistoryRedisTable(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist)
 {
	 //	 GenerateEntryList(entrylist);
	 if (entrylist.empty()){
		 return true;
	 }

	 int nBatchCount = 5000;

		//update redis
		CHttpOperation http(_T("127.0.0.1"), 5000, _T("redis/updateHistoryData"));

		Json::Value myRoot,jsonPointNameList, jsonPointValueList;
		string strTime_utf8 = Project::Tools::SystemTimeToString(st);
		Json::Value jsonTime= Json::Value(strTime_utf8);
		for(int i=0;i<entrylist.size();i++)
		{
			jsonPointNameList.append(Json::Value(entrylist[i].GetName()));
			string strValue_utf8 = Project::Tools::WideCharToUtf8(entrylist[i].GetValue().c_str());
			jsonPointValueList.append(Json::Value(strValue_utf8));


			if(jsonPointNameList.size()>=nBatchCount || (i==(entrylist.size()-1) && jsonPointNameList.size()>0))
			{

				myRoot["time"]= jsonTime;
				myRoot["pointNameList"] = jsonPointNameList;
				myRoot["pointValueList"] = jsonPointValueList;

				string szBuf = myRoot.toStyledString();
				CString strResult;
				bool bRet = http.HttpPostGetRequestEx(0, szBuf.data(), strResult,  _T("Content-Type: application/json"));
				if(bRet)
				{
					
				}
				else
				{
				}

				jsonPointNameList.clear();
				jsonPointValueList.clear();
			}

		}

		


	 return true;
 }

 bool CBEOPDataLinkManager::UpdateHistoryTable(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nCount)
 {
	 //	 GenerateEntryList(entrylist);
	 if (entrylist.empty()){
		 return true;
	 }
	 CString strPeriod;
	 switch(tCycle)
	 {
	 case E_STORE_FIVE_SECOND:
		 strPeriod = L"5 second";
		 break;
	 case E_STORE_ONE_MINUTE:
		 strPeriod = L"1 mintue";
		 break;
	 case E_STORE_FIVE_MINUTE:
		 strPeriod = L"5 miute";
		 break;
	 case E_STORE_ONE_HOUR:
		 strPeriod = L"1 hour";
		 break;
	 case E_STORE_ONE_DAY:
		 strPeriod = L"1 day";
		 break;
	 case E_STORE_ONE_MONTH:
		 strPeriod = L"1 month";
		 break;
	 }

	 bool bresult = false;
	 if (m_dbsession_history)
	 {
		 //只保存修改过的点值


		 bresult = m_dbsession_history->InsertHistoryData(st, tCycle, entrylist,m_nMaxInsertLimit);

		 if (!bresult)
		 {
			 CString loginfo;
			 loginfo.Format(_T("ERROR: Update history data(%s) row: %d , %s!\r\n"), strPeriod,  entrylist.size(), (bresult ? _T("succeed"): _T("failed")));
			 SendLogMsg(L"History", loginfo.GetString());
		 }

		 
	 }

	 return bresult;
 }

 void CBEOPDataLinkManager::GenerateEntryList( vector<CRealTimeDataEntry>& resultlist )
 {
	 resultlist.clear();
	 // first, get all the data
	 vector<pair<wstring, wstring> >	valuesets;
	 GetOPCValueSets(valuesets);
	 GetModbusValueSets(valuesets);
	 GetFCbusValueSets(valuesets);
	 if (m_bacnetengine){
		 m_bacnetengine->GetValueSet(valuesets);
	 }
	 if (m_memoryengine){
		 m_memoryengine->GetValueSet(valuesets);
	 }

	 resultlist.reserve(valuesets.size());
	 SYSTEMTIME st;
	 GetLocalTime(&st);
	 CRealTimeDataEntry entry;
	 for (unsigned int i = 0; i < valuesets.size(); i++)
	 {
		 const pair<wstring, wstring>& pval = valuesets[i];
		 entry.SetTimestamp(st);
		 string ansiname = Project::Tools::WideCharToAnsi(pval.first.c_str());
		 entry.SetName(ansiname);
					
		 entry.SetValue( pval.second);
		 resultlist.push_back(entry);
	 }
 }

 //退出所有线程。
 void CBEOPDataLinkManager::ExitTAllThread()
 {
	 m_bexitthread_output = true;
	 m_bexitthread_input = true;
	 m_bexitthread_history = true;

	 if(m_hhistorydata)
		WaitForSingleObject(m_hhistorydata, INFINITE);

	 if(m_hhistoryS5data)
		WaitForSingleObject(m_hhistoryS5data, INFINITE);

	 if(m_hupdateinput)
		WaitForSingleObject(m_hupdateinput, INFINITE);

	 if(m_hsenddtuhandle)
	 {
		SetEvent(m_senddtuevent);
		WaitForSingleObject(m_hsenddtuhandle, INFINITE);
	 }

	 if(m_hReceivedtuhandle)
		 WaitForSingleObject(m_hReceivedtuhandle, INFINITE);

	 if(m_hscanwarninghandle)
		 WaitForSingleObject(m_hscanwarninghandle, INFINITE);

//	 WaitForSingleObject(m_hupdateoutput, INFINITE);
	 
	 //各个引擎退出  采取标准的Exit退出  Robert 20140807

	 //bacnet退出
	 if (m_bacnetengine){
		 m_bacnetengine->Exit();
	 }

	  map<wstring,CModbusEngine*>::iterator iter = m_mapMdobusEngine.begin();
	  while(iter != m_mapMdobusEngine.end())
	  {
		  iter->second->ExitThread();
		  iter++;
	  }

	  map<wstring,CCO3PEngine*>::iterator iterCO3P = m_mapCO3PEngine.begin();
	  while(iterCO3P != m_mapCO3PEngine.end())
	  {
		  iterCO3P->second->ExitThread();
		  iterCO3P++;
	  }

	  map<wstring,CFCBusEngine*>::iterator iterFC = m_mapFCBusEngine.begin();
	  while(iterFC != m_mapFCBusEngine.end())
	  {
		  iterFC->second->ExitThread();
		  iterFC++;
	  }
 }

 bool CBEOPDataLinkManager::GetExitThread_Input() const
 {
	 return m_bexitthread_input;
 }

 bool CBEOPDataLinkManager::GetExitThread_Output() const
 {
	 return m_bexitthread_output;
 }

 bool CBEOPDataLinkManager::GetExitThread_History() const
 {
	 return m_bexitthread_history;
 }

 void CBEOPDataLinkManager::SetHistoryDbCon( Beopdatalink::CCommonDBAccess* phisdbcon )
 {
	 m_dbsession_history = phisdbcon;
	 CDebugLog::GetInstance()->SetHistoryDbCon(m_dbsession_history);
 }

 void CBEOPDataLinkManager::AddLog( const wstring& loginfo )
 {
	 if (m_dbsession_log){
		 m_dbsession_log->InsertLog(loginfo);
	 }
 }

 void CBEOPDataLinkManager::SetLogDbCon( Beopdatalink::CLogDBAccess* plogdb )
 {
	 m_dbsession_log = plogdb;
 }

 bool CBEOPDataLinkManager::InitOPC( COPCEngine* opcengine ,int nOPCClientIndex)
 {
	 if (!opcengine){
		 return false;
	 }

#ifdef DEBUG
	 return opcengine->Init(nOPCClientIndex);
#else
	 int repeattime = 2;
	 while(repeattime--){
		 if (opcengine->Init(nOPCClientIndex)){
			 return true;
		 }else{
			 Sleep(5*1000);
		 }
	 }
#endif
	 return false;
}

 Beopdatalink::CLogDBAccess* CBEOPDataLinkManager::GetLogAccess()
 {
	 return m_dbsession_log;
 }

 void CBEOPDataLinkManager::GetOPCValueSets( vector<pair<wstring, wstring> >& opcvaluesets )
 {
	 if(m_bMutilOPCClientMode)
	 {
		 map<int,COPCEngine*>::iterator iter = m_mapOPCEngine.begin();
		 while(iter != m_mapOPCEngine.end())
		 {
			 //scan enable
			 vector<DataPointEntry> &ptList = iter->second->GetPointList();
			 for(int i=0;i<ptList.size();i++)
			 {
				 wstring wstrEquipment = ptList[i].GetEquipmentName();
				 map<wstring,Equipment>::iterator iter = m_mapEquipment.find(wstrEquipment);
				 if(iter!=m_mapEquipment.end())
				 {
					 ptList[i].Enable(IsEquipmentEnabled(iter->second));
				 }
				 else
				 {
					 ptList[i].Enable(true);
				 }

			 }

			 iter->second->GetValueSet(opcvaluesets);
			 iter++;
		 }
	 }
	 else
	 {
		 if (m_opcengine)
		 {
			 //scan enable
			 vector<DataPointEntry> &ptList = m_opcengine->GetPointList();
			 for(int i=0;i<ptList.size();i++)
			 {
				 wstring wstrEquipment = ptList[i].GetEquipmentName();
				 map<wstring,Equipment>::iterator iter = m_mapEquipment.find(wstrEquipment);
				 if(iter!=m_mapEquipment.end())
				 {
					 ptList[i].Enable(IsEquipmentEnabled(iter->second));
				 }
				 else
				 {
					 ptList[i].Enable(true);
				 }

			 }

			 m_opcengine->GetValueSet(opcvaluesets);
		 }
	 }
 }

 void CBEOPDataLinkManager::GetSiemensTCPValueSets(vector<pair<wstring, wstring> >& vSets)
 {
	 //if(m_pSiemensEngine)
		//m_pSiemensEngine->GetValueSet(vSets);

	 //20200323
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_Siemense_TCP(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsSiemens1200Point()|| pentry.IsSiemens300Point())
		 {
			 vSets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }

 bool    CBEOPDataLinkManager::IsEquipmentEnabled(Equipment &equip)
 {
	 if(equip.m_wstrEnableType==L"const")
	 {
		 int nEnableValue = _wtoi(equip.m_wstrEnableBindName.c_str());
		 return nEnableValue==1;

	 }
	 else if(equip.m_wstrEnableType==L"point")
	 {
		 bool bValue = true;
		 m_memoryengine->GetValue(equip.m_wstrEnableBindName, bValue);
		 return bValue;
	 }
	 else
	 {
		 return true;
	 }

 }


 void CBEOPDataLinkManager::GetModbusValueSets( vector<pair<wstring, wstring> >& modbusvaluesets )
 {

	 map<wstring,CModbusEngine*>::iterator iter = m_mapMdobusEngine.begin();
	 while(iter != m_mapMdobusEngine.end())
	 {
		 //scan enable
		 vector<DataPointEntry> &ptList = iter->second->GetPointList();
		 for(int i=0;i<ptList.size();i++)
		 {
			 wstring wstrEquipment = ptList[i].GetEquipmentName();
			 map<wstring,Equipment>::iterator iter = m_mapEquipment.find(wstrEquipment);
			 if(iter!=m_mapEquipment.end())
			 {
				 ptList[i].Enable(IsEquipmentEnabled(iter->second));
			 }
			 else
			 {
				 ptList[i].Enable(true);
			 }
			 
		 }

		 iter->second->GetValueSet(modbusvaluesets);
		 iter++;
	 }
 }

 void CBEOPDataLinkManager::GetCO3PValueSets( vector<pair<wstring, wstring> >& co3pvaluesets )
 {
	 map<wstring,CCO3PEngine*>::iterator iter = m_mapCO3PEngine.begin();
	 while(iter != m_mapCO3PEngine.end())
	 {
		 iter->second->GetValueSet(co3pvaluesets);
		 iter++;
	 }
 }

 void CBEOPDataLinkManager::GetProtocol104ValueSets( vector<pair<wstring, wstring> >& protocol104valuesets )
 {
	 map<wstring,CProtocol104Engine*>::iterator iter = m_mapProtocol104Engine.begin();
	 while(iter != m_mapProtocol104Engine.end())
	 {
		 iter->second->GetValueSet(protocol104valuesets);
		 iter++;
	 }
 }

 void CBEOPDataLinkManager::GetMysqlValueSets( vector<pair<wstring, wstring> >& mysqlvaluesets )
 {
	 map<wstring,CMysqlEngine*>::iterator iter = m_mapMysqlEngine.begin();
	 while(iter != m_mapMysqlEngine.end())
	 {
		 iter->second->GetValueSet(mysqlvaluesets);
		 iter++;
	 }
 }


 void	CBEOPDataLinkManager::GetCoreStationValueSets(vector<pair<wstring, wstring> >& coreStationvaluesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_CoreStation(tempAllList);

	 CString strLog;
	 strLog.Format(_T("GetCoreStationValueSets : %d count points\r\n"), tempAllList.size());
	 PrintLog(strLog.GetString());

	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsCoreStationPoint())
		 {
			 coreStationvaluesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }


 void	CBEOPDataLinkManager::GetKNXValueSets(vector<pair<wstring, wstring> >& KNXvaluesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_KNX(tempAllList);

	 CString strLog;
	 strLog.Format(_T("GetKNXValueSets : %d count points\r\n"), tempAllList.size());
	 PrintLog(strLog.GetString());

	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsKNXPoint())
		 {
			 KNXvaluesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }


 void	CBEOPDataLinkManager::GetDLT645ValueSets(vector<pair<wstring, wstring> >& DLT645valuesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_DLT645(tempAllList);

	 CString strLog;
	 strLog.Format(_T("GetDLT645ValueSets : %d count points\r\n"), tempAllList.size());
	 PrintLog(strLog.GetString());

	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 //CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsDLT645Point())
		 {
			 DLT645valuesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }

 void CBEOPDataLinkManager::GetWebValueSets( vector<pair<wstring, wstring> >& fcbusvaluesets )
 {
	 map<wstring,CWebEngine*>::iterator iter = m_mapWebEngine.begin();
	 while(iter != m_mapWebEngine.end())
	 {
		 iter->second->GetValueSet(fcbusvaluesets);
		 iter++;
	 }
 }

 bool CBEOPDataLinkManager::WriteOPCValue( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 bool bWriteOPCSuccess = false;			//写值状态
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 COPCEngine* pWriteOPCEngine = m_opcengine;		//对应OPC引擎句柄

	 if(m_bMutilOPCClientMode)						//多引擎模式
	 {
		 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);
		 map<int,COPCEngine*>::iterator iter = m_mapOPCEngine.find(pentry.GetOPClientIndex());
		 if(iter != m_mapOPCEngine.end())
		 {
			 pWriteOPCEngine = iter->second;		
		 }
	 }

	 if(pWriteOPCEngine)
	 {
		 bWriteOPCSuccess = pWriteOPCEngine->SetValue(pointname,entry.GetValue());		//异步写值（同步方式负荷过重)  成功后触发 OnWriteComplete	
	 }
	 return bWriteOPCSuccess;
 }

 bool CBEOPDataLinkManager::WriteModbusValue( const Beopdatalink::CRealTimeDataEntry& entry )
 {

	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);
	  map<wstring,CModbusEngine*>::iterator iter = m_mapMdobusEngine.find(pentry.GetServerAddress());
	  if(iter != m_mapMdobusEngine.end())
	  {
		  m_mapMdobusEngine[pentry.GetServerAddress()]->SetValue(pointname, _wtof(entry.GetValue().c_str()));
		  return true;
	  }

	 return false;
 }

 bool CBEOPDataLinkManager::WriteCO3PValue( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);
	 map<wstring,CCO3PEngine*>::iterator iter = m_mapCO3PEngine.find(pentry.GetParam(1));
	 if(iter != m_mapCO3PEngine.end())
	 {
		 m_mapCO3PEngine[pentry.GetParam(1)]->SetValue(pointname, _wtof(entry.GetValue().c_str()));
		 return true;
	 }

	 return false;
 }

 COPCEngine* CBEOPDataLinkManager::GetOPCEngine()
 {
	 return m_opcengine;
 }

 bool CBEOPDataLinkManager::WriteMemoryPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 if (m_memoryengine)
	 {
		 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
		 return m_memoryengine->SetValue(pointname, entry.GetValue());
	 }

	 return false;
 }

 bool CBEOPDataLinkManager::WriteBacnetPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 if (m_bacnetengine){
		 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
		 m_bacnetengine->SetValue(pointname, _wtof( entry.GetValue().c_str()));
		 return true;
	 }

	 return false;

 }

 bool CBEOPDataLinkManager::WriteSiemensTCPPoint(wstring strIP, const Beopdatalink::CRealTimeDataEntry& entry)
 {

	 //if (m_pSiemensEngine)
	 //{
		// wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
		// double dvalue = _wtof( entry.GetValue().c_str());
		// string svalue = Project::Tools::WideCharToAnsi(entry.GetValue().c_str());
		// float fvalue = atof(svalue.c_str());
		// return m_pSiemensEngine->SetValue(pointname, fvalue);

	 //}

	 //return false;

	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_Siemense_TCP(realtimeEntry);
	 }

	 return false;
 }
 /*
 double CBEOPDataLinkManager::GetValue( const wstring& pointname )
 {
	 DataPointEntry pointentry = m_pointmanager->GetEntry(pointname);
	 double result = 0.f;

	 //如果点位不存在
	 if (pointentry.GetShortName().empty()){
		 return 0.f;
	 }

	 if (pointentry.IsOpcPoint()){
		 if(m_bMutilOPCClientMode)
		 {
			 DataPointEntry entry = m_pointmanager->GetEntry(pointname);
			 map<int,COPCEngine*>::iterator iter = m_mapOPCEngine.find(entry.GetOPClientIndex());
			 if(iter != m_mapOPCEngine.end())
			 {
				 m_mapOPCEngine[entry.GetOPClientIndex()]->GetValue(pointname, result);
			 }
		 }
		 else
		 {
			 if (m_opcengine){
				 m_opcengine->GetValue(pointname, result);
			 }
		 }
	 }
	 else if (pointentry.IsModbusPoint()){

		 DataPointEntry entry = m_pointmanager->GetEntry(pointname);
		 map<wstring,CModbusEngine*>::iterator iter = m_mapMdobusEngine.find(entry.GetServerAddress());
		 if(iter != m_mapMdobusEngine.end())
		 {
			 m_mapMdobusEngine[entry.GetServerAddress()]->GetValue(pointname, result);
		 }

	 }
	 else if (pointentry.IsCO3PPoint()){

		 DataPointEntry entry = m_pointmanager->GetEntry(pointname);
		 map<wstring,CCO3PEngine*>::iterator iter = m_mapCO3PEngine.find(entry.GetParam(1));
		 if(iter != m_mapCO3PEngine.end())
		 {
			 m_mapCO3PEngine[entry.GetParam(1)]->GetValue(pointname, result);
		 }

	 }
	 else if (pointentry.IsFCbusPoint()){

		 DataPointEntry entry = m_pointmanager->GetEntry(pointname);
		 wstring strFC = entry.GetFCServerIP() + entry.GetFCServerPort();
		 map<wstring,CFCBusEngine*>::iterator iter = m_mapFCBusEngine.find(strFC);
		 if(iter != m_mapFCBusEngine.end())
		 {
			 m_mapFCBusEngine[strFC]->GetValue(pointname, result);
		 }

	 }
	 else if (pointentry.IsVPoint()){
		 if (m_memoryengine)
		 {
			 m_memoryengine->GetValue(pointname, result);
		 }
	 }
	 else if (pointentry.IsBacnetPoint())
	 {
		if (m_bacnetengine)
		{
			m_bacnetengine->GetValue(pointname, result);
		}
	 }
	 else if(pointentry.IsSiemens1200Point() || pointentry.IsSiemens300Point())
	 {
		 
	 }
		
	 return result;
 }*/

 void CBEOPDataLinkManager::InitPointsValueFromDB()
 {
	 vector<CRealTimeDataEntry>	entrylist;
	 m_dbsession_realtime->ReadRealtimeData_Input(entrylist);
	 for (unsigned int i = 0; i < entrylist.size(); i++){
		 wstring pointname = Project::Tools::AnsiToWideChar(entrylist[i].GetName().c_str());
		 DataPointEntry pointentry = m_pointmanager->GetEntry(pointname);
		 if(pointentry.IsBacnetPoint())
		 {
			 if(m_bacnetengine)
				m_bacnetengine->InitDataEntryValue(pointname, _wtof(entrylist[i].GetValue().c_str()));
		 }
		 else if(pointentry.IsOpcPoint())
		 {

		 }
		 else if(pointentry.IsModbusPoint())
		 {
			 map<wstring,CModbusEngine*>::iterator iter = m_mapMdobusEngine.begin();
			 while(iter != m_mapMdobusEngine.end())
			 {
				 CModbusEngine* pModbusEngine = iter->second;
				 pModbusEngine->InitDataEntryValue(pointname, _wtof(entrylist[i].GetValue().c_str()));
				 iter++;
			 }

		 }
		 else if(pointentry.IsCO3PPoint())
		 {
			
		 }
		 else if(pointentry.IsFCbusPoint())
		 {
			 map<wstring,CFCBusEngine*>::iterator iter = m_mapFCBusEngine.begin();
			 while(iter != m_mapFCBusEngine.end())
			 {
				 CFCBusEngine* pFCbusEngine = iter->second;
				 pFCbusEngine->InitDataEntryValue(pointname, _wtof(entrylist[i].GetValue().c_str()));
				 iter++;
			 }

		 }
		 else if (pointentry.IsVPoint())
		 {
			 if (m_memoryengine)
			 {
				 m_memoryengine->SetValue(pointname, entrylist[i].GetValue());
			 }
		 }

	 }

 }

 void CBEOPDataLinkManager::EnableLog(BOOL bEnable)
 {
	 m_bLog = bEnable;
 }

 BOOL CBEOPDataLinkManager::IsLogEnabled()
 {
	 return m_bLog;
 }

 bool CBEOPDataLinkManager::IsMasterMode()
 {
	 UpdateMainServerMode();
	 BeopDataEngineMode::EngineMode enginemode = GetEngineMode();
	 return (enginemode == BeopDataEngineMode::Mode_Master);
 }

 void CBEOPDataLinkManager::SetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist)
 {
	 EnterCriticalSection(&m_RealTimeData_CriticalSection);
	 m_RealTimeDataEntryList  = entrylist;
	 LeaveCriticalSection(&m_RealTimeData_CriticalSection);

 }


 /************************************************************************/
 /* 
 当前值列表：entryList
 缓存所有点的值：entrylistMapBuffer

 合并后存入的列表：newEntryList
 
 */
 /************************************************************************/
 void CBEOPDataLinkManager::CompareModifiedRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, map<string, wstring> &entrylistMapBuffer, vector<Beopdatalink::CRealTimeDataEntry> &newEntrylist)
 {
	 
	 for(int i =0; i<entrylist.size();i++)
	 {
		 std::map<string, wstring>::iterator iter = entrylistMapBuffer.find(entrylist[i].GetName());
		 if(iter!=entrylistMapBuffer.end())
		 {
			 wstring strEntryValue = entrylist[i].GetValue();
			 if(strEntryValue!= (*iter).second)
			 {
				  newEntrylist.push_back(entrylist[i]);
				  (*iter).second = strEntryValue;
			 }
		 }
		 else
		 {
			 newEntrylist.push_back(entrylist[i]);
			 entrylistMapBuffer[entrylist[i].GetName()] =entrylist[i].GetValue();
		 }
	 }
	

 }


 void CBEOPDataLinkManager::CombineRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, map<string, wstring> &entrylistMapBuffer, vector<Beopdatalink::CRealTimeDataEntry> &newEntrylist)
 {

	 for(int i =0; i<entrylist.size();i++)
	 {
		 std::map<string, wstring>::iterator iter = entrylistMapBuffer.find(entrylist[i].GetName());
		 if(iter!=entrylistMapBuffer.end())
		 {
			 wstring strEntryValue = entrylist[i].GetValue();
			 if(strEntryValue!= (*iter).second)
			 {
				 (*iter).second = strEntryValue;
			 }
		 }
		 else
		 {
			 entrylistMapBuffer[entrylist[i].GetName()] =entrylist[i].GetValue();
		 }
	 }

	 //合并后转换
	  std::map<string, wstring>::iterator iter = entrylistMapBuffer.begin();


	  SYSTEMTIME sNow;
	  GetLocalTime(&sNow);

	  while(iter != entrylistMapBuffer.end())
	  {
		  CRealTimeDataEntry ee;
		  ee.SetName(iter->first);
		  ee.SetValue(iter->second);
		  ee.SetTimestamp(sNow);

		  newEntrylist.push_back(ee);
		  iter++;
	  }
 }


 void CBEOPDataLinkManager::GetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist)
 {
	 entrylist.clear();
	 EnterCriticalSection(&m_RealTimeData_CriticalSection);
	 entrylist = m_RealTimeDataEntryList;
	 LeaveCriticalSection(&m_RealTimeData_CriticalSection);
 }

 void CBEOPDataLinkManager::GetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, const POINT_STORE_CYCLE & tCycle)
 {
	 entrylist.clear();
	 EnterCriticalSection(&m_RealTimeData_CriticalSection);
	 for(int i =0;i<m_RealTimeDataEntryList.size();i++)
	 {
		 POINT_STORE_CYCLE ptCycle = m_pointmanager->GetPointSavePeriod(m_RealTimeDataEntryList[i].GetName());
		 if(ptCycle!=E_STORE_NULL &&ptCycle<=tCycle)
		 {
			entrylist.push_back(m_RealTimeDataEntryList[i]);
		 }
	 }
	 LeaveCriticalSection(&m_RealTimeData_CriticalSection);
 }

 void CBEOPDataLinkManager::SetEngineMode( int modevalue )
 {
	  Project::Tools::Scoped_Lock<Mutex> guardlock(m_enginemode_lock);
	 m_enginemode = (BeopDataEngineMode::EngineMode)modevalue;
 }

 BeopDataEngineMode::EngineMode CBEOPDataLinkManager::GetEngineMode()
 {
	 Project::Tools::Scoped_Lock<Mutex> guardlock(m_enginemode_lock);
	 return m_enginemode;
 }
 void CBEOPDataLinkManager::UpdateMainServerMode()
 {

 }

 void CBEOPDataLinkManager::UpdateRealTimeDataEntryList( vector<Beopdatalink::CRealTimeDataEntry> &entrylist )
 {
	 if (!IsMasterMode())
	 {
		 return;
	 }

	 bool bresult = m_dbsession_realtime->UpdateRealTimeDatas_Input(entrylist);

	 //loginfoxx.Format(_T(""));
	 CString loginfo;

	 //失败时记载
	 if(!bresult)
	 {
		 loginfo.Format(_T("ERROR: DataEngine Update points(total nums: %d) To Table Failed!"), entrylist.size());
		 SendLogMsg(L"RealtimeInput", loginfo.GetString());

		 if(m_bLog)
			 AddLog(loginfo.GetString());
	 }
	 else
	 {
		
		 loginfo.Format(_T("INFO : Update points(total nums: %d) To RealtimeInputTable"), entrylist.size());
		 SendLogMsg(L"RealtimeInput", loginfo.GetString());		 
	 }
 }

 void CBEOPDataLinkManager::UpdateOutputParamsByList( vector<Beopdatalink::CRealTimeDataEntry> &entrylist )
 {
	m_dbsession_realtime->StartTransaction();
	for(int i=0; i<entrylist.size(); i++)
	{
		bool bresult = m_dbsession_realtime->UpdatePointData(entrylist[i]);
	}
	m_dbsession_realtime->Commit();
 }

 string CBEOPDataLinkManager::BuildDTUSendData(int nPointStoreType, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nCount)
 {
	// Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
	 if (entrylist.empty()){
		 return "";
	 }

	 string m_sendbuffer;
	 std::ostringstream sqlstream;
	 //pointtype,pointname,pointvalue;
	 for (unsigned int i = 0; i < entrylist.size(); i++)
	 {
		 string strvalue;
		 Project::Tools::WideCharToUtf8(entrylist[i].GetValue(),strvalue);
		 sqlstream << nPointStoreType << "," << entrylist[i].GetName() << "," << strvalue << ";";
	 }
	 sqlstream << GenerateDTUUpdateInfo(nPointStoreType,entrylist.size(),nCount);
	 m_sendbuffer = sqlstream.str();
	 return m_sendbuffer;
 }

 void CBEOPDataLinkManager::SetRedundancyDbCon( Beopdatalink::CCommonDBAccess* phisdbcon )
 {
	 m_dbsession_Redundancy = phisdbcon;
 }

 bool CBEOPDataLinkManager::InsertRedundancyInput()
 {
	 if(m_dbsession_Redundancy != NULL)
	 {
		 if(m_dbsession_Redundancy->IsConnected())
		 {
			 vector<Beopdatalink::CRealTimeDataEntry> entrylist;

			 GetRealTimeDataEntryList(entrylist);

			 if (entrylist.empty()){
				 return false;
			 }

			 bool bRedun = m_dbsession_Redundancy->InsertRealTimeDatas_Input(entrylist);
			 if(!bRedun)
			 {
				 //_tprintf(L"ERROR: Redundancy Failed!\r\n");
				 SendLogMsg(L"Redundancy",L"ERROR: Redundancy Failed!\r\n");
			 }
		 }		
	 }
	 return true;
 }

 unsigned int WINAPI CBEOPDataLinkManager::SendDTUThread( LPVOID lpVoid )
 {
	 CBEOPDataLinkManager* pDTUSendComm = reinterpret_cast<CBEOPDataLinkManager*>(lpVoid);
	 if (NULL == pDTUSendComm)
	 {
		 return 0;
	 }

	 int nWaitDTU = 60;			//等待60秒，DTU重启
	 while(pDTUSendComm->m_bDTUChecked && !pDTUSendComm->GetExitThread_History())
	 {
		 if(nWaitDTU>=0)
		 {
			 nWaitDTU--;
			 Sleep(1000);
		 }
		 else
		 {
			 break;
		 }
	 }
	 
	 while(!pDTUSendComm->GetExitThread_History())
	 {
		 // send Data
		 DWORD waitstatus = WaitForSingleObject(pDTUSendComm->GetSendDTUEvent(), INFINITE);
		 if (waitstatus == WAIT_OBJECT_0)
		 {
			//发送DTU
			 pDTUSendComm->SendDTUData();					   			
		 }

	 }

	 return 1;
 }

 HANDLE CBEOPDataLinkManager::GetSendDTUEvent()
 {
	 return m_senddtuevent;
 }

 bool CBEOPDataLinkManager::SendDTUData()
 {	
	 Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_lock);
	 bool bResult = true;
	 //发送DTU数据
 	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 if(m_vecSendBuffer.size() <= 0)
		 {
			 if(m_pdtusender->GetSendIdle())
			 {
				 //发送一个历史丢失文件
				SendOneLostHistoryData();
				SYSTEMTIME sNow;
				GetLocalTime(&sNow);
				SetSendDTUEventAndType(0,sNow);
			 }
			 return bResult;
		 }
		 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Run SendDTUData \n");
		 //将全局发送vec拷贝到临时变量中
		 m_bVecCopying = true;
		 vector<DTUSendInfo> vecDTU;
		 vecDTU = m_vecSendBuffer;
		 m_vecSendBuffer.clear();
		 m_bVecCopying = false;

		 bool bSendDTUState = true;

		 if(m_strAck.length()>0)
		 {
			 bSendDTUState = m_pdtusender->SendCmdState(m_ackTime,3,4,m_strAck);
			 m_strAck = "";

			 CString strSendInfo;
			 if(bSendDTUState)
			 {
				 strSendInfo = L"INFO : Ack DTUServer Change Values Cmd.\r\n";
			 }
			 else
			 {
				 strSendInfo = L"ERROR: Ack DTUServer Change Values Cmd.\r\n";
				 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_CHANGE_VALUE,true);
			 }
			 SendLogMsg(L"DTU",strSendInfo);

			 Sleep(1000);
		 }

		 if(m_strSendRealFilePath.length()>0)
		 {
			 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Run SendDTUData->SendRealDataFile(Resend) \n");
			 bSendDTUState = m_pdtusender->SendRealDataFile(m_strSendRealFilePath,m_nSendRealFileCount);
			 m_nSendRealFileCount = 0;
			 m_strSendRealFilePath = "";
			 Sleep(1000);
		 }

		 for(int i=0; i<vecDTU.size(); ++i)
		 {
			 //立即发送
			 if(m_strSendRealFilePath.length()>0)
			 {
				 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Run SendDTUData->SendRealDataFile(Resend) \n");
				 bSendDTUState = m_pdtusender->SendRealDataFile(m_strSendRealFilePath,m_nSendRealFileCount);
				 m_nSendRealFileCount = 0;
				 m_strSendRealFilePath = "";
				 Sleep(1000);
			 }

			 DTUSendInfo sDTU = vecDTU[i];
			 CString strDebugInfo;
			 strDebugInfo.Format(_T("DEBUG:Run SendDTUData->SendData(Type:%d,SubType:%d).\n"),sDTU.nType,sDTU.nSubType);
			 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,strDebugInfo.GetString());
			 if(sDTU.nType == 0)		//实时数据
			 {
				 if(sDTU.strSendBuf.size()>0)
				 {
					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->SendData(sDTU.tSendTime,sDTU.strSendBuf);
						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo.Format(L"SEND : DTU Send RealData Buffer(%d) Successed.\r\n", sDTU.strSendBuf.size());
						 }
						 else
						 {
							 strSendInfo.Format(L"ERROR: DTU Send RealData Buffer(%d) Failed.\r\n", sDTU.strSendBuf.size());
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_SEND_REALDATA,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }				
				 }

				 if(sDTU.nSubType == 1)			//同步
				 {
					 Sleep(1000);

					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,0,1,sDTU.strPacketIndex);

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo = L"SEND : Ack DTUServer Syn Data Cmd.\r\n";
						 }
						 else
						 {
							 strSendInfo = L"ERROR: Ack DTUServer Syn Data Cmd.\r\n";
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_SYN_DATA,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }
				 }
				 else if(sDTU.nSubType == 2)		//修改
				 {
					 Sleep(1000);

					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,0,2,sDTU.strPacketIndex);

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo = L"SEND : Ack DTUServer Change Data Cmd.\r\n";
						 }
						 else
						 {
							 strSendInfo = L"ERROR: Ack DTUServer Change Data Cmd.\r\n";
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_CHANGE_DATA,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }
				 }	
				 else if(sDTU.nSubType == 3)		//重发
				 {
					 Sleep(1000);

					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,0,3,sDTU.strPacketIndex);

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo = L"SEND : Ack DTUServer ReSend Lost Data Cmd.\r\n";
						 }
						 else
						 {
							 strSendInfo = L"ERROR: Ack DTUServer ReSend Lost Data Cmd.\r\n";
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_LOST_DATA,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }
				 }	
			 }
			 else if(sDTU.nType == 1)	//报警数据
			 {
				 if(sDTU.strSendBuf.size()>0)
				 {
					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->SendWarningData(sDTU.tSendTime,sDTU.strSendBuf);

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo.Format(L"SEND : DTU Send WarningData Buffer(%d) Successed.\r\n", sDTU.strSendBuf.size());
						 }
						 else
						 {
							 strSendInfo.Format(L"ERROR: DTU Send WarningData Buffer(%d) Failed.\r\n", sDTU.strSendBuf.size());
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_SEND_WARNING_DATA,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }				
				 }
			 }
			 else if(sDTU.nType == 2)		//报警操作
			 {
				 if(sDTU.strSendBuf.size()>0)
				 {
					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->SendWarnigOperationData(sDTU.tSendTime,sDTU.strSendBuf);

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo.Format(L"SEND : DTU Send WarningOperation Buffer(%d) Successed.\r\n", sDTU.strSendBuf.size());
						 }
						 else
						 {
							 strSendInfo.Format(L"ERROR: DTU Send WarningOperation Buffer(%d) Failed.\r\n", sDTU.strSendBuf.size());
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_SEND_WARNIN_GOPERATION,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }					
				 }
			 }
			 else if(sDTU.nType == 9)		//用户操作
			 {
				 if(sDTU.strSendBuf.size()>0)
				 {
					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->SendUserOperationData(sDTU.tSendTime,sDTU.strSendBuf);

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo.Format(L"SEND : DTU Send UserOperation Buffer(%d) Successed.\r\n", sDTU.strSendBuf.size());
						 }
						 else
						 {
							 strSendInfo.Format(L"ERROR: DTU Send UserOperation Buffer(%d) Failed.\r\n", sDTU.strSendBuf.size());
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_SEND_USER_OPERATION,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }					
				 }
			 }
			 else if(sDTU.nType == 3)		//Unit01
			 {
				 if(sDTU.strSendBuf.size()>0)
				 {
					 bSendDTUState = m_pdtusender->SendUnit01(sDTU.tSendTime,sDTU.strSendBuf);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo.Format(L"SEND : DTU Send Unit01 Buffer(%d) Successed.\r\n", sDTU.strSendBuf.size());
					 }
					 else
					 {
						 strSendInfo.Format(L"ERROR: DTU Send Unit01 Buffer(%d) Failed.\r\n", sDTU.strSendBuf.size());
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_SEND_UNIT,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }

				 if(sDTU.nSubType == 1)			//同步
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,1,sDTU.strPacketIndex);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer Syn Unit01 Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer Syn Unit01 Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_SYN_UNIT,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
				 else if(sDTU.nSubType == 2)		//修改
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,2,sDTU.strPacketIndex);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer Edit Unit01 Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer Edit Unit01 Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_EDIT_UNIT,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
				 else if(sDTU.nSubType == 3)		//restart
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,3,sDTU.strPacketIndex);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer Reatart Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer Reatart Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_RESTART,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
				 else if(sDTU.nSubType == 4)		//restart
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,4,sDTU.strPacketIndex);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer Reatart Watch Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer Reatart Watch Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_RESTART_WATCH,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
				 else if(sDTU.nSubType == 5)		//sql命令
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,5,sDTU.strPacketIndex);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer Execute Sql Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer Execute Sql Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_SQL,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
				 else if(sDTU.nSubType == 6)		//心跳包相应
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendHeartBeat(sDTU.tSendTime);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer HeartBeat Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer HeartBeat Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_HEARTBEAT,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
				 else if(sDTU.nSubType == 7)		//start
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCoreStart(Project::Tools::WideCharToAnsi(m_strTCPName.c_str()));
				 }
				 else if(sDTU.nSubType == 8)		//restart dtu
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->RestartDTU();
					 if(bSendDTUState)
					 {
						 Sleep(5000);
						 m_pdtusender->SetDTUToRTU(false);
						 Sleep(25000);
						 bSendDTUState = m_pdtusender->SendCoreStart(Project::Tools::WideCharToAnsi(m_strTCPName.c_str()));

						 Sleep(1000);

						 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,8,"1");

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo = L"SEND : Ack DTUServer ReStart DTU Cmd Success.\r\n";
						 }
						 else
						 {
							 strSendInfo = L"ERROR: Ack DTUServer ReStart DTU Cmd Success.\r\n";
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_RESTART_DTU,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }
					 else
					 {
						 Sleep(5000);
						 m_pdtusender->SetDTUToRTU(false);

						 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,8,"0");

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo = L"SEND : Ack DTUServer ReStart DTU Cmd Fail.\r\n";
						 }
						 else
						 {
							 strSendInfo = L"ERROR: Ack DTUServer ReStart DTU Cmd Fail.\r\n";
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_RESTART_DTU,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }
				 }
				 else if(sDTU.nSubType == 9)		//同步现场时钟
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,9,sDTU.strPacketIndex);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer Syn SystemTime Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer Syn SystemTime Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_SYN_TIME,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
				 else if(sDTU.nSubType == 10)		//同步现场时钟
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,10,sDTU.strPacketIndex);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer Query Version Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer Query Version Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_VERSION,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
				 else if(sDTU.nSubType == 11)		//restart
				 {
					 Sleep(1000);

					 bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,11,sDTU.strPacketIndex);

					 CString strSendInfo;
					 if(bSendDTUState)
					 {
						 strSendInfo = L"SEND : Ack DTUServer Reatart Logic Cmd.\r\n";
					 }
					 else
					 {
						 strSendInfo = L"ERROR: Ack DTUServer Reatart Logic Cmd.\r\n";
						 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_ACK_RESTART_LOGIC,true);
					 }
					 SendLogMsg(L"DTU",strSendInfo);
				 }
			 }
			 else if(sDTU.nType == 4)		//ExecuteLog
			 {
				 if(sDTU.strSendBuf.size()>0)
				 {
					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->SendExecuteLog(sDTU.tSendTime,sDTU.strSendBuf);

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo.Format(L"SEND : DTU Send ExecuteLog Buffer(%d) Successed.\r\n", sDTU.strSendBuf.size());
						 }
						 else
						 {
							 strSendInfo.Format(L"ERROR: DTU Send ExecuteLog Buffer(%d) Failed.\r\n", sDTU.strSendBuf.size());
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_SEND_EXECUTE_LOG,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }
				 }
			 }
			 else if(sDTU.nType == 5)		//ExecuteLog
			 {
				 if(sDTU.strSendBuf.size()>0)
				 {
					 if(m_bdtuSuccess)
					 {
						 bSendDTUState = m_pdtusender->ReSendRealData(sDTU.strSendBuf);

						 CString strSendInfo;
						 if(bSendDTUState)
						 {
							 strSendInfo.Format(L"SEND : DTU ReSend RealData(%s) Successed.\r\n", Project::Tools::AnsiToWideChar(sDTU.strSendBuf.c_str()).c_str());
						 }
						 else
						 {
							 strSendInfo.Format(L"ERROR: DTU ReSend RealData(%s) Failed.\r\n", Project::Tools::AnsiToWideChar(sDTU.strSendBuf.c_str()).c_str());
							 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_RESEND_REALDATA,true);
						 }
						 SendLogMsg(L"DTU",strSendInfo);
					 }
				 }
			 }
			 else if(sDTU.nType == 6)		//History
			 {
				 Sleep(1000);		

				 bSendDTUState = m_pdtusender->SendFile(sDTU.strSendBuf);
				 CString strSendInfo;
				 if(bSendDTUState)
				 {
					 strSendInfo = L"SEND : Send History File.\r\n";
				 }
				 else
				 {
					 strSendInfo = L"ERROR: Send History File.\r\n";
					 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_SEND_HISTORY,true);
				 }
				 SendLogMsg(L"DTU",strSendInfo);
			 }
			 else if(sDTU.nType == 7)		//realdata file
			 {
				 Sleep(1000);		

				 bSendDTUState = m_pdtusender->SendRealDataFile(sDTU.strSendBuf,sDTU.nPointCount);
				 CString strSendInfo;
				 wstring strTime;
				 Project::Tools::SysTime_To_String(sDTU.tSendTime,strTime);
				 if(bSendDTUState)
				 {
					 strSendInfo.Format(_T("SEND : Send RealData File(%s).\r\n"),strTime.c_str());
				 }
				 else
				 {
					 strSendInfo.Format(_T("ERROR: Send RealData File(%s).\r\n"),strTime.c_str());
					 CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_SEND_REALFILE,true);
				 }
				 SendLogMsg(L"DTU",strSendInfo);
			 }
			 else if(sDTU.nType == 8)		//exe更新回执
			 {
				 Sleep(1000);

				 bSendDTUState = m_pdtusender->SendUpdateFileAck(sDTU.nSubType,sDTU.strSendBuf);
			 }
			 Sleep(1000);			//DTU 延迟1s的发送间隔  
		 }

		 //发送丢失队列
		 SYSTEMTIME sNow;
		 GetLocalTime(&sNow);
		 SetSendDTUEventAndType(0,sNow);
		 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Exit SendDTUData \n");
	 }
	
	 return bResult;
 }

 bool CBEOPDataLinkManager::InsertRedundancyUnit01(string strRedundancyIP)
 {
	 if(m_dbsession_Redundancy != NULL)
	 {
		 if(m_dbsession_Redundancy->IsConnected())
		 {
			 vector<unitStateInfo> unitlist;

			 m_dbsession_history->ReadRealTimeData_Unit01(unitlist);

			 if(unitlist.empty())
			 {
				 return false;
			 }

			 m_dbsession_Redundancy->InsertUnit01(unitlist,strRedundancyIP);
		 }		
	 }
	 return true;
 }

 string CBEOPDataLinkManager::GetHostIP()
 {
	 WORD wVersionRequested = MAKEWORD(2, 2);   
	 WSADATA wsaData;   
	 if (WSAStartup(wVersionRequested, &wsaData) != 0)   
		 return "";   
	 char local[255] = {0};   
	 gethostname(local, sizeof(local));   
	 hostent* ph = gethostbyname(local);   
	 if (ph == NULL)   
		 return "0";   
	 in_addr addr;   
	 memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr));   
	 std::string localIP;   
	 localIP.assign(inet_ntoa(addr));   
	 WSACleanup();   
	 return localIP;   
 }

 unsigned int WINAPI CBEOPDataLinkManager::ScanWarningAndOperation( LPVOID lpVoid )
 {
	 CBEOPDataLinkManager* pScanWarningComm = reinterpret_cast<CBEOPDataLinkManager*>(lpVoid);
	 if (NULL == pScanWarningComm)
	 {
		 return 0;
	 }
	 int nScanUserOperation = 5;		//5分钟扫描一次
	 while(!pScanWarningComm->GetExitThread_History())
	 {
		 nScanUserOperation--;
		 // 扫描报警表
		 pScanWarningComm->ScanWarning();

		 Sleep(1000);			//延迟1s,防止DTU阻塞

		 pScanWarningComm->ScanOperation();

		 Sleep(1000);

		 if(nScanUserOperation == 0)
		 {
			 pScanWarningComm->ScanUserOperation();
			 nScanUserOperation = 5;
		 }

		 int nWaitSleep = scan_warning_interval;
		 while(!pScanWarningComm->GetExitThread_History())
		 {
			 if(nWaitSleep>=0)
			 {
				 nWaitSleep--;
				 Sleep(1000);	
			 }
			 else
			 {
				 break;
			 }
		 }
	 }

	 return 1;
 }

 bool CBEOPDataLinkManager::SetSendDTUEventAndType( int nDTUDataType , SYSTEMTIME tSendDTUTime)
 {
	 SetEvent(m_senddtuevent);
	// CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"Run SetSendDTUEventAndType \n");
	 return true;
 }

 bool CBEOPDataLinkManager::ScanWarning()
 {
	 m_strSendWarningBuf = "";
	 vector<CWarningEntry> resultList;
	 if(m_dbsession_history->ReadRecentWarning(resultList,scan_warning_interval))
	 {
		 std::ostringstream sqlstream;
		 //time,level,info,pointname,confirmed;
		for(int i=0; i<resultList.size(); ++i)	
		{			
			sqlstream << Project::Tools::SystemTimeToString(resultList[i].GetTimestamp()) << ","  << resultList[i].GetWarningLevel() << "," << Project::Tools::WideCharToUtf8(resultList[i].GetWarningInfo().c_str())
				<< "," << Project::Tools::WideCharToUtf8(resultList[i].GetWarningPointName().c_str()) << "," << resultList[i].GetWarningConfirmedType() << ";";			
		}
		m_strSendWarningBuf = sqlstream.str();

		if(m_bRunDTUEngine)		//Json格式存入数据库
		{
			SYSTEMTIME stNow;
			GetLocalTime(&stNow);
			string strTime = Project::Tools::SystemTimeToString(stNow);

			Json::Value jsonBody;
			jsonBody["id"] = "";
			jsonBody["type"] = DTU_CMD_WARNING_DATA_SEND;

			std::ostringstream sqlstream;
			sqlstream << "wdp:time:" << strTime << ";" << m_strSendWarningBuf;
			jsonBody["content"] = sqlstream.str().c_str();

			DTU_DATA_INFO data;
			data.strTime = strTime;
			data.strContent = jsonBody.toStyledString();

			CString strLog;
			strLog.Format(_T("INFO : Save Warning(%d).\r\n"),data.strContent.length());
			PrintLog(strLog.GetString());
			return m_dbsession_history->SaveOneSendData(data);
		}
		else
		{
			if(m_bdtuSuccess)
			{
				SYSTEMTIME st;
				GetLocalTime(&st);

				DTUSendInfo sDTUInfo;
				sDTUInfo.tSendTime = st;
				sDTUInfo.nType = 1;
				sDTUInfo.strSendBuf = m_strSendWarningBuf;
				if(!m_bVecCopying)
					m_vecSendBuffer.push_back(sDTUInfo);

				SetSendDTUEventAndType(1,st);
			}
		}
	 }
	 return true;
 }

 bool CBEOPDataLinkManager::ScanOperation()
 {
	 m_strSendOperationBuf = "";
	 vector<WarningOperation> resultList;
	 if(m_dbsession_history->ReadRecentOperation(resultList,scan_warning_interval))
	 {
		 //info pointname user operation
		 for(int i=0; i<resultList.size(); ++i)
		 {
			 m_strSendOperationBuf += Project::Tools::WideCharToUtf8(resultList[i].strWarningInfo.c_str());
			 m_strSendOperationBuf += ",";
			 m_strSendOperationBuf += Project::Tools::WideCharToUtf8(resultList[i].strBindPointName.c_str());
			 m_strSendOperationBuf += ",";
			 m_strSendOperationBuf += Project::Tools::WideCharToUtf8(resultList[i].strUser.c_str());
			 m_strSendOperationBuf += ",";
			 m_strSendOperationBuf += Project::Tools::WideCharToUtf8(resultList[i].strOpeartion.c_str());
			 m_strSendOperationBuf += ";";
		 }

		 if(m_bRunDTUEngine)		//Json格式存入数据库
		 {
			 SYSTEMTIME stNow;
			 GetLocalTime(&stNow);
			 string strTime = Project::Tools::SystemTimeToString(stNow);

			 Json::Value jsonBody;
			 jsonBody["id"] = "";
			 jsonBody["type"] = DTU_CMD_WARNING_OPERATION_DATA_SEND;

			 std::ostringstream sqlstream;
			 sqlstream << "wop:time:" << strTime << ";" << m_strSendOperationBuf;
			 jsonBody["content"] = sqlstream.str().c_str();

			 DTU_DATA_INFO data;
			 data.strTime = strTime;
			 data.strContent = jsonBody.toStyledString();

			 CString strLog;
			 strLog.Format(_T("INFO : Save Warning Operation(%d).\r\n"),data.strContent.length());
			 PrintLog(strLog.GetString());
			 return m_dbsession_history->SaveOneSendData(data);
		 }
		 else
		 {
			 if(m_bdtuSuccess)
			 {
				 SYSTEMTIME st;
				 GetLocalTime(&st);

				 DTUSendInfo sDTUInfo;
				 sDTUInfo.tSendTime = st;
				 sDTUInfo.nType = 2;
				 sDTUInfo.strSendBuf = m_strSendOperationBuf;
				 if(!m_bVecCopying)
					 m_vecSendBuffer.push_back(sDTUInfo);

				 SetSendDTUEventAndType(2,st);
			 }
		 }
	 }
	 return true;
 }

 bool CBEOPDataLinkManager::ScanUserOperation()
 {
	 m_strSendUserOperationBuf = "";

	 if(m_strLastSendOperation == L"")
	 {
		 m_strLastSendOperation = m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"lastoperationtime");
		 if(m_strLastSendOperation == L"")
		 {
			 COleDateTime oleLastTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0,0,6,0);
			 Project::Tools::OleTime_To_String(oleLastTime,m_strLastSendOperation);
			 m_dbsession_history->WriteCoreDebugItemValue(L"lastoperationtime",m_strLastSendOperation.c_str());
		 }
	 }

	 vector<UserOperation> resultList;
	 if(m_dbsession_history->ReadRecentUserOperation(resultList,m_strLastSendOperation))
	 {
		 //info pointname user operation
		 for(int i=0; i<resultList.size(); ++i)
		 {
			 if( i == 0 )
			 {
				m_strLastSendOperation = resultList[i].strTime;
				m_dbsession_history->WriteCoreDebugItemValue(L"lastoperationtime",m_strLastSendOperation.c_str());
			 }
			 m_strSendUserOperationBuf += Project::Tools::WideCharToUtf8(resultList[i].strTime.c_str());
			 m_strSendUserOperationBuf += "|";
			 m_strSendUserOperationBuf += Project::Tools::WideCharToUtf8(resultList[i].strUser.c_str());
			 m_strSendUserOperationBuf += "|";
			 m_strSendUserOperationBuf += Project::Tools::WideCharToUtf8(resultList[i].strOpeartion.c_str());
			 m_strSendUserOperationBuf += ";";
		 }

		 if(m_bRunDTUEngine)		//Json格式存入数据库
		 {
			 SYSTEMTIME stNow;
			 GetLocalTime(&stNow);
			 string strTime = Project::Tools::SystemTimeToString(stNow);

			 Json::Value jsonBody;
			 jsonBody["id"] = "";
			 jsonBody["type"] = DTU_CMD_OPERATION_SEND;
			 std::ostringstream sqlstream;
			 sqlstream << "orp:time:" << strTime << ";" << m_strSendUserOperationBuf;
			 jsonBody["content"] = sqlstream.str().c_str();

			 DTU_DATA_INFO data;
			 data.strTime = strTime;
			 data.strContent = jsonBody.toStyledString();

			 CString strLog;
			 strLog.Format(_T("INFO : Save User Operation(%d).\r\n"),data.strContent.length());
			 PrintLog(strLog.GetString());
			 return m_dbsession_history->SaveOneSendData(data);
		 }
		 else
		 {
			 if(m_bdtuSuccess)
			 {
				 SYSTEMTIME st;
				 GetLocalTime(&st);

				 DTUSendInfo sDTUInfo;
				 sDTUInfo.tSendTime = st;
				 sDTUInfo.nType = 9;
				 sDTUInfo.strSendBuf = m_strSendUserOperationBuf;
				 if(!m_bVecCopying)
					 m_vecSendBuffer.push_back(sDTUInfo);

				 SetSendDTUEventAndType(6,st);
			 }
		 }
	 }
	 return true;
 }

 ////改为压缩包发送全部  20150409
 //bool CBEOPDataLinkManager::SendAllRealData(SYSTEMTIME tSendDTUTime)
 //{
	// //发送DTU数据   首个;			1s 及 1m中数据不发送
	// if((m_pdtusender && m_bdtuSuccess) || m_pTcpSender)
	// {
	//	 vector<Beopdatalink::CRealTimeDataEntry> entrylist;
	//	 GetRealTimeDataEntryList(entrylist);

	//	 if(entrylist.size()>0)
	//	 {
	//		 string strDTUCSQ = m_pdtusender->GetDTUCSQInfo();
	//		 if(strDTUCSQ.length()>0)
	//		 {
	//			 CRealTimeDataEntry entry = entrylist[0];
	//			 entry.SetName(DTU_CSQ);
	//			 entry.SetValue(Project::Tools::AnsiToWideChar(strDTUCSQ.c_str()));
	//			 entrylist.push_back(entry);
	//		 }

	//		 m_strSendBuf = BuildDTUSendData((int)E_STORE_NULL,entrylist,entrylist.size());
	//		 if(m_strSendBuf.size()>0)
	//		 {			 
	//			 DTUSendInfo sDTUInfo1;
	//			 sDTUInfo1.tSendTime = tSendDTUTime;
	//			 sDTUInfo1.nType = 0;
	//			 sDTUInfo1.strSendBuf = m_strSendBuf;
	//			 if(!m_bVecCopying)
	//				 m_vecSendBuffer.push_back(sDTUInfo1);
	//		 }

	//		 SetSendDTUEventAndType(0,tSendDTUTime);
	//		 m_bFirstStart = false;
	//	 }		
	//	 return true;
	// }
	//return false;
 //}

 bool CBEOPDataLinkManager::SendAllRealData(SYSTEMTIME tSendDTUTime)
 {
	 //发送DTU数据   首个;			1s 及 1m中数据不发送
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 vector<Beopdatalink::CRealTimeDataEntry> entrylist;
		 GetStoreRealTimeDataEntryList(entrylist);

		 if(entrylist.size()>0)
		 {
			 string strDTUCSQ = m_pdtusender->GetDTUCSQInfo();
			 if(strDTUCSQ.length()>0)
			 {
				 CRealTimeDataEntry entry = entrylist[0];
				 entry.SetName(DTU_CSQ);
				 entry.SetValue(Project::Tools::AnsiToWideChar(strDTUCSQ.c_str()));
				 entrylist.push_back(entry);
			 }

			 int nCount = entrylist.size();
			
			 Beopdatalink::CRealTimeDataEntry enrty;

			 wstring strFileName,strFilePath;

			 GenerateDTUUpdateInfo_Change(0,nCount,nCount,enrty);
			 entrylist.push_back(enrty);
			 GenerateAndZipDataBack(tSendDTUTime,entrylist,strFileName,strFilePath);

			 if(strFilePath.length()>0)
			 {
				 if(m_bdtuSuccess)
				 {
					 DTUSendInfo sDTUInfo;
					 sDTUInfo.tSendTime = tSendDTUTime;
					 sDTUInfo.nType = 7;
					 sDTUInfo.nPointCount = nCount;
					 sDTUInfo.strSendBuf = Project::Tools::WideCharToAnsi(strFilePath.c_str());
					 if(!m_bVecCopying)
						 m_vecSendBuffer.push_back(sDTUInfo);
				 }
			 }
			 m_bFirstStart = false;
		 }		
		 return true;
	 }
	 return false;
 }

 unsigned int WINAPI CBEOPDataLinkManager::SendDTUReceiveHandleThread( LPVOID lpVoid )
 {
	 CBEOPDataLinkManager* pDTUReceiveHandle = reinterpret_cast<CBEOPDataLinkManager*>(lpVoid);
	 if (NULL == pDTUReceiveHandle)
	 {
		 return 0;
	 }

	 while(!pDTUReceiveHandle->GetExitThread_History())
	 {		
		 pDTUReceiveHandle->ActiveLostSendEvent();		//不停激活待发送队列
		 pDTUReceiveHandle->HandleCmdFromDTUServer();
		 pDTUReceiveHandle->UpdateWatch();
		 Sleep(1000);
	 }
	 return 1;
 }

 bool CBEOPDataLinkManager::HandleCmdFromDTUServer()
 {
	  if(m_pdtusender && m_bdtuSuccess)
	  {
		  string strUpdateExeAck, strLostFile;
		  if(m_pdtusender->ScanAndSendLostPackage(strUpdateExeAck, strLostFile))
		  {
			  CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Run HandleCmdFromDTUServer->ScanAndSendLostPackage \n");

			  if(strUpdateExeAck.length() > 0)
			  {
				  AckUpdateExeFile(strUpdateExeAck);
			  }

			  if(strLostFile.length() > 0)
			  {
				  AckLostExeFile(strLostFile);
			  }
		  }

		  vector<vector<string>> vecCmd;	
		  if(m_pdtusender->GetReceiveDTUServerEvent(vecCmd))		//扫描到命令
		  {
			  CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Run HandleCmdFromDTUServer->GetReceiveDTUServerEvent \n");
			  for(int i=0;i<vecCmd.size(); ++i)
			  {
				  vector<string> vec = vecCmd[i];
				  if(vec.size() == 2)
				  {
					  if(vec[0] == "1")		//同步数据
					  {
						  int nType = ATOI(vec[1].c_str());
						  SynRealData((POINT_STORE_CYCLE)nType);
					  }
					  else if(vec[0] == "2"&& vec[1] == "0")	//同步Unit01
					  {
						  SynUnit01();
					  }
					  else if(vec[0] == "3")			//根据文件重发包
					  {
						  ReadCSVFileAndSendHistory(vec[1]);
					  }
					  else if(vec[0] == "6")		//批量修改点
					  {
						  ChangeValuesByDTUServer(vec[1]);
					  }
					  else if(vec[0] == "7")		//sql语句
					  {
						  //ExecuteDTUCmd(vec[1]);
					  }
					  else if(vec[0] == "8" && vec[1] == "0")		//心跳包请求
					  {
						  AckHeartBeat();
					  }
					  else if(vec[0] == "9")		//重发丢失数据包
					  {
						  AckReSendData(vec[1]);
					  }
					  else if(vec[0] == "4")		//重发丢失实时文件包
					  {
						  AckReSendFileData(vec[1]);
					  }
					  else if(vec[0] == "5" && vec[1] == "0")		//重启DTU
					  {
						  AckReStartDTU();
					  }
					  else if(vec[0] == "10")			//同步现场时钟
					  {
						  AckSynSystemTime(vec[1]);
					  }
				  }
				  else if(vec.size() == 3)
				  {
					  if(vec[0] == "3" && vec[1] == "0" && vec[2] == "0")		//重启Core
					  {
						  RestartCore();
					  }
					  else if(vec[0] == "3" && vec[1] == "1" && vec[2] == "0")		//Core版本号
					  {
						  AckCoreVersion();
					  }
					  else if(vec[0] == "2" && vec[1] == "0" && vec[2] == "0")		//重启watch
					  {
						  RestartWatch();
					  }
					  else if(vec[0] == "2" && vec[1] == "1" && vec[2] == "0")		//重启Logic
					  {
						  RestartLogic();
					  }
					  else if(vec[0] == "1")								//历史数据
					  {
						 SendHistoryDataFile(vec[1],vec[2]);
					  }
					  else if(vec[0] == "4")	//设置点
					  {
						  ChangeValueByDTUServer(vec[1],vec[2]);
					  }
					  else if(vec[0] == "5")	//设置Unit01
					  {
						  ChangeUnit01ByDTUServer(vec[1],vec[2]);
					  }
					  else if(vec[0] == "9")		//回应丢失掉线期间数据
					  {
						  AckReSendLostData(vec[1],vec[2]);
					  }
					  else if(vec[0] == "7")			//回应丢失掉线期间数据zip
					  {
						  AckReSendLostZipData(vec[1],vec[2]);
					  }
				  }
			  }
			  CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Exit HandleCmdFromDTUServer->GetReceiveDTUServerEvent \n");
		  }
	  }
	  return true;
 }

 bool CBEOPDataLinkManager::ChangeValueByDTUServer( string strPointName, string strPointValue )
 {
	 if (!IsMasterMode()){
		 return false;
	 }

	 wstring strChanged_Value;
	 bool bwriteresult = false;
	 if(m_bRealSite)
	 {
		 wstring pointname = Project::Tools::AnsiToWideChar(strPointName.c_str());
		 wstring strValue = Project::Tools::AnsiToWideChar(strPointValue.c_str());
		 DataPointEntry pointentry = m_pointmanager->GetEntry(pointname);
		 Beopdatalink::CRealTimeDataEntry tempentry;
		 tempentry.SetName(strPointName);
		 tempentry.SetValue(strValue);

		 if (pointentry.IsVPoint())
		 {
			 //更新内存
			 bwriteresult = WriteMemoryPoint(tempentry);
			 strChanged_Value = L"--------DTUServer Changed Memory Points--------\r\n";

			 strChanged_Value +=L"    (Memory) ";
			 strChanged_Value += pointname;
			 strChanged_Value += L"->";
			 strChanged_Value += tempentry.GetValue();
			 strChanged_Value += L";\r\n";

			 if(m_bdtuSuccess)
			 {
				 SYSTEMTIME st;
				 GetLocalTime(&st);

				 DTUSendInfo sDTUInfo;
				 sDTUInfo.tSendTime = st;
				 sDTUInfo.nType = 0;
				 sDTUInfo.nSubType = 2;
				 sDTUInfo.strSendBuf = "";
				 if(!m_bVecCopying)
					 m_vecSendBuffer.push_back(sDTUInfo);

				 SetSendDTUEventAndType(0,st);
			 }
			 SendLogMsg(L"DTUServer",strChanged_Value.c_str());
		 }
	 }
	
	 return true;
 }

 bool CBEOPDataLinkManager::ChangeUnit01ByDTUServer( string strParamName, string strParamValue )
 {
	 wstring strName = Project::Tools::AnsiToWideChar(strParamName.c_str());
	 wstring strValue = Project::Tools::AnsiToWideChar(strParamValue.c_str());
	 if(m_dbsession_history->WriteCoreDebugItemValue(strName,strValue))
	 {
		 SYSTEMTIME st;
		 GetLocalTime(&st);
		 if(m_bdtuSuccess)
		 {
			 DTUSendInfo sDTUInfo;
			 sDTUInfo.tSendTime = st;
			 sDTUInfo.nType = 3;
			 sDTUInfo.nSubType = 2;
			 sDTUInfo.strSendBuf = "";
			 sDTUInfo.strPacketIndex = "11";
			 if(!m_bVecCopying)
				 m_vecSendBuffer.push_back(sDTUInfo);
			 SetSendDTUEventAndType(0,st);
		 }
		 CString strSendInfo;
		 strSendInfo.Format(_T("RECV : DTUServer Cmd: Change Unit01 Param(%s->%s).\r\n"), strName.c_str(),strValue.c_str());
		 SendLogMsg(L"DTUServer",strSendInfo);
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::RestartCore()
 {
	 if(m_dbsession_history->WriteCoreDebugItemValue(L"reset",L"1"))
	 {
		 SYSTEMTIME st;
		 GetLocalTime(&st);
		 if(m_bdtuSuccess)
		 {
			 DTUSendInfo sDTUInfo;
			 sDTUInfo.tSendTime = st;
			 sDTUInfo.nType = 3;
			 sDTUInfo.nSubType = 3;
			 sDTUInfo.strSendBuf = "";
			 sDTUInfo.strPacketIndex = "12";
			 if(!m_bVecCopying)
				 m_vecSendBuffer.push_back(sDTUInfo);
			 SetSendDTUEventAndType(0,st);
		 }
		 CString strSendInfo = L"RECV : DTUServer Cmd: Restart Core. \r\n";
		 SendLogMsg(L"DTUServer",strSendInfo);
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::SynUnit01()
 {
	 //
	 vector<unitStateInfo> unitlist;
	 if(m_dbsession_history->ReadRealTimeData_Unit01(unitlist))
	 {
		 if(unitlist.size()>0)
		 {
			 SYSTEMTIME st;
			 GetLocalTime(&st);

			 string strSendBuf = BuildDTUSendUnit01(unitlist);
			 if(strSendBuf.size()>0)
			 {		
				 if(m_bdtuSuccess)
				 {
					 DTUSendInfo sDTUInfo;
					 sDTUInfo.tSendTime = st;
					 sDTUInfo.nType = 3;
					 sDTUInfo.nSubType = 1;
					 sDTUInfo.strSendBuf = strSendBuf;
					 sDTUInfo.strPacketIndex = "10";
					 if(!m_bVecCopying)
						 m_vecSendBuffer.push_back(sDTUInfo);
				 }
			 }

			 SetSendDTUEventAndType(3,st);
		 }

		 CString strSendInfo = L"RECV : DTUServer Cmd: Syn Unit01. \r\n";
		 SendLogMsg(L"DTUServer",strSendInfo);
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::SynRealData( POINT_STORE_CYCLE nType )
 {
	 //发送DTU数据 
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 vector<Beopdatalink::CRealTimeDataEntry> vecSynDataList;
		 if(nType == E_STORE_NULL)
		 {
			 GetStoreRealTimeDataEntryList(vecSynDataList);
		 }
		 else
		 {
			  GetRealTimeDataEntryList(vecSynDataList, nType);
		 }
		 
		 if(vecSynDataList.size()>0)
		 {
			 SYSTEMTIME st;
			 GetLocalTime(&st);

			 string strSendBuf = BuildDTUSendData((int)nType,vecSynDataList,vecSynDataList.size());
			 if(strSendBuf.size()>0)
			 {		
				 if(m_bdtuSuccess)
				 {
					 DTUSendInfo sDTUInfo;
					 sDTUInfo.tSendTime = st;
					 sDTUInfo.nType = 0;
					 sDTUInfo.nSubType = 1;
					 sDTUInfo.strSendBuf = strSendBuf;
					 sDTUInfo.strPacketIndex = (int)nType;
					 if(!m_bVecCopying)
						 m_vecSendBuffer.push_back(sDTUInfo);

					 SetSendDTUEventAndType(3,st);
				 }
			 }
		 }
		 CString strSendInfo;
		 strSendInfo.Format(L"RECV : DTUServer Cmd: Syn RealData(%s).\r\n", ConvertPointType(nType).c_str());
		 SendLogMsg(L"DTUServer",strSendInfo);
		 return true;
	 }
	 return false;
 }

 string CBEOPDataLinkManager::BuildDTUSendUnit01( vector<unitStateInfo> unitlist )
 {
	 if (unitlist.empty()){
		 return "";
	 }

	 string m_sendbuffer;
	 std::ostringstream sqlstream;
	 //pointtype,pointname,pointvalue;
	 for (unsigned int i = 0; i < unitlist.size(); i++)
	 {
		 sqlstream <<  unitlist[i].unitproperty01 << "," << unitlist[i].unitproperty02 << ";";
	 }
	 m_sendbuffer = sqlstream.str();
	 return m_sendbuffer;
 }

 wstring CBEOPDataLinkManager::ConvertPointType( POINT_STORE_CYCLE nType )
 {
	 wstring strType;
	 switch(nType)
	 {
	 case E_STORE_NULL:
		 strType = L"All";
		 break;
	 case E_STORE_FIVE_SECOND:
		 strType = L"5 second";
		 break;
	 case E_STORE_ONE_MINUTE:
		 strType = L"1 minute";
		 break;
	 case E_STORE_FIVE_MINUTE:
		 strType = L"5 minute";
		 break;
	 case E_STORE_HALF_HOUR:
		 strType = L"30 minute";
		 break;
	 case E_STORE_ONE_HOUR:
		 strType = L"1 hour";
		 break;
	 case E_STORE_ONE_DAY:
		 strType = L"1 day";
		 break;
	 case E_STORE_ONE_WEEK:
		 strType = L"1 week";
		 break;
	 case E_STORE_ONE_MONTH:
		 strType = L"1 month";
		 break;
	 case E_STORE_ONE_YEAR:
		 strType = L"1 year";
		 break;
	 default:
		 break;
	 }
	 return strType;
 }

 bool CBEOPDataLinkManager::ChangeValuesByDTUServer( string strReceive )
 {
	 wstring strChangedValues = L"";

	 if (!IsMasterMode()){
		 return false;
	 }

	 vector<wstring> m_vecCmd;
	 Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strReceive.c_str()).c_str(),L',',m_vecCmd);

	 int nPointSize = m_vecCmd.size()/2;
	 if(nPointSize<=0)
		 return false;

	 wstring strChanged_OPC, strChanged_Modbus, strChanged_Bacnet, strChanged_VPoint,strChanged_PLC,strChanged_Mysql,strChanged_Sqlite,strChanged_SqlServer;
	 if(m_bRealSite)
	 {
		 int nPacketIndex = -1;
		 wstring strPacketIndex = L"0";
		 for (unsigned int i = 0; i < nPointSize; i++)
		 {
			 CRealTimeDataEntry entry;
			 bool bwriteresult = false;
			 wstring pointname = m_vecCmd[2*i];		 
			 wstring pointvalue = m_vecCmd[2*i+1];
			
			 string strPointname_Ansi = Project::Tools::WideCharToAnsi(pointname.c_str());
			 entry.SetName(strPointname_Ansi);
			 entry.SetValue(pointvalue);

			 if(pointname == L"DTU_P_INDEX")
			 {
				 nPacketIndex = _wtoi(pointvalue.c_str());
				 strPacketIndex = pointvalue;
			 }

			 DataPointEntry pointentry = m_pointmanager->GetEntry(pointname);
			 Beopdatalink::CRealTimeDataEntry tempentry = entry;

			 if (pointentry.IsVPoint())
			 {
				 //更新内存
				 bwriteresult = WriteMemoryPoint(tempentry);	
				 if(strChanged_VPoint.length()<=0)
					 strChanged_VPoint+=L"--------DTUServer Changed Memory Points--------\r\n";

				 strChanged_VPoint +=L"    (Memory) ";
				 strChanged_VPoint += pointname;
				 strChanged_VPoint += L"->";
				 strChanged_VPoint += tempentry.GetValue();
				 strChanged_VPoint += L";\r\n";
			 }
			 else if (pointentry.IsOpcPoint() && m_nRemoteSetOPC == 1)
			 {
				 bwriteresult = WriteOPCValue(tempentry);
				 if(strChanged_OPC.length()<=0)
					 strChanged_OPC+=L"--------DTUServer Changed OPC Points-------\r\n";

				 strChanged_OPC +=L"    (OPC) ";
				 strChanged_OPC += pointname;
				 strChanged_OPC += L"->";
				 strChanged_OPC += tempentry.GetValue();
				 strChanged_OPC += L";\r\n";
			 }
			 else if (pointentry.IsModbusPoint() && m_nRemoteSetModbus == 1)
			 {
				 bwriteresult =  WriteModbusValue(tempentry);
				 if(strChanged_Modbus.length()<=0)
					 strChanged_Modbus+=L"--------DTUServer Changed Modbus Points--------\r\n";

				 strChanged_Modbus +=L"    (modbus) ";
				 strChanged_Modbus += pointname;
				 strChanged_Modbus += L"->";
				 strChanged_Modbus += tempentry.GetValue();
				 strChanged_Modbus += L";\r\n";
			 }
			 else if (pointentry.IsBacnetPoint() && m_nRemoteSetBacnet== 1)
			 {
				 bwriteresult = WriteBacnetPoint(tempentry);
				 if(strChanged_Bacnet.length()<=0)
					 strChanged_Bacnet+=L"--------DTUServer Changed Bacnet Points--------\r\n";

				 strChanged_Bacnet +=L"    (bacnet) ";
				 strChanged_Bacnet += pointname;
				 strChanged_Bacnet += L"->";
				 strChanged_Bacnet += tempentry.GetValue();
				 strChanged_Bacnet += L";\r\n";
			 }
			 else if((pointentry.IsSiemens1200Point() || pointentry.IsSiemens300Point()) && m_nRemoteSetSiemens== 1)
			 {
				 bwriteresult = WriteSiemensTCPPoint(pointentry.GetParam(3), tempentry);
				 if(strChanged_PLC.length()<=0)
					 strChanged_PLC+=L"--------DTUServer Changed SiemensTCP Points--------\r\n";

				 strChanged_PLC +=L"    (SiemensTCP) ";
				 strChanged_PLC += pointname;
				 strChanged_PLC += L"->";
				 strChanged_PLC += tempentry.GetValue();
				 strChanged_PLC += L";\r\n";
			 }
			 else if(pointentry.IsMysqlPoint() && m_nRemoteSetMysql == 1)
			 {
				 bwriteresult = WriteMysqlPoint(tempentry);
				 if(strChanged_Mysql.length()<=0)
					 strChanged_Mysql+=L"--------DTUServer Changed Mysql Points--------\r\n";

				 strChanged_Mysql +=L"    (Mysql) ";
				 strChanged_Mysql += pointname;
				 strChanged_Mysql += L"->";
				 strChanged_Mysql += tempentry.GetValue();
				 strChanged_Mysql += L";\r\n";
			 }
			 else if(pointentry.IsSqlServerPoint() && m_nRemoteSetSqlServer == 1)
			 {
				 bwriteresult = WriteSqlServerPoint(tempentry);
				 if(strChanged_SqlServer.length()<=0)
					 strChanged_SqlServer+=L"--------DTUServer Changed SqlServer Points--------\r\n";

				 strChanged_SqlServer +=L"    (SqlServer) ";
				 strChanged_SqlServer += pointname;
				 strChanged_SqlServer += L"->";
				 strChanged_SqlServer += tempentry.GetValue();
				 strChanged_SqlServer += L";\r\n";
			 }
			 else if(pointentry.IsSqlitePoint() && m_nRemoteSetSqlite == 1)
			 {
				 bwriteresult = WriteSqlitePoint(tempentry);
				 if(strChanged_Sqlite.length()<=0)
					 strChanged_Sqlite+=L"--------DTUServer Changed Sqlite Points--------\r\n";

				 strChanged_Sqlite +=L"    (Sqlite) ";
				 strChanged_Sqlite += pointname;
				 strChanged_Sqlite += L"->";
				 strChanged_Sqlite += tempentry.GetValue();
				 strChanged_Sqlite += L";\r\n";
			 }
		 }
		 if(strChanged_VPoint.length()>0 || strChanged_OPC.length()>0 || strChanged_Modbus.length()>0 || strChanged_Bacnet.length()>0 || strChanged_PLC.length()>0 || strChanged_Mysql.length()>0  || strChanged_SqlServer.length()>0  ||strChanged_Sqlite.length()>0)
		 {
			 GetLocalTime(&m_ackTime);

			 if(m_strAck.length() <= 0)
			 {
				 m_strAck =  Project::Tools::WideCharToAnsi(strPacketIndex.c_str());
			 }
			 else
			 {
				  m_strAck += "|";
				  m_strAck += Project::Tools::WideCharToAnsi(strPacketIndex.c_str());
			 }
			 SetSendDTUEventAndType(0,m_ackTime);
			 CString strSendInfo = L"RECV : DTUServer Cmd: Change points value. \r\n";
			 SendLogMsg(L"DTUServer",strSendInfo);
		 }

		 if(strChanged_OPC.length()>0)
		 {
			 strChangedValues+= strChanged_OPC;
		 }
		 if(strChanged_Modbus.length()>0)
		 {
			 strChangedValues+= strChanged_Modbus;
			 m_oleLastRemoteSetting = COleDateTime::GetCurrentTime();
		 }
		 if(strChanged_Bacnet.length()>0)
		 {
			 strChangedValues+= strChanged_Bacnet;
		 }
		 if(strChanged_VPoint.length()>0)
		 {
			 strChangedValues+= strChanged_VPoint;
		 }
		 if(strChanged_PLC.length()>0)
		 {
			 strChangedValues+= strChanged_PLC;
		 }
		 if(strChanged_Mysql.length()>0)
		 {
			 strChangedValues+= strChanged_Mysql;
		 }
		 if(strChanged_SqlServer.length()>0)
		 {
			 strChangedValues+= strChanged_SqlServer;
		 }
		 if(strChanged_Sqlite.length()>0)
		 {
			 strChangedValues+= strChanged_Sqlite;
		 }
		 if(strChangedValues.length()>0)
			 PrintLog(strChangedValues,false);
	 }
	 return true;
 }

 bool CBEOPDataLinkManager::ExecuteDTUCmd( string strReceive )
 {
	 string strCmd_Utf8 = Project::Tools::MultiByteToUtf8(strReceive.c_str());
	 bool bResult = m_dbsession_history->Execute(strCmd_Utf8.c_str());
	 if(bResult)
	 {
		 SYSTEMTIME st;
		 GetLocalTime(&st);

		 if(m_bdtuSuccess)
		 {
			 DTUSendInfo sDTUInfo;
			 sDTUInfo.tSendTime = st;
			 sDTUInfo.nType = 3;
			 sDTUInfo.nSubType = 5;
			 sDTUInfo.strSendBuf = "";
			 if(!m_bVecCopying)
				 m_vecSendBuffer.push_back(sDTUInfo);
			 SetSendDTUEventAndType(0,st);
		 }
		 if(strReceive.length() <= 200)
		 {
			 wstring strCmd = Project::Tools::AnsiToWideChar(strReceive.c_str());
			 CString strSendInfo;
			 strSendInfo.Format(_T("RECV : DTUServer Cmd: Execute Cmd(%s). \r\n"),strCmd.c_str());
			 SendLogMsg(L"DTUServer",strSendInfo);
		 }
		 else
		 {
			 CString strSendInfo = L"RECV : DTUServer Cmd: Execute Cmd. \r\n";
			 SendLogMsg(L"DTUServer",strSendInfo);
		 }
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::Exit()
 {
	 m_bexitthread_output = true;
	 m_bexitthread_input = true;
	 m_bexitthread_history = true;

	 if(m_hhistorydata)
		 WaitForSingleObject(m_hhistorydata, INFINITE);

	 if(m_hhistoryS5data)
		 WaitForSingleObject(m_hhistoryS5data, INFINITE);

	 if(m_hupdateinput)
		 WaitForSingleObject(m_hupdateinput, INFINITE);

	 if(m_hsenddtuhandle)
	 {
		 SetEvent(m_senddtuevent);
		 WaitForSingleObject(m_hsenddtuhandle, INFINITE);
	 }

	 if(m_hReceivedtuhandle)
		 WaitForSingleObject(m_hReceivedtuhandle, INFINITE);

	 if(m_hscanwarninghandle)
		 WaitForSingleObject(m_hscanwarninghandle, INFINITE);

	 //	 WaitForSingleObject(m_hupdateoutput, INFINITE);

	 //各个引擎退出  采取标准的Exit退出  Robert 20140807

	 //modbus退出
	 map<wstring,CModbusEngine*>::iterator iter = m_mapMdobusEngine.begin();
	 while(iter != m_mapMdobusEngine.end())
	 {
		 iter->second->Exit();
		 iter++;
	 }

	 //co3p退出
	 map<wstring,CCO3PEngine*>::iterator iterCO3P = m_mapCO3PEngine.begin();
	 while(iterCO3P != m_mapCO3PEngine.end())
	 {
		 iterCO3P->second->Exit();
		 iterCO3P++;
	 }

	 //modbus退出
	 map<wstring,CFCBusEngine*>::iterator iterFC = m_mapFCBusEngine.begin();
	 while(iterFC != m_mapFCBusEngine.end())
	 {
		 iterFC->second->Exit();
		 iterFC++;
	 }

	  //104退出
	 map<wstring,CProtocol104Engine*>::iterator iter1 = m_mapProtocol104Engine.begin();
	 while(iter1 != m_mapProtocol104Engine.end())
	 {
		 iter1->second->Exit();
		 iter1++;
	 }

	 //mysql 退出
	 map<wstring,CMysqlEngine*>::iterator iter2 = m_mapMysqlEngine.begin();
	 while(iter2 != m_mapMysqlEngine.end())
	 {
		 iter2->second->Exit();
		 iter2++;
	 }

	 //web 退出
	 map<wstring,CWebEngine*>::iterator iterWeb = m_mapWebEngine.begin();
	 while(iterWeb != m_mapWebEngine.end())
	 {
		 iterWeb->second->Exit();
		 iterWeb++;
	 }

	 //sqlserver 退出
	 map<wstring,CSqlServerEngine*>::iterator iterSqlServer = m_mapSqlServerEngine.begin();
	 while(iterSqlServer != m_mapSqlServerEngine.end())
	 {
		 iterSqlServer->second->Exit();
		 iterSqlServer++;
	 }

	 //sqlite 退出
	 map<wstring,CSqliteEngine*>::iterator iterSqlite = m_mapSqliteEngine.begin();
	 while(iterSqlite != m_mapSqliteEngine.end())
	 {
		 iterSqlite->second->Exit();
		 iterSqlite++;
	 }


	  //opc 退出
	 if (m_opcengine){
		 m_opcengine->Exit();
	 }

	 //opc 退出 大批量
	 map<int,COPCEngine*>::iterator iter3 = m_mapOPCEngine.begin();
	 while(iter3 != m_mapOPCEngine.end())
	 {
		 iter3->second->Exit();
		 iter3++;
	 }

	  //memory 退出
	 if (m_memoryengine){
		m_memoryengine->Exit();
	 }

	 //diagnose退出
	 if(m_pDiagnoseEngine)
	 {
		 m_pDiagnoseEngine->Exit();
	 }

	//bacnet退出
	 if (m_bacnetengine){
		 m_bacnetengine->Exit();
	 }

	// DTU退出
	 if (m_pdtusender)
	 {
		 m_pdtusender->Exit();
		 m_bdtuSuccess = false;
	 }

	 //err退出
	 CDebugLog::GetInstance()->Exit();
	 CDebugLog::GetInstance()->DestroyInstance();

	 //dog退出
	 CDogTcpCtrl::GetInstance()->Exit();
	 CDogTcpCtrl::GetInstance()->DestroyInstance();

	 //map
	 m_RealTimeDataMap.clear();
	 m_vecModbusSer.clear();
	 m_vecCO3PSer.clear();
	 m_vecProtocol104Ser.clear();
	 m_vecMysqlSer.clear();
	 m_vecSqlSer.clear();
	 m_vecSqliteSer.clear();
	 m_vecFCSer.clear();
	 m_vecOPCSer.clear();
	 m_vecOutputEntry.clear();
	 m_vecMemoryPointChangeList.clear();
	 m_RealTimeDataEntryList.clear();
	 m_RealTimeDataEntryListBufferMap_S5.clear();
	 m_RealTimeDataEntryListBufferMap_M1.clear();
	 m_RealTimeDataEntryListBufferMap_M5.clear();
	 m_RealTimeDataEntryListBufferMap_H1.clear();
	 m_RealTimeDataEntryListBufferMap_D1.clear();
	 m_RealTimeDataEntryListBufferMap_Month1.clear();
	 m_mapSendLostHistory.clear();
	 m_vecSendBuffer.clear();

	 return true;
 }

 bool CBEOPDataLinkManager::InitDTUEngine(bool bDTUEnabled, bool bTCPEnable)
 {
	 if(bDTUEnabled == bTCPEnable && bDTUEnabled == true)
	 {
		  SendLogMsg(L"", L"ERROR: It does not support running simultaneously DTU and TCPSender...\r\n");
		  return false;
	 }

	 if(bDTUEnabled)
	 {
		 m_bexitthread_history = false;
		 //初始化DTU
		 SendLogMsg(L"", L"INFO : Init DTU Engine...\r\n");
		 m_pdtusender		= new CDTUSender(this->m_dbsession_history,m_bDTUChecked,m_bDTURecCmd,Project::Tools::WideCharToAnsi(m_strTCPName.c_str()),Project::Tools::WideCharToAnsi(m_strTCPServerIP.c_str()),m_nTCPServerPort,true,m_nDTUPort);
		 if(m_pdtusender)
		 {
			 m_bdtuSuccess = m_pdtusender->InitDTU();
			 if(m_bdtuSuccess)
			 {
				 if(m_hsenddtuhandle == NULL)
					 m_hsenddtuhandle = (HANDLE)_beginthreadex(NULL, 0, SendDTUThread, this, 0, NULL);
				 if(m_hReceivedtuhandle == NULL)
					 m_hReceivedtuhandle = (HANDLE)_beginthreadex(NULL, 0, SendDTUReceiveHandleThread, this, 0, NULL);
				 if(m_hscanwarninghandle == NULL)
					 m_hscanwarninghandle = (HANDLE)_beginthreadex(NULL, 0, ScanWarningAndOperation, this, 0, NULL);
			 }
			 else
			 {
				 SendLogMsg(L"", L"ERROR: Init DTU Engine(DTU) Fail,Please Check DTU Setting...\r\n");
			 }
		 }
		 return m_bdtuSuccess;
	 }

	 if(bTCPEnable)
	 {
		 m_bexitthread_history = false;
		 //初始化DTU
		 SendLogMsg(L"", L"INFO : Init TCPSender Engine...\r\n");

		 if(m_strTCPName.length() <= 0)
		 {
			  SendLogMsg(L"", L"ERROR: Init TCPSender Engine Fail(TCPSenderName is null)...\r\n");
			  return false;
		 }

		 m_pdtusender		= new CDTUSender(this->m_dbsession_history,m_bDTUChecked,m_bDTURecCmd,Project::Tools::WideCharToAnsi(m_strTCPName.c_str()),Project::Tools::WideCharToAnsi(m_strTCPServerIP.c_str()),m_nTCPServerPort,false,m_nDTUPort);
		 if(m_pdtusender)
		 {
			 m_bdtuSuccess = m_pdtusender->InitDTU();
			 if(!m_bdtuSuccess)
			 {
				  SendLogMsg(L"", L"ERROR: Init TCP Engine(TCP) Fail,Please Check TCP Setting...\r\n");
			 }

			 if(m_hsenddtuhandle == NULL)
				 m_hsenddtuhandle = (HANDLE)_beginthreadex(NULL, 0, SendDTUThread, this, 0, NULL);
			 if(m_hReceivedtuhandle == NULL)
				 m_hReceivedtuhandle = (HANDLE)_beginthreadex(NULL, 0, SendDTUReceiveHandleThread, this, 0, NULL);
			 if(m_hscanwarninghandle == NULL)
				 m_hscanwarninghandle = (HANDLE)_beginthreadex(NULL, 0, ScanWarningAndOperation, this, 0, NULL);
		 }
		 return m_bdtuSuccess;
	 }
 }

 bool CBEOPDataLinkManager::SendDTUTestPoint(string strPointName, wstring strValue)
 {
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 vector<Beopdatalink::CRealTimeDataEntry> entrylist;
		 Beopdatalink::CRealTimeDataEntry entry;
		 SYSTEMTIME sDTUTime;
		 GetLocalTime(&sDTUTime);
		 entry.SetTimestamp(sDTUTime);
		 entry.SetName(strPointName);
		 entry.SetValue(strValue);
		 entrylist.push_back(entry);
		
		 if(entrylist.size()>0)
		 {
			 m_strSendBuf = BuildDTUSendData((int)E_STORE_NULL,entrylist,entrylist.size());
			 if(m_strSendBuf.size()>0)
			 {	
				 if(m_bdtuSuccess)
				 {
					 DTUSendInfo sDTUInfo;
					 sDTUInfo.tSendTime = sDTUTime;
					 sDTUInfo.nType = 0;
					 sDTUInfo.strSendBuf = m_strSendBuf;
					 if(!m_bVecCopying)
						 m_vecSendBuffer.push_back(sDTUInfo);
				 }
			 }

			 SetSendDTUEventAndType(0,sDTUTime);
		 }		
		 return true;
	 }
	 return false;
 }

 CDTUSender* CBEOPDataLinkManager::GetDTUSender()
 {
	 return m_pdtusender;
 }

 void CBEOPDataLinkManager::GetBacnetValueSets( vector<pair<wstring, wstring> >& bacnetvaluesets )
 {
	 if (m_bacnetengine){
		 m_bacnetengine->GetValueSet(bacnetvaluesets);
	 }
 }

 string CBEOPDataLinkManager::GenerateDTUUpdateInfo( int nPointStoreType,int nUpdateSize ,int nCountSize)
 {
	 string strDTUUpdateInfo = "";
	 std::ostringstream sqlstream;
	 switch((POINT_STORE_CYCLE)nPointStoreType)
	 {
	 case E_STORE_NULL:
		 {
			 sqlstream << nPointStoreType << "," << DTU_UPDATE_ALL << "," << nUpdateSize << "/" << nCountSize << ";";
		 }
		 break;
	 case E_STORE_FIVE_SECOND:
		 {
			 sqlstream << nPointStoreType << "," << DTU_UPDATE_S5 << "," << nUpdateSize << "/" << nCountSize << ";";
		 }
		 break;
	 case E_STORE_ONE_MINUTE:
		 {
			  sqlstream << nPointStoreType << "," << DTU_UPDATE_M1 << "," << nUpdateSize << "/" << nCountSize << ";";
		 }
		 break;
	 case E_STORE_FIVE_MINUTE:
		 {
			  sqlstream << nPointStoreType << "," << DTU_UPDATE_M5 << "," << nUpdateSize << "/" << nCountSize << ";";
		 }
		 break;
	 case E_STORE_ONE_HOUR:
		 {
			  sqlstream << nPointStoreType << "," << DTU_UPDATE_H1 << "," << nUpdateSize << "/" << nCountSize << ";";
		 }
		 break;
	 case E_STORE_ONE_DAY:
		 {
			  sqlstream << nPointStoreType << "," << DTU_UPDATE_D1 << "," << nUpdateSize << "/" << nCountSize << ";";
		 }
		 break;
	 case E_STORE_ONE_MONTH:
		 {
			 sqlstream << nPointStoreType << "," << DTU_UPDATE_MONTH1 << "," << nUpdateSize << "/" << nCountSize << ";";
		 }
		 break;
	 default:
		 break;
	 }
	 strDTUUpdateInfo = sqlstream.str();
	 return strDTUUpdateInfo;
 }

 bool CBEOPDataLinkManager::InitFCbusEngine( vector<DataPointEntry> fcbuspoint, int nReadCmdMs /*= 50*/ ,int nPrecision)
 {
	 if(fcbuspoint.empty())
		 return false;

	 vector<vector<DataPointEntry>> vecServerList = GetFCbusServer(fcbuspoint);

	 for(int i=0; i<vecServerList.size(); i++)
	 {
		 if(!vecServerList[i].empty())
		 {
			 CFCBusEngine* pFCbusengine = new CFCBusEngine(this);
			 wstring strSer = vecServerList[i][0].GetFCServerIP();
			 wstring strPort = vecServerList[i][0].GetFCServerPort();
			 pFCbusengine->SetPointList(vecServerList[i]);
			 pFCbusengine->SetLogSession(m_dbsession_log);
			 pFCbusengine->SetSendCmdTimeInterval(nReadCmdMs,nPrecision);
			 pFCbusengine->SetIndex(i);
			 if(!pFCbusengine->Init())
			 {
				 CString strFailedInfo;
				 strFailedInfo.Format(L"ERROR: Init FCbus Engine(%s:%s) Failed.\r\n", strSer.c_str(),strPort.c_str());
				 SendLogMsg(L"", strFailedInfo);
			 }
			 m_mapFCBusEngine[strSer+strPort] = pFCbusengine;
		 }
	 }
	 return true;
 }

 vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetFCbusServer( vector<DataPointEntry> fcbuspoint )
 {
	 vector<vector<DataPointEntry>> vecFCbusPList;
	 m_vecFCSer.clear();
	 for(int i=0; i<fcbuspoint.size(); i++)
	 {
		 //
		 wstring strSer = fcbuspoint[i].GetFCServerIP();
		 wstring strPort = fcbuspoint[i].GetFCServerPort();
		 if(strSer.empty() || strPort.empty())
			 continue;

		 int nPos = IsExistInVec(strSer,strPort,m_vecFCSer);
		 if(nPos==m_vecFCSer.size())	//不存在
		 {
			 FCServer server;
			 server.strIP = strSer;
			 server.strPort = strPort;
			 m_vecFCSer.push_back(server);
			 vector<DataPointEntry> vecPList;
			 vecFCbusPList.push_back(vecPList);
		 }
		 vecFCbusPList[nPos].push_back(fcbuspoint[i]);		
	 }
	 return vecFCbusPList;
 }

 void CBEOPDataLinkManager::GetFCbusValueSets( vector<pair<wstring, wstring> >& fcbusvaluesets )
 {
	 map<wstring,CFCBusEngine*>::iterator iter = m_mapFCBusEngine.begin();
	 while(iter != m_mapFCBusEngine.end())
	 {
		 iter->second->GetValueSet(fcbusvaluesets);
		 iter++;
	 }
 }

 bool CBEOPDataLinkManager::WriteFCbusValue( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);
	  wstring strFC = pentry.GetFCServerIP() + pentry.GetFCServerPort();
	 map<wstring,CFCBusEngine*>::iterator iter = m_mapFCBusEngine.find(strFC);
	 if(iter != m_mapFCBusEngine.end())
	 {
		 m_mapFCBusEngine[strFC]->SetValue(pointname, _wtof(entry.GetValue().c_str()));
		 return true;
	 }

	 return false;
 }

 bool CBEOPDataLinkManager::AckHeartBeat()
 {
	 SYSTEMTIME st;
	 GetLocalTime(&st);
	 if(m_bdtuSuccess)
	 {
		 DTUSendInfo sDTUInfo;
		 sDTUInfo.tSendTime = st;
		 sDTUInfo.nType = 3;
		 sDTUInfo.nSubType = 6;
		 sDTUInfo.strSendBuf = "";
		 sDTUInfo.strPacketIndex = "13";
		 if(!m_bVecCopying)
			 m_vecSendBuffer.push_back(sDTUInfo);
		 SetSendDTUEventAndType(0,st);
	 }
	 CString strSendInfo = L"RECV : DTUServer Cmd: HeartBeat. \r\n";
	 SendLogMsg(L"DTUServer",strSendInfo);
	 return true;
 }

 bool CBEOPDataLinkManager::SendExecuteOrderLog()
 {
	 wstring strEnable;
	 if(m_dbsession_history)
	 {
		  m_dbsession_history->ReadCoreDebugItemValue(L"LogEnable",strEnable);
		  if(_wtoi(strEnable.c_str()) == 1)
		  {
			   vector<string> vecLog;
	 
				if (m_bacnetengine){
					string strLog = m_bacnetengine->GetExecuteOrderLog();
					if(strLog.length()>0)
						vecLog.push_back(strLog);
				}

				for(int i=0; i<vecLog.size(); ++i)
				{
					SYSTEMTIME st;
					GetLocalTime(&st);
					if(m_bdtuSuccess)
					{
						DTUSendInfo sDTUInfo;
						sDTUInfo.tSendTime = st;
						sDTUInfo.nType = 4;
						sDTUInfo.strSendBuf = vecLog[i];
						OutPutLogString(st,vecLog[i]);
						if(!m_bVecCopying)
							m_vecSendBuffer.push_back(sDTUInfo);
					}
				}
				return true;
		  }
	 }
	return false;
 }

 bool CBEOPDataLinkManager::OutPutLogString( SYSTEMTIME st,string strOut )
 {
	 wstring strPath;
	 GetSysPath(strPath);
	 strPath +=  _T("\\beopexecutelog.log");
	
	 CString strLog;
	 wstring strTime;
	 Project::Tools::SysTime_To_String(st,strTime);
	 strLog += strTime.c_str();
	 strLog += _T(" - ");
	 wstring strOut_;
	 Project::Tools::UTF8ToWideChar(strOut,strOut_);
	 strLog += strOut_.c_str();
	 strLog += _T("\n");

	 //记录Log
	 CStdioFile	ff;
	 if(ff.Open(strPath.c_str(),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
	 {
		 ff.Seek(0,CFile::end);
		 ff.WriteString(strLog);
		 ff.Close();
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::AckReSendData( string strIndex )
 {
	 Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	 //发送DTU数据 
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 SYSTEMTIME st;
		 GetLocalTime(&st);
		 if(m_bdtuSuccess)
		 {
			 DTUSendInfo sDTUInfo;
			 sDTUInfo.nType = 5;
			 sDTUInfo.nSubType = 0;
			 sDTUInfo.strSendBuf = strIndex;
			 if(!m_bVecCopying)
				 m_vecSendBuffer.push_back(sDTUInfo);
			 SetSendDTUEventAndType(3,st);
		 }

		 CString strSendInfo;
		 strSendInfo.Format(L"RECV : DTUServer Cmd: ReSend RealData(%s).\r\n", Project::Tools::AnsiToWideChar(strIndex.c_str()).c_str());
		 SendLogMsg(L"DTUServer",strSendInfo);
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::WriteMysqlPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 wstring strSer = pentry.GetParam(3) + pentry.GetParam(8);
	 map<wstring,CMysqlEngine*>::iterator iter = m_mapMysqlEngine.find(strSer);
	 if(iter != m_mapMysqlEngine.end())
	 {
		 if(m_mapMysqlEngine[strSer]->SetValue(pointname, entry.GetValue()))
		 {
			 return true;
		 }
	 }

	 return false;
 }

 void CBEOPDataLinkManager::CompareAndUpdateRealTimeDataEntryList( SYSTEMTIME sNow)
 {
	 vector<Beopdatalink::CRealTimeDataEntry> entrylist;
	 GetStoreRealTimeDataEntryList(entrylist);

	 for(int i =0; i<entrylist.size();i++)
	 {
		 string strPointName = entrylist[i].GetName();
		 std::map<string,CRealTimeDataEntry>::iterator iter = m_RealTimeDataMap.find(strPointName);
		 if(iter!=m_RealTimeDataMap.end())
		 {
			 wstring strEntryValue = entrylist[i].GetValue();
			 if(strEntryValue != (*iter).second.GetValue())
			 {
				 (*iter).second.SetValue(strEntryValue);
				  (*iter).second.SetTimestamp(sNow);
			 }
		 }
		 else
		 {
			 m_RealTimeDataMap[strPointName] = entrylist[i];
		 }
	 }
 }

 void CBEOPDataLinkManager::GetModifiedRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, int nMinutes )
 {
	 COleDateTime oleBegin = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0,0,nMinutes,0);
	 SYSTEMTIME sBegin;
	 oleBegin.GetAsSystemTime(sBegin);
	 entrylist.clear();
	 std::map<string,CRealTimeDataEntry>::iterator iter = m_RealTimeDataMap.begin();
	 while(iter != m_RealTimeDataMap.end())
	 {
		 CRealTimeDataEntry entry = (*iter).second;
		 if(memcmp(&sBegin,&entry.GetTimestamp(),sizeof(SYSTEMTIME))<=0)
		 {
			 entrylist.push_back(entry);
		 }
		 iter++;
	 }
 }

 bool CBEOPDataLinkManager::AckReSendLostData( string strMinutes ,string strPacketIndex)
 {
	 Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	 //发送DTU数据 
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 vector<Beopdatalink::CRealTimeDataEntry> vecSynDataList;
		 int nMinutes = atoi(strMinutes.c_str());
		 GetModifiedRealTimeDataEntryList(vecSynDataList,nMinutes);
		 if(vecSynDataList.size()>0)
		 {
			 SYSTEMTIME st;
			 GetLocalTime(&st);

			 string strSendBuf = BuildDTUSendData(1,vecSynDataList,vecSynDataList.size());
			 if(strSendBuf.size()>0)
			 {		
				 if(m_bdtuSuccess)
				 {
					 DTUSendInfo sDTUInfo;
					 sDTUInfo.tSendTime = st;
					 sDTUInfo.nType = 0;
					 sDTUInfo.nSubType = 3;
					 sDTUInfo.strSendBuf = strSendBuf;
					 sDTUInfo.strPacketIndex = strMinutes;
					 if(!m_bVecCopying)
						 m_vecSendBuffer.push_back(sDTUInfo);

					 SetSendDTUEventAndType(3,st);
				 }
			 }
		 }
		 CString strSendInfo;
		 strSendInfo.Format(L"RECV : DTUServer Cmd: ReSend RealData(%sm).\r\n", Project::Tools::AnsiToWideChar(strMinutes.c_str()).c_str());
		 SendLogMsg(L"DTUServer",strSendInfo);
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::SendCoreStart(SYSTEMTIME tSendDTUTime)
 {
	 //发送DTU数据   首个;			1s 及 1m中数据不发送
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 DTUSendInfo sDTUInfo;
		 sDTUInfo.tSendTime = tSendDTUTime;
		 sDTUInfo.nType = 3;
		 sDTUInfo.nSubType = 7;
		 sDTUInfo.strSendBuf = "";
		 if(!m_bVecCopying)
			 m_vecSendBuffer.push_back(sDTUInfo);

		 SetSendDTUEventAndType(0,tSendDTUTime);
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::UpdateHistoryStatusTable()
 {

	 return m_dbsession_history->UpdateHistoryStatusTable();
 }

 bool CBEOPDataLinkManager::SendCoreStart()
 {
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 SYSTEMTIME sNow;
		 GetLocalTime(&sNow);

		 DTUSendInfo sDTUInfo;
		 sDTUInfo.tSendTime = sNow;
		 sDTUInfo.nType = 3;
		 sDTUInfo.nSubType = 7;
		 sDTUInfo.strSendBuf = "";
		 if(!m_bVecCopying)
			 m_vecSendBuffer.push_back(sDTUInfo);

		 SetSendDTUEventAndType(0,sNow);
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::GenerateAndZipDataBack(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,wstring& strFileName_,wstring& strFilePath_)
 {
	 return true;//golding 20190208临时去掉，减少现场操作磁盘IO的密度

	 wstring strBackupFileFolder,strBackupFileName;
	 strFileName_ = L"";
	 strFilePath_ = L"";
	if(GenerateDataBack(st,strBackupFileName,strBackupFileFolder,entrylist))
	{
		CString strZipPath,strFileName,strFilePath;
		strZipPath.Format(_T("%s\\%s.zip"),strBackupFileFolder.c_str(),strBackupFileName.c_str());
		strFileName.Format(_T("%s.csv"),strBackupFileName.c_str());
		strFilePath.Format(_T("%s\\%s"),strBackupFileFolder.c_str(),strFileName);

		strFileName_ = strBackupFileName;
		strFilePath_ = strZipPath.GetString();

		HZIP hz = CreateZip(strZipPath,NULL);
		ZipAdd(hz,strFileName,strFilePath);
		CloseZip(hz);
		DeleteFile(strFilePath);

		//存储文件
		strFileName.Format(_T("%s.zip"),strBackupFileName.c_str());
		StoreFileInfo(strBackupFileFolder,strFileName.GetString(),entrylist.size());
		return true;
	}
	return false;
 }

 bool CBEOPDataLinkManager::GenerateDataBack( const SYSTEMTIME &st , wstring& strFileName, wstring& strFileFolder, vector<Beopdatalink::CRealTimeDataEntry> &entrylist)
 {
	 return true;//golding 20190208临时去掉，减少现场操作磁盘IO的密度

	 if(entrylist.size() <= 0)
		 return false;

	 int nCount = 0;
	 std::ostringstream sqlstream;
	 vector<string> vecContent;
	 for (unsigned int i = 0; i < entrylist.size(); i++)
	 {
		 string strvalue;
		 Project::Tools::WideCharToUtf8(entrylist[i].GetValue(),strvalue);
		 sqlstream << entrylist[i].GetName() << "," << strvalue << ";";
		 if(nCount == 1000)
		 {
			 string sql_temp = sqlstream.str();
			 vecContent.push_back(sql_temp);
			 sqlstream.str("");
			 nCount = 0;
		 }
		 nCount++;
	 }
	 string sql_temp = sqlstream.str();
	 vecContent.push_back(sql_temp);

	 wstring strPath;
	 GetSysPath(strPath);
	 strPath += L"\\backup";
	 if(Project::Tools::FindOrCreateFolder(strPath))
	 {
		 CString strFolder;
		 strFolder.Format(_T("%s\\%d%02d%02d"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);
		 strPath = strFolder.GetString();
		 if(Project::Tools::FindOrCreateFolder(strPath))
		 {
			 CString strBackFilePath,strBackFileName;
			 strBackFileName.Format(_T("databackup_%d%02d%02d%02d%02d"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute);
			 strBackFilePath.Format(_T("%s\\%s.csv"),strPath.c_str(),strBackFileName);
			 strFileName = strBackFileName.GetString();
			 strFileFolder = strPath;
			 char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			 setlocale( LC_ALL, "chs" );
			 CStdioFile	ff;
			 if(ff.Open(strBackFilePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			 {
				 CString strContent;
				 for(int j=0; j<vecContent.size(); ++j)
				 {
					 //strContent = Project::Tools::AnsiToWideChar(vecContent[j].c_str()).c_str();
					 wstring strValue;
					 Project::Tools::UTF8ToWideChar(vecContent[j],strValue);
					 strContent = strValue.c_str();
					 ff.Seek(0,CFile::end);
					 ff.WriteString(strContent);
				 }
				 ff.Close();
				 setlocale( LC_CTYPE, old_locale ); 
				 free( old_locale ); 
				 return true;
			 }
			 setlocale( LC_CTYPE, old_locale ); 
			 free( old_locale );
		 }
		 return false;
	 }
 }

 bool CBEOPDataLinkManager::SendHistoryDataFile( string strStart,string strEnd )
 {
	 Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	 //发送DTU数据 
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 wstring strPath;
		 GetSysPath(strPath);
		 strPath += L"\\backup";	 
		 if(Project::Tools::FindOrCreateFolder(strPath))
		 {
			 //将全部数据文件打包成一个压缩包
			 COleDateTime oleStart,oleEnd;
			 COleDateTimeSpan oleSpan(0,0,1,0);
			 Project::Tools::String_To_OleTime(Project::Tools::AnsiToWideChar(strStart.c_str()),oleStart);
			 Project::Tools::String_To_OleTime(Project::Tools::AnsiToWideChar(strEnd.c_str()),oleEnd);

			 CString strZipPath;
			 strZipPath.Format(_T("%s\\history_%d%02d%02d.zip"),strPath.c_str(),oleStart.GetYear(),oleStart.GetMonth(),oleStart.GetDay());
			 HZIP hz = CreateZip(strZipPath,NULL);
			 bool bFileExist = false;
			 while(oleStart <= oleEnd)
			 {
				 if(oleStart.GetMinute() %5 != 0)
				 {
					 oleStart = oleStart + oleSpan;
					 continue;
				 }
				 CString strFilePath,strFileName;
				 strFileName.Format(_T("databackup_%d%02d%02d%02d%02d.zip"),oleStart.GetYear(),oleStart.GetMonth(),oleStart.GetDay(),oleStart.GetHour(),oleStart.GetMinute());
				 strFilePath.Format(_T("%s\\%d%02d%02d\\%s"),strPath.c_str(),oleStart.GetYear(),oleStart.GetMonth(),oleStart.GetDay(),strFileName);
				 if(Project::Tools::FindFileExist(strFilePath.GetString()))
				 {
					 bFileExist = true;
					 ZipAdd(hz,strFileName,strFilePath);
				 }
				 oleStart = oleStart + oleSpan;
			 }
			 CloseZip(hz);

			 SYSTEMTIME st;
			 GetLocalTime(&st);

			 DTUSendInfo sDTUInfo;
			 sDTUInfo.tSendTime = st;
			 sDTUInfo.nType = 6;
			 m_strZipPath = Project::Tools::WideCharToAnsi(strZipPath);
			 if(bFileExist)
			 {
				 sDTUInfo.strSendBuf = m_strZipPath;
			 }
			 else
			 {
				 sDTUInfo.strSendBuf = m_strZipPath;
			 }
			 if(m_bdtuSuccess)
			 {
				 if(!m_bVecCopying)
					 m_vecSendBuffer.push_back(sDTUInfo);

				 SetSendDTUEventAndType(3,st);
			 }

			 CString strSendInfo;
			 strSendInfo.Format(L"RECV : DTUServer Cmd: Send History Data(%s-%s).\r\n", Project::Tools::AnsiToWideChar(strStart.c_str()).c_str(), Project::Tools::AnsiToWideChar(strEnd.c_str()).c_str());
			 SendLogMsg(L"DTUServer",strSendInfo);
			 return true;
		 }
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::GenerateDTURealData( const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle, int nSendAll )
 {
	 //发送DTU数据   首个;			1s 及 5s中数据不发送   或者TCP网络发送模式  只更新1分钟数据
	 CString strDebugInfo;
	 strDebugInfo.Format(_T("DEBUG:Run ThreadFuncHistoryData->GenerateDTURealData(StoreType:%d,MinType:%d,SendAll:%d).\n"),(int)tCycle,m_nDTUSendMinType,nSendAll);
	 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DATALINK,E_DEBUG_LOG_LEVEL_PROCESS,strDebugInfo.GetString());
	 if( tCycle ==(POINT_STORE_CYCLE)m_nDTUSendMinType)
	 {
		  vector<Beopdatalink::CRealTimeDataEntry> entrylist,entryChangelist;
		  int nMinuteInterval = 0;
		  switch(m_nDTUSendMinType)
		  {
		  case 2:
			  nMinuteInterval = 1;
			  break;
		  case 3:
			  nMinuteInterval = 5;
			  break;
		  case 5:
			  nMinuteInterval = 60;
			  break;
		  default:
			  nMinuteInterval = 0;
			  break;
		  }

		  GetStoreRealTimeDataEntryList(entrylist);
		  GetModifiedRealTimeDataEntryList(entryChangelist,nMinuteInterval);
		  int nChangeSize = entryChangelist.size();
		  int nCount = entrylist.size();
		  if(nCount <= 0)
			  return false;

		  Beopdatalink::CRealTimeDataEntry enrty;
		  int nSendPointCount = nCount;
		  wstring strFileName,strFilePath;
		  if(nSendAll == 1)		//发送全部数据
		  {
			  GenerateDTUUpdateInfo_Change((int)tCycle,nChangeSize,nCount,enrty);
			  entrylist.push_back(enrty);
			  GenerateAndZipDataBack(st,entrylist,strFileName,strFilePath);
			  nSendPointCount = entrylist.size();
		  }
		  else
		  {
			  if(nMinuteInterval <= 0)		//发送全部
			  {
				  GenerateDTUUpdateInfo_Change((int)tCycle,nChangeSize,nCount,enrty);
				  entrylist.push_back(enrty);
				  GenerateAndZipDataBack(st,entrylist,strFileName,strFilePath);
				   nSendPointCount = entrylist.size();
			  }
			  else							//发送变化
			  {
				  GenerateDTUUpdateInfo_Change((int)tCycle,nChangeSize,nCount,enrty);
				  entryChangelist.push_back(enrty);
				  GenerateAndZipDataBack(st,entryChangelist,strFileName,strFilePath);
				   nSendPointCount = entryChangelist.size();
			  }
		  }

		  if(strFilePath.length()>0)
		  {
			  if(m_bRunDTUEngine)
			  {
				  string strFilePathAnsi = Project::Tools::WideCharToAnsi(strFilePath.c_str());
				  int pos = strFilePathAnsi.find_last_of('\\');
				  string strFileNameAnsi = strFilePathAnsi.substr(pos + 1);
				  string strTime = Project::Tools::SystemTimeToString(st);

				  Json::Value jsonBody;
				  jsonBody["id"] = GetFileInfoPointCont(strFilePathAnsi, strFileNameAnsi);
				  jsonBody["type"] = DTU_CMD_REAL_FILE_SYN;
				  jsonBody["content"] = strFilePathAnsi;

				  DTU_DATA_INFO data;
				  data.strTime = strTime;
				  data.strFileName = strFileNameAnsi;
				  data.nSubType = 0;
				  data.strContent = jsonBody.toStyledString();
					
				  CString strLog;
				  strLog.Format(_T("INFO : Save RealFile(%s).\r\n"),strFileName.c_str());
				  PrintLog(strLog.GetString());
				  return m_dbsession_history->SaveOneSendData(data);
			  }
			  else
			  {
				  if(m_pdtusender && m_bdtuSuccess)
				  {
					  if(m_bdtuSuccess)
					  {
						  DTUSendInfo sDTUInfo;
						  sDTUInfo.tSendTime = st;
						  sDTUInfo.nType = 7;
						  sDTUInfo.nPointCount = nSendPointCount;
						  sDTUInfo.strSendBuf = Project::Tools::WideCharToAnsi(strFilePath.c_str());
						  if(!m_bVecCopying)
							  m_vecSendBuffer.push_back(sDTUInfo);
					  }
				  }
			  }
		  }
		 return true;
	 }
	 return false;
 }

 string CBEOPDataLinkManager::BuildDTUSendData_Change( int nPointStoreType, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nChange,int nCount )
 {
	 if (entrylist.empty()){
		 return "";
	 }

	 string m_sendbuffer;
	 std::ostringstream sqlstream;
	 for (unsigned int i = 0; i < entrylist.size(); i++)
	 {
		 string strvalue;
		 Project::Tools::WideCharToUtf8(entrylist[i].GetValue(),strvalue);
		 sqlstream << nPointStoreType << "," << entrylist[i].GetName() << "," << strvalue << ";";
	 }
	 sqlstream << GenerateDTUUpdateInfo(nPointStoreType,nChange,nCount);
	 m_sendbuffer = sqlstream.str();
	 return m_sendbuffer;
 }

 string CBEOPDataLinkManager::GenerateDTUUpdateInfo_Change( int nPointStoreType,int nUpdateSize,int nCountSize,Beopdatalink::CRealTimeDataEntry& enrty )
 {
	 std::ostringstream sqlstream;
	 string strName = "";
	 switch((POINT_STORE_CYCLE)nPointStoreType)
	 {
	 case E_STORE_NULL:
		 {
			 sqlstream << nUpdateSize << "/" << nCountSize << ";";
			 strName = DTU_UPDATE_ALL;
		 }
		 break;
	 case E_STORE_FIVE_SECOND:
		 {
			 sqlstream<< nUpdateSize << "/" << nCountSize << ";";
			 strName = DTU_UPDATE_S5;
		 }
		 break;
	 case E_STORE_ONE_MINUTE:
		 {
			 sqlstream << nUpdateSize << "/" << nCountSize << ";";
			  strName = DTU_UPDATE_M1;
		 }
		 break;
	 case E_STORE_FIVE_MINUTE:
		 {
			 sqlstream <<  nUpdateSize << "/" << nCountSize << ";";
			 strName = DTU_UPDATE_M5;
		 }
		 break;
	 case E_STORE_ONE_HOUR:
		 {
			 sqlstream << nUpdateSize << "/" << nCountSize << ";";
			 strName = DTU_UPDATE_H1;
		 }
		 break;
	 case E_STORE_ONE_DAY:
		 {
			 sqlstream << nUpdateSize << "/" << nCountSize << ";";
			 strName = DTU_UPDATE_D1;
		 }
		 break;
	 case E_STORE_ONE_MONTH:
		 {
			 sqlstream << nUpdateSize << "/" << nCountSize << ";";
			 strName = DTU_UPDATE_MONTH1;
		 }
		 break;
	 default:
		 break;
	 }
	 enrty.SetName(strName);
	 enrty.SetValue(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));
	 return "";
 }

 bool CBEOPDataLinkManager::AckUpdateExeFile( string strExeName )
 {
	 SYSTEMTIME st;
	 GetLocalTime(&st);

	 if(m_bdtuSuccess)
	 {
		 DTUSendInfo sDTUInfo;
		 sDTUInfo.tSendTime = st;
		 sDTUInfo.nType = 8;
		 sDTUInfo.nSubType = 1;
		 sDTUInfo.strSendBuf = strExeName;
		 if(!m_bVecCopying)
			 m_vecSendBuffer.push_back(sDTUInfo);
		 SetSendDTUEventAndType(0,st);
		 CString strSendInfo;
		 strSendInfo.Format(_T("SEND : DTUServer Update exe file(%s)\r\n"),Project::Tools::AnsiToWideChar(strExeName.c_str()).c_str());
		 SendLogMsg(L"DTUServer",strSendInfo);
	 } 
	 return true;
 }

 bool CBEOPDataLinkManager::AckLostExeFile( string strLostFile )
 {
	 SYSTEMTIME st;
	 GetLocalTime(&st);

	 if(m_bdtuSuccess)
	 {
		 DTUSendInfo sDTUInfo;
		 sDTUInfo.tSendTime = st;
		 sDTUInfo.nType = 8;
		 sDTUInfo.nSubType = 2;
		 sDTUInfo.strSendBuf = strLostFile;
		 if(!m_bVecCopying)
			 m_vecSendBuffer.push_back(sDTUInfo);
		 SetSendDTUEventAndType(0,st);
		 CString strSendInfo;
		 strSendInfo.Format(_T("SEND : DTUServer Update lost file(%s)\r\n"),Project::Tools::AnsiToWideChar(strLostFile.c_str()).c_str());
		 SendLogMsg(L"DTUServer",strSendInfo);
	 }
	 return true;
 }

 void CBEOPDataLinkManager::ReadCSVFileAndSendHistory( string strPath )
 {
	 if(strPath.length() <= 0)
		 return;

	 FILE* pfile = fopen(strPath.c_str(), "r");
	 if (pfile)
	 {
		 fseek(pfile,0,SEEK_END);
		 int dwsize = ftell(pfile);
		 rewind(pfile);
		 char* filebuffer = new char[dwsize];
		 fread(filebuffer, 1, dwsize, pfile);
		 fclose(pfile);

		 wstring strFolder;
		 Project::Tools::GetSysPath(strFolder);
		 strFolder += L"\\backup";
		 std::ostringstream sqlstream_path;

		 vector<string> vec;
		 Project::Tools::SplitString(filebuffer,",",vec);
		 for(int i=0; i<vec.size(); ++i)
		 {

			 string strFileName = vec[i];
			 if(strFileName.length() < 19)
				 continue;

			 //判断文件是否存在的
			 sqlstream_path.str("");
			 sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str())
				 << "\\" << strFileName.substr(11,4)  << strFileName.substr(15,2) 
				 << strFileName.substr(17,2);
			 string strFolder_ = sqlstream_path.str();
			 sqlstream_path  << "\\" << strFileName;
			 string strPath = sqlstream_path.str();
			 if(Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strPath.c_str())))
			 {
				 m_mapSendLostHistory[vec[i]] = 0;
			 }
			 else
			 {
				/* if(ReadDataAndGenerateHistoryFile(strFileName,strFolder_,strPath))
				 {
					 m_mapSendLostHistory[strFileName] = 0;
				 }*/
			 }
		 }

		 delete filebuffer;
		 filebuffer = NULL;
	 }

	 //Ack
	 SYSTEMTIME st;
	 GetLocalTime(&st);

	 int pos = strPath.find_last_of('\\');
	 string strFileName(strPath.substr(pos + 1));

	 if(m_bdtuSuccess)
	 {
		 DTUSendInfo sDTUInfo;
		 sDTUInfo.tSendTime = st;
		 sDTUInfo.nType = 8;
		 sDTUInfo.nSubType = 3;
		 sDTUInfo.strSendBuf = strFileName;
		 if(!m_bVecCopying)
			 m_vecSendBuffer.push_back(sDTUInfo);
		 SetSendDTUEventAndType(0,st);
	 }
 }

 bool CBEOPDataLinkManager::ScanSendHistoryQueue(int nSecond)
 {
	 return true;
 }

 bool CBEOPDataLinkManager::SendOneLostHistoryData()
 {
	 map<string,int>::iterator iter = m_mapSendLostHistory.begin();
	 if(iter != m_mapSendLostHistory.end())
	 {
		 string strFileName = iter->first;
		 if(strFileName.length() <= 0)
			 return false;

		 wstring strFolder;
		 Project::Tools::GetSysPath(strFolder);

		 std::ostringstream sqlstream_path;
		 sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str()) << "\\backup" 
			 << "\\" << strFileName.substr(11,4) << strFileName.substr(15,2) 
			 << strFileName.substr(17,2) << "\\" << strFileName;

		 string strPath = sqlstream_path.str();
		 int nPointCount = GetFileInfoPointCont(strPath,strFileName);
		 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Run SendDTUData->SendOneLostHistoryData \n");
		 m_pdtusender->SendHistoryDataFile(strPath,nPointCount);
		 m_mapSendLostHistory.erase(iter);
		 Sleep(1000);
		 CDebugLog::GetInstance()->DebugLog(E_DEBUG_LOG_DTU,E_DEBUG_LOG_LEVEL_PROCESS,L"DEBUG:Exit SendDTUData \n");
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::ActiveLostSendEvent()
 {
	 if(!m_mapSendLostHistory.empty())
	 {
		 SYSTEMTIME st;
		 GetLocalTime(&st);
		 SetSendDTUEventAndType(0,st);
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::AckReSendLostZipData( string strMinutes,string strFileNames )
 {
	 Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	 //发送DTU数据 
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 vector<string> vec;
		 Project::Tools::SplitString(strFileNames.c_str(),",",vec);

		 wstring strFolder;
		 Project::Tools::GetSysPath(strFolder);
		 strFolder += L"\\backup";
		 std::ostringstream sqlstream_path;

		 for(int i=0; i<vec.size(); ++i)
		 {
			 string strFileName = vec[i];
			 if(strFileName.length() < 19)
				 continue;

			 //判断文件是否存在的
			 sqlstream_path.str("");
			 sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str())
				 << "\\" << strFileName.substr(11,4)  << strFileName.substr(15,2) 
				 << strFileName.substr(17,2);
			 string strFolder_ = sqlstream_path.str();
			 sqlstream_path  << "\\" << strFileName;
				 string strPath = sqlstream_path.str();

			 if(Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strPath.c_str())))
			 {
				 m_mapSendLostHistory[vec[i]] = 0;
			 }
			 else
			 {
				 /*if(ReadDataAndGenerateHistoryFile(strFileName,strFolder_,strPath))
				 {
					  m_mapSendLostHistory[strFileName] = 0;
				 }*/
			 }
		 }

		 //Ack
		 if(m_bdtuSuccess)
		 {
			 SYSTEMTIME st;
			 GetLocalTime(&st);
			 DTUSendInfo sDTUInfo;
			 sDTUInfo.tSendTime = st;
			 sDTUInfo.nType = 8;
			 sDTUInfo.nSubType = 4;
			 sDTUInfo.strSendBuf = strMinutes;
			 if(!m_bVecCopying)
				 m_vecSendBuffer.push_back(sDTUInfo);
			 SetSendDTUEventAndType(0,st);
		 }
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::AckReSendFileData( string strFileName )
 {
	 Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	 //发送DTU数据 
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 if(strFileName.length() <= 0)
			 return false;

		 wstring strFolder;
		 Project::Tools::GetSysPath(strFolder);
		 strFolder += L"\\backup";
		 std::ostringstream sqlstream_path;

		 //判断文件是否存在的
		 sqlstream_path.str("");
		 sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str())
			 << "\\" << strFileName.substr(11,4)  << strFileName.substr(15,2) 
			 << strFileName.substr(17,2) << "\\" << strFileName;
		 string strPath = sqlstream_path.str();
		 if(Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strPath.c_str())))
		 {
			 //Ack
			m_strSendRealFilePath = strPath;	
			m_nSendRealFileCount = GetFileInfoPointCont(strPath,strFileName);

			SYSTEMTIME st;
			GetLocalTime(&st);
			SetSendDTUEventAndType(0,st);
		 }
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::StoreFileInfo( wstring strFolder,wstring strFileName,int nPointCount )
 {
	 strFolder = strFolder + L"\\FilePoint.ini";
	 CString strValue;
	 strValue.Format(_T("%d"),nPointCount);
	 WritePrivateProfileString(L"file", strFileName.c_str(), strValue, strFolder.c_str());
	 return true;
 }

 int CBEOPDataLinkManager::GetFileInfoPointCont( string strPath,string strFileName )
 {
	 if(strPath.length() <= 0 || strFileName.length() <=0 )
		 return 0;

	 int pos = strPath.find_last_of('\\');
	 ostringstream strFilePath_;
	 strFilePath_ << strPath.substr(0,pos) << "\\FilePoint.ini";
	 string strFilePath = strFilePath_.str(); 
	 int nPointCount = 0;
	 CString strName = Project::Tools::AnsiToWideChar(strFileName.c_str()).c_str();
	 wchar_t charPointCount[256];
	 GetPrivateProfileString(L"file", strName.GetString(), L"", charPointCount, 256, Project::Tools::AnsiToWideChar(strFilePath.c_str()).c_str());
	 wstring wstrPointCount = charPointCount;
	 if(wstrPointCount == L"")
		 wstrPointCount = L"0";
	 nPointCount = _wtoi(wstrPointCount.c_str());
	 return nPointCount;
 }

 bool CBEOPDataLinkManager::AckReStartDTU()
 {
	 //
	 SYSTEMTIME st;
	 GetLocalTime(&st);
	 if(m_bdtuSuccess)
	 {
		 DTUSendInfo sDTUInfo;
		 sDTUInfo.tSendTime = st;
		 sDTUInfo.nType = 3;
		 sDTUInfo.nSubType = 8;
		 sDTUInfo.strSendBuf = "";
		 sDTUInfo.strPacketIndex = "14";
		 if(!m_bVecCopying)
			 m_vecSendBuffer.push_back(sDTUInfo);
		 SetSendDTUEventAndType(0,st);
	 }
	 CString strSendInfo = L"RECV : DTUServer Cmd: ReStart DTU. \r\n";
	 SendLogMsg(L"DTUServer",strSendInfo);
	 return true;
 }

 void CBEOPDataLinkManager::GetStoreRealTimeDataEntryList( vector<Beopdatalink::CRealTimeDataEntry> &entrylist )
 {
	 entrylist.clear();
	 EnterCriticalSection(&m_RealTimeData_CriticalSection);
	 for(int i =0;i<m_RealTimeDataEntryList.size();i++)
	 {
		 POINT_STORE_CYCLE ptCycle = m_pointmanager->GetPointSavePeriod(m_RealTimeDataEntryList[i].GetName());
		 if(ptCycle!=E_STORE_NULL || m_RealTimeDataEntryList[i].IsEngineSumPoint())
		 {
			 entrylist.push_back(m_RealTimeDataEntryList[i]);
		 }
	 }
	 LeaveCriticalSection(&m_RealTimeData_CriticalSection);
 }

 bool CBEOPDataLinkManager::UpdateDTUSuccess()
 {
	 if(m_pdtusender && !m_bdtuSuccess)
	 {
		 m_bdtuSuccess = m_pdtusender->GetDTUSuccess();
	 }
	 return true;
 }

 bool CBEOPDataLinkManager::ReadDataAndGenerateHistoryFile( string strFileName,string strFolder,string strPath )
 {
	 //databackup_201506121721.zip
	 if(strFileName.length() < 23)
		 return false;

	 std::ostringstream sqlstream_time,sqlstream_time_start;
	 sqlstream_time << strFileName.substr(11,4)  << "-" << strFileName.substr(15,2)  << "-" << strFileName.substr(17,2) 
		 << " " << strFileName.substr(19,2)  << ":" << strFileName.substr(21,2)  << ":00";
	 string strEndTime = sqlstream_time.str();
	 SYSTEMTIME st,st_start;
	 Project::Tools::String_To_SysTime(strEndTime,st);
	 sqlstream_time_start << st.wYear << "-" << st.wMonth << "-" << st.wDay << " 00:00:00";

	 //先扫描本地日期文件夹 选取时间最近的一个文件包
	 vector<wstring> vecAllFile;
	 CString strFilePath,strFileFolder;
	 strFileFolder = Project::Tools::AnsiToWideChar(strFolder.c_str()).c_str();
	 strFilePath.Format(_T("%s\\*.zip"),strFileFolder);

	 CFileFind fileFinder;
	 BOOL bWorking = fileFinder.FindFile(strFilePath);
	 hash_map<wstring,int>	mapAllFile;
	 CString strFileName_ = Project::Tools::AnsiToWideChar(strFileName.c_str()).c_str();
	 while (bWorking)
	 {   
		 bWorking = fileFinder.FindNextFile();
		 if (fileFinder.IsDots())
		 {
			 continue;
		 }
		 CString strFName = fileFinder.GetFileName();
		 if(strFName.GetLength() > 0 && strFName <= strFileName_)
			 vecAllFile.push_back(strFName.GetString());
	 }
	 fileFinder.Close();

	 map<string,wstring> mapHistoryValue;
	 int nFileSize = vecAllFile.size();
	 bool bHasFile = false;
	 if(nFileSize > 0)		//若有文件包,先取时间最新的一个包，再拿该时间差数据
	 {
		 std::sort(vecAllFile.begin(), vecAllFile.end(), new_time);
		 wstring strRecentFile = vecAllFile[nFileSize-1];
		 string strRecentFile_ = Project::Tools::WideCharToAnsi(strRecentFile.c_str());
		 sqlstream_time_start.str("");
		 sqlstream_time_start << strRecentFile_.substr(11,4)  << "-" << strRecentFile_.substr(15,2)  << "-" << strRecentFile_.substr(17,2) 
			 << " " << strRecentFile_.substr(19,2)  << ":" << strRecentFile_.substr(21,2)  << ":00";
		 strFilePath.Format(_T("%s\\%s"),strFileFolder,strRecentFile.c_str());
		
		 //解压文件 根据;划分值
		 HZIP hz = OpenZip(strFilePath,0);
		 SetUnzipBaseDir(hz,strFileFolder);
		 ZIPENTRY ze;
		 GetZipItem(hz,-1,&ze); 
		 int numitems=ze.index;
		 bool bDeleteZipFile = false;
		 for (int j=0; j<numitems; ++j)
		 {
			 GetZipItem(hz,j,&ze);
			 ZRESULT zr = UnzipItem(hz,j,ze.name);
			 if(zr == ZR_OK)
			 {
				 //处理解压出来的.csv文件
				 CString strCSVPath;
				 strCSVPath.Format(_T("%s\\%s"),strFileFolder,ze.name);
				 FILE* pfile = fopen(Project::Tools::WideCharToAnsi(strCSVPath).c_str(), "r");
				 if (pfile)
				 {
					 fseek(pfile,0,SEEK_END);
					 int dwsize = ftell(pfile);
					 rewind(pfile);
					 char* filebuffer = new char[dwsize];
					 fread(filebuffer, 1, dwsize, pfile);
					 fclose(pfile);

					 char* p = strtok((char*)filebuffer, ";");
					 while(p){
						 //逗号分隔
						 vector<string> vec;
						 SplitString(p,",",vec);
						 if(vec.size() >= 2)
						 {
							 mapHistoryValue[vec[0]] = Project::Tools::AnsiToWideChar(vec[1].c_str());
						 }
						 p = strtok(NULL, ";");
					 }
					 delete filebuffer;
					 filebuffer = NULL;

					 //删除csv
					 DeleteFile(strCSVPath);
					 bHasFile = true;
				 }
			 }
		 }
		 CloseZip(hz);
	 }
	 
	 string strStartTime = sqlstream_time_start.str();
	 Project::Tools::String_To_SysTime(strStartTime,st_start);
	 vector<Beopdatalink::CRealTimeDataEntry> vecHistory;
	 if(m_dbsession_history)
	 {
		 if(m_dbsession_history->IsConnected())
		 {
			 m_dbsession_history->GetAllHistoryValueByTime(st_start,st,vecHistory,bHasFile);
		 }
	 }

	 for(int i=0; i<vecHistory.size(); ++i)
	 {
		 Beopdatalink::CRealTimeDataEntry entry = vecHistory[i];
		 mapHistoryValue[entry.GetName()] = entry.GetValue();
	 }

	 if(mapHistoryValue.size() > 0)
	 {
		 vecHistory.clear();
		 map<string,wstring>::iterator iter =  mapHistoryValue.begin();
		 while(iter != mapHistoryValue.end())
		 {
			 Beopdatalink::CRealTimeDataEntry entry;
			 entry.SetName(iter->first);
			 entry.SetValue(iter->second);
			 entry.SetTimestamp(st);
			 vecHistory.push_back(entry);
			 iter++;
		 }

		 //生成Zip文件
		 if(GenerateAndZipDataBack(st,vecHistory,Project::Tools::AnsiToWideChar(strFileName.c_str()),Project::Tools::AnsiToWideChar(strPath.c_str())))
			 return true;
	 }


	 return false;
 }

 void CBEOPDataLinkManager::SplitString( const std::string& strsource, const char* sep, std::vector<string>& resultlist )
 {
	 Project::Tools::Scoped_Lock<Mutex> scopelock(m_toollock);
	 if( !sep)
		 return;

	 resultlist.clear();
	 memset(m_cSpiltBuffer,0,DATA_BUFFER_SIZE);
	 int length = strsource.length();
	 memcpy(m_cSpiltBuffer,strsource.c_str(),length);

	 char* token = NULL;
	 char* nexttoken = NULL;
	 token = strtok_s(m_cSpiltBuffer, sep, &nexttoken);

	 while(token != NULL)
	 {
		 string str = token;
		 resultlist.push_back(str);
		 token = strtok_s(NULL, sep, &nexttoken);
	 }

	 if(strsource.length() > 1 && strsource[length-1] == sep[0])
	 {
		 resultlist.push_back("");
	 }
 }

 unsigned int WINAPI CBEOPDataLinkManager::ThreadProjectDB( LPVOID lpVoid )
 {
	 CBEOPDataLinkManager* pProjectDBComm = reinterpret_cast<CBEOPDataLinkManager*>(lpVoid);
	 if (NULL == pProjectDBComm)
	 {
		 return 0;
	 }
	 return 1;
 }

 bool CBEOPDataLinkManager::CheckDiskFreeSpace( string strPath )
 {
	 if(strPath.length() >= 2 )
	 {
		 unsigned __int64 i64FreeBytesToCaller;  
		 unsigned __int64 i64TotalBytes;  
		 unsigned __int64 i64FreeBytes;  

		 string strBase = strPath.substr(0,2);
		 bool fResult = ::GetDiskFreeSpaceEx (  
			 Project::Tools::AnsiToWideChar(strBase.c_str()).c_str(),  
			 (PULARGE_INTEGER)&i64FreeBytesToCaller,  
			 (PULARGE_INTEGER)&i64TotalBytes,  
			 (PULARGE_INTEGER)&i64FreeBytes);  
		 //GetDiskFreeSpaceEx函数，可以获取驱动器磁盘的空间状态,函数返回的是个BOOL类型数据  
		 if(fResult)
		 {
			 int nCount = (float)i64TotalBytes/1024/1024/1024;
			 int nSpace = (float)i64FreeBytesToCaller/1024/1024/1024;
			 if(nSpace <= 0)
			 {
				 nSpace = (float)i64FreeBytesToCaller/1024/1024;
				 CString strWarning;
				 strWarning.Format(_T("Disk Space Warning(Free:%dM/Total:%dG).\n"),nSpace,nCount);
				 if(nSpace <= 100)
					 return true;
			 }
			 else
			 {
				 CString strWarning;
				 strWarning.Format(_T("Disk Space(Free:%dG/Total:%dG).\n"),nSpace,nCount);
				 TRACE(strWarning);
			 }
		 }
		 return false;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::UpdateWatch()
 {
	 if(m_dbsession_history && m_dbsession_history->IsConnected())
	 {
		 if(m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"updatedog",L"0") == L"1")
		 {
			 wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
			 GetSysPath(strFolder);
			 strFolder += L"\\fileupdate";
			 if(FindOrCreateFolder(strFolder))
			 {
				 strUpdateFolder = strFolder + L"\\update";
				 strBacFolder = strFolder + L"\\back";

				 if(m_dbsession_history && m_dbsession_history->IsConnected())
				 {
					 strS3dbPath =  m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"corefilepath");
					 if(strS3dbPath.length() <= 0)
						 return false;

					 CString strFolderPath = strS3dbPath.c_str();
					 strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
					 if(FindOrCreateFolder(strUpdateFolder))
					 {
						 //找到更新文件
						 if(FindFile(strUpdateFolder.c_str(),_T("BEOPWatch.exe")))
						 {
							 //拷贝更新文件到运行目录  以.bak结尾 并删除更新文件
							 CString strUpdatePath;
							 strUpdatePath.Format(_T("%s\\BEOPWatch.exe"),strUpdateFolder.c_str());

							 CString strCurrentBacPath;
							 strCurrentBacPath.Format(_T("%s\\BEOPWatch.exe.bak"),strFolderPath);
							 if(!DelteFile(strUpdatePath,strCurrentBacPath))
								 return false;

							 //备份当前运行的Watch文件到back文件夹
							 CString strCurrentPath;
							 strCurrentPath.Format(_T("%s\\BEOPWatch.exe"),strFolderPath);

							 SYSTEMTIME sNow;
							 GetLocalTime(&sNow);
							 CString strTime;
							 strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

							 if(FindOrCreateFolder(strBacFolder))
							 {
								 CString strBackPath;
								 strBackPath.Format(_T("%s\\BEOPWatch_%s.exe"),strBacFolder.c_str(),strTime);

								 //当前exe存在拷贝一份
								 if(FindFile(strFolderPath,_T("BEOPWatch.exe")))
								 {
									 if(!CopyFile(strCurrentPath,strBackPath,FALSE))
										 return false;

									 CString strLog;
									 strLog.Format(_T("Backup Before Update Watch(%s)"),strBackPath);
									 OutPutUpdateLog(strLog.GetString());
								 }
							 }

							 //关闭当前运行Watch
							 CloseWatchDog();

							 //删除当前运行Watch
							 DelteFile(strCurrentPath);

							 //重命名.bak为
							 rename(Project::Tools::WideCharToAnsi(strCurrentBacPath).c_str(),Project::Tools::WideCharToAnsi(strCurrentPath).c_str());

							 //重启Watch
							 RestartByMaster(E_SLAVE_UPDATE,E_REASON_UPDATE,strCurrentPath.GetString());

							 CString strLog;
							 strLog.Format(_T("Update And Restart Watch(%s)"),strUpdatePath);
							 OutPutUpdateLog(strLog.GetString());

							 m_dbsession_history->WriteCoreDebugItemValue(L"updatedog",L"0");
							 m_dbsession_history->WriteCoreDebugItemValue(L"updatedogfinish",L"1");
						 }
					 }
				 }
			 }
		 }
	 }
	 return true;
 }

 bool CBEOPDataLinkManager::CloseWatchDog()
 {
	 bool bResult = false;
	 if (m_PrsV.FindProcessByName_other(g_NAME_Update))
	 {
		 if(m_PrsV.CloseApp(g_NAME_Update))
		 {
			 bResult = true;
		 }
		 else
		 {
			 bResult = false;
		 }
	 }
	 return bResult;
 }

 bool CBEOPDataLinkManager::OutPutUpdateLog( wstring strLog )
 {
	 wstring wstrFileFolder;
	 Project::Tools::GetSysPath(wstrFileFolder);
	 SYSTEMTIME sNow;
	 GetLocalTime(&sNow);
	 wstrFileFolder += L"\\fileupdate";
	 if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
	 {
		 char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
		 setlocale( LC_ALL, "chs" );

		 CString strOut;
		 wstring strTime;
		 Project::Tools::SysTime_To_String(sNow,strTime);
		 strOut.Format(_T("%s ::%s\n"),strTime.c_str(),strLog.c_str());

		 CString strLogPath;
		 strLogPath.Format(_T("%s\\fileupdate.log"),wstrFileFolder.c_str());

		 //记录Log
		 CStdioFile	ff;
		 if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		 {
			 ff.Seek(0,CFile::end);
			 ff.WriteString(strOut);
			 ff.Close();
			 setlocale( LC_CTYPE, old_locale ); 
			 free( old_locale ); 
			 return true;
		 }
		 setlocale( LC_CTYPE, old_locale ); 
		 free( old_locale ); 
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::UpdateDebug()
 {
	 if(m_dbsession_history && m_dbsession_history->IsConnected())
	 {
		 wstring strDebugBacnet = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debugbacnet",L"0");
		 if(m_bacnetengine)
		 {
			 m_bacnetengine->SetDebug(_wtoi(strDebugBacnet.c_str()));
		 }

		 wstring strDebugModbus = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debugmodbus",L"0");
		 map<wstring,CModbusEngine*>::iterator iterModbus =  m_mapMdobusEngine.begin();
		 while(iterModbus != m_mapMdobusEngine.end())
		 {
			 iterModbus->second->SetDebug(_wtoi(strDebugModbus.c_str()));
			 ++iterModbus;
		 }

		 wstring strDebugOPC = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debugopc",L"0");
		 wstring strOPCCheck = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opccheckreconnect",L"5");
		 wstring strOPCReconnect = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcreconnect",L"30");
		 wstring strReconnectMinute = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcreconnectignore",L"0");
		 wstring strEnableSecurity = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"enableSecurity",L"0");
		 wstring strOPCMainThreadOPCPollSleep = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"opcmainpollsleep",L"10");
		 if(m_opcengine)
		 {
			 m_opcengine->SetDebug(_wtoi(strDebugOPC.c_str()),_wtoi(strOPCCheck.c_str()),_wtoi(strOPCReconnect.c_str()),_wtoi(strReconnectMinute.c_str()),_wtoi(strEnableSecurity.c_str()),_wtoi(strOPCMainThreadOPCPollSleep.c_str()));
		 }

		 map<int,COPCEngine*>::iterator iter = m_mapOPCEngine.begin();
		 while(iter != m_mapOPCEngine.end())
		 {
			 iter->second->SetDebug(_wtoi(strDebugOPC.c_str()),_wtoi(strOPCCheck.c_str()),_wtoi(strOPCReconnect.c_str()),_wtoi(strReconnectMinute.c_str()),_wtoi(strEnableSecurity.c_str()),_wtoi(strOPCMainThreadOPCPollSleep.c_str()));
			 ++iter;
		 }
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::AckSynSystemTime( string strSystemTime )
 {
	 wstring strTime = Project::Tools::AnsiToWideChar(strSystemTime.c_str());
	 COleDateTime oleNow;
	 Project::Tools::String_To_OleTime(strTime,oleNow);
	 if(oleNow.GetStatus() == 0)
	 {
		 //
		ImproveProcPriv();
		SYSTEMTIME st;
		Project::Tools::String_To_SysTime(strSystemTime,st);
		/*bool bSetSuccess = SetLocalTime(&st);
		int nErrCode = 0;
		if(!bSetSuccess)
		{
		nErrCode = GetLastError();
		}*/

		bool bSetSuccess = RunBatCmd_SetSystemTime(st);
		if(m_bdtuSuccess && bSetSuccess)
		{
			DTUSendInfo sDTUInfo;
			sDTUInfo.tSendTime = st;
			sDTUInfo.nType = 3;
			sDTUInfo.nSubType = 9;
			sDTUInfo.strSendBuf = "";
			sDTUInfo.strPacketIndex = "0";
			if(!m_bVecCopying)
				m_vecSendBuffer.push_back(sDTUInfo);
			SetSendDTUEventAndType(0,st);
		}
		CString strSendInfo;
		strSendInfo.Format(_T("RECV : DTUServer Cmd: Syn SystemTime(%s). \r\n"),strTime.c_str());
		SendLogMsg(L"DTUServer",strSendInfo);
		return true;
	 }

	 return false;
 }

 bool CBEOPDataLinkManager::UpdatePrivilege()
 {
	 bool bUpdate = false;
	 HANDLE hToken = NULL;
	 //打开当前进程的访问令牌
	 int hRet = OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken);
	 if( hRet)
	 {
		 TOKEN_PRIVILEGES tp;
		 tp.PrivilegeCount = 1;
		 //取得描述权限的LUID
		 LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);
		 tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		 //调整访问令牌的权限
		 bUpdate = AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL);
		 CloseHandle(hToken);
	 }
	 return bUpdate;
 }

 bool CBEOPDataLinkManager::ImproveProcPriv()
 {
	 HANDLE token;
	 //提升权限
	 if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token))
	 {
		 return FALSE;
	 }
	 TOKEN_PRIVILEGES tkp;
	 tkp.PrivilegeCount = 1;
	 ::LookupPrivilegeValue(NULL,SE_SYSTEMTIME_NAME,&tkp.Privileges[0].Luid);
	 tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	 if(!AdjustTokenPrivileges(token,FALSE,&tkp,sizeof(tkp),NULL,NULL))
	 {
		 return FALSE;
	 }
	 CloseHandle(token);
	 return TRUE;
 }

 bool CBEOPDataLinkManager::RunBatCmd_SetSystemTime( SYSTEMTIME st )
 {
	 CString strConfigPath;
	 wstring strPath;
	 Project::Tools::GetSysPath(strPath);
	 strConfigPath.Format(_T("%s\\runsettime.bat"),strPath.c_str());

	 CStdioFile	ff;
	 char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
	 setlocale( LC_ALL, "chs" );
	 if(ff.Open(strConfigPath,CFile::modeCreate|CFile::modeWrite))
	 {
		 ff.WriteString(_T("@echo off\n"));
		 CString str;
		 str.Format(_T("date %d/%02d/%02d & time %02d:%02d:%02d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		 ff.WriteString(str);
		 ff.Close();

		 setlocale( LC_ALL, old_locale );
		 free(old_locale);

		 SHELLEXECUTEINFO ShExecInfo = {0};
		 ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		 ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		 ShExecInfo.hwnd = NULL;
		 ShExecInfo.lpVerb = NULL;
		 ShExecInfo.lpFile = strConfigPath;	
		 ShExecInfo.lpParameters = _T("");	
		 ShExecInfo.lpDirectory = _T("");
		 ShExecInfo.nShow = SW_HIDE;
		 ShExecInfo.hInstApp = NULL;

		 int nResult = ShellExecuteEx(&ShExecInfo);
		 WaitForSingleObject(ShExecInfo.hProcess, INFINITE); 

		 DWORD dwExitCode;
		 BOOL bOK = GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
		 return bOK;
	 }
	 setlocale( LC_ALL, old_locale );
	 free(old_locale);
	 return false;
 }

 bool CBEOPDataLinkManager::RestartWatch()
 {
	 if(m_dbsession_history && m_dbsession_history->IsConnected())
	 {
		 wstring strS3dbPath =  m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"corefilepath");

		 if(strS3dbPath.length() <= 0)
			 return false;

		 CString strFolderPath = strS3dbPath.c_str();
		 strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
		 if(FindOrCreateFolder(strFolderPath.GetString()))
		 {
			 //找到更新文件
			 if(FindFile(strFolderPath,_T("domwatch.exe")))
			 {
				 CString strCurrentPath;
				 strCurrentPath.Format(_T("%s\\domwatch.exe"),strFolderPath);
				 //重启Watch
				 if(RestartByMaster(E_SLAVE_UPDATE,E_REASON_REMOTE,strCurrentPath.GetString()))
				 {
					 SYSTEMTIME st;
					 GetLocalTime(&st);
					 if(m_bdtuSuccess)
					 {
						 DTUSendInfo sDTUInfo;
						 sDTUInfo.tSendTime = st;
						 sDTUInfo.nType = 3;
						 sDTUInfo.nSubType = 4;
						 sDTUInfo.strSendBuf = "";
						 sDTUInfo.strPacketIndex = "12";
						 if(!m_bVecCopying)
							 m_vecSendBuffer.push_back(sDTUInfo);
						 SetSendDTUEventAndType(0,st);
					 }
					 CString strSendInfo = L"RECV : DTUServer Cmd: Restart Watch. \r\n";
					 SendLogMsg(L"DTUServer",strSendInfo);

					 return true;
				 }
			 }
		 }
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::AckCoreVersion()
 {
	 if(m_dbsession_history && m_dbsession_history->IsConnected())
	 {
		 wstring strVersion =  m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"version");

		 SYSTEMTIME st;
		 GetLocalTime(&st);
		 if(m_bdtuSuccess)
		 {
			 DTUSendInfo sDTUInfo;
			 sDTUInfo.tSendTime = st;
			 sDTUInfo.nType = 3;
			 sDTUInfo.nSubType = 10;
			 sDTUInfo.strSendBuf = "";
			 sDTUInfo.strPacketIndex = Project::Tools::WideCharToAnsi(strVersion.c_str());
			 if(!m_bVecCopying)
				 m_vecSendBuffer.push_back(sDTUInfo);
			 SetSendDTUEventAndType(0,st);
		 }
		 CString strSendInfo = L"RECV : DTUServer Cmd: Query Version. \r\n";
		 SendLogMsg(L"DTUServer",strSendInfo);
	 }
	 return true;
 }

 vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetOPCServer( vector<DataPointEntry> opcpoint ,int nMaxSize,wstring strDefaultIP)
 {
	 vector<vector<DataPointEntry>> vecOPCPList;
	 m_vecOPCSer.clear();
	 for(int i=0; i<opcpoint.size(); i++)
	 {
		 //
		 wstring strIP = opcpoint[i].GetParam(6);
		 wstring strProgName = opcpoint[i].GetParam(3);
		 if(strProgName.empty())
			 continue;
		 if(strIP.empty())
			strIP = strDefaultIP;
		 int nPos = IsExistInVec(strIP,strProgName,m_vecOPCSer,nMaxSize);
		 if(nPos==m_vecOPCSer.size())	//不存在
		 {
			 OPCServer pServer;
			 pServer.strIP = strIP;
			 pServer.strServerName = strProgName;
			 pServer.nCount = 1;
			 m_vecOPCSer.push_back(pServer);

			 vector<DataPointEntry> vecPList;
			 vecOPCPList.push_back(vecPList);
		 }
		 vecOPCPList[nPos].push_back(opcpoint[i]);		
	 }
	 return vecOPCPList;
 }

 bool CBEOPDataLinkManager::SendRealData( POINT_STORE_CYCLE nType )
 {
	 //发送DTU数据 
	 if(m_pdtusender && m_bdtuSuccess)
	 {
		 vector<Beopdatalink::CRealTimeDataEntry> vecSynDataList;
		 if(nType == E_STORE_NULL)
		 {
			 GetStoreRealTimeDataEntryList(vecSynDataList);
		 }
		 else
		 {
			 GetRealTimeDataEntryList(vecSynDataList, nType);
		 }

		 if(vecSynDataList.size()>0)
		 {
			 SYSTEMTIME st;
			 GetLocalTime(&st);

			 string strSendBuf = BuildDTUSendData((int)nType,vecSynDataList,vecSynDataList.size());
			 if(strSendBuf.size()>0)
			 {		
				 if(m_bdtuSuccess)
				 {
					 DTUSendInfo sDTUInfo;
					 sDTUInfo.tSendTime = st;
					 sDTUInfo.nType = 0;
					 sDTUInfo.strSendBuf = strSendBuf;
					 if(!m_bVecCopying)
						 m_vecSendBuffer.push_back(sDTUInfo);

					 SetSendDTUEventAndType(3,st);
				 }
			 }
		 }
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::GenerateDTURealDataS5( const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle,int nSendAll )
 {
	 //发送DTU数据   首个;			1s 及 5s中数据不发送   或者TCP网络发送模式  只更新1分钟数据
	 CString strDebugInfo;
	 vector<Beopdatalink::CRealTimeDataEntry> entrylist,entryChangelist;
	 GetStoreRealTimeDataEntryList(entrylist);
	 int nCount = entrylist.size();
	 if(nCount <= 0)
		 return false;

	 Beopdatalink::CRealTimeDataEntry enrty;
	 int nSendPointCount = nCount;
	 wstring strFileName,strFilePath;
	 GenerateAndZipDataBackS5(st,entrylist,strFileName,strFilePath);
	 nSendPointCount = entrylist.size();

	 if(strFilePath.length()>0 && (m_pdtusender && m_bdtuSuccess))
	 {
		 if(m_bdtuSuccess)
		 {
			 DTUSendInfo sDTUInfo;
			 sDTUInfo.tSendTime = st;
			 sDTUInfo.nType = 7;
			 sDTUInfo.nPointCount = nSendPointCount;
			 sDTUInfo.strSendBuf = Project::Tools::WideCharToAnsi(strFilePath.c_str());
			 if(!m_bVecCopying)
				 m_vecSendBuffer.push_back(sDTUInfo);
			  return true;
		 }
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::GenerateAndZipDataBackS5( const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,wstring& strFileName_,wstring& strFilePath_ )
 {
	 wstring strBackupFileFolder,strBackupFileName;
	 strFileName_ = L"";
	 strFilePath_ = L"";
	 if(GenerateDataBackS5(st,strBackupFileName,strBackupFileFolder,entrylist))
	 {
		 CString strZipPath,strFileName,strFilePath;
		 strZipPath.Format(_T("%s\\%s.zip"),strBackupFileFolder.c_str(),strBackupFileName.c_str());
		 strFileName.Format(_T("%s.csv"),strBackupFileName.c_str());
		 strFilePath.Format(_T("%s\\%s"),strBackupFileFolder.c_str(),strFileName);

		 strFileName_ = strBackupFileName;
		 strFilePath_ = strZipPath.GetString();

		 HZIP hz = CreateZip(strZipPath,NULL);
		 ZipAdd(hz,strFileName,strFilePath);
		 CloseZip(hz);
		 DeleteFile(strFilePath);

		 //存储文件
		 strFileName.Format(_T("%s.zip"),strBackupFileName.c_str());
		 StoreFileInfo(strBackupFileFolder,strFileName.GetString(),entrylist.size());
		 return true;
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::GenerateDataBackS5( const SYSTEMTIME &st, wstring& strFileName, wstring& strFileFolder, vector<Beopdatalink::CRealTimeDataEntry> &entrylist )
 {
	 if(entrylist.size() <= 0)
		 return false;

	 int nCount = 0;
	 std::ostringstream sqlstream;
	 vector<string> vecContent;
	 for (unsigned int i = 0; i < entrylist.size(); i++)
	 {
		 string strvalue;
		 Project::Tools::WideCharToUtf8(entrylist[i].GetValue(),strvalue);
		 sqlstream << entrylist[i].GetName() << "," << strvalue << ";";
		 if(nCount == 1000)
		 {
			 string sql_temp = sqlstream.str();
			 vecContent.push_back(sql_temp);
			 sqlstream.str("");
			 nCount = 0;
		 }
		 nCount++;
	 }
	 string sql_temp = sqlstream.str();
	 vecContent.push_back(sql_temp);

	 wstring strPath;
	 GetSysPath(strPath);
	 strPath += L"\\backup";
	 if(Project::Tools::FindOrCreateFolder(strPath))
	 {
		 CString strFolder;
		 strFolder.Format(_T("%s\\%d%02d%02d"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);
		 strPath = strFolder.GetString();
		 if(Project::Tools::FindOrCreateFolder(strPath))
		 {
			 CString strBackFilePath,strBackFileName;
			 strBackFileName.Format(_T("databackup_%d%02d%02d%02d%02d%02d"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
			 strBackFilePath.Format(_T("%s\\%s.csv"),strPath.c_str(),strBackFileName);
			 strFileName = strBackFileName.GetString();
			 strFileFolder = strPath;
			 char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			 setlocale( LC_ALL, "chs" );
			 CStdioFile	ff;
			 if(ff.Open(strBackFilePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			 {
				 CString strContent;
				 for(int j=0; j<vecContent.size(); ++j)
				 {
					 //strContent = Project::Tools::AnsiToWideChar(vecContent[j].c_str()).c_str();
					 wstring strValue;
					 Project::Tools::UTF8ToWideChar(vecContent[j],strValue);
					 strContent = strValue.c_str();
					 ff.Seek(0,CFile::end);
					 ff.WriteString(strContent);
				 }
				 ff.Close();
				 setlocale( LC_CTYPE, old_locale ); 
				 free( old_locale ); 
				 return true;
			 }
			 setlocale( LC_CTYPE, old_locale ); 
			 free( old_locale );
		 }
		 return false;
	 }
 }

 bool CBEOPDataLinkManager::SaveBacnetInfo()
 {
	 if(!m_bUpdateBacnetInfo && m_bacnetengine)
	 {
		 COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleCoreStart;
		 if(oleSpan.GetMinutes() >= 5)
		 {
			 m_bacnetengine->SaveBacnetInfo();
			 m_bUpdateBacnetInfo = true;
			 return true;
		 }
	 }
	 return false;
 }

 void CBEOPDataLinkManager::GetSqliteValueSets(vector<pair<wstring, wstring> >& sqlitevaluesets)
 {
	 map<wstring,CSqliteEngine*>::iterator iter = m_mapSqliteEngine.begin();
	 while(iter != m_mapSqliteEngine.end())
	 {
		 iter->second->GetValueSet(sqlitevaluesets);
		 iter++;
	 }
 }

 void	CBEOPDataLinkManager::GetThirdPartyValueSets(vector<pair<wstring, wstring> >& valuesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_ThirdParty(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 
		 }
		 else if(pentry.IsThirdPartyPoint())
		 {
			 valuesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
		 }
	 }
 }

 void	CBEOPDataLinkManager::GetOPCUAValueSets(vector<pair<wstring, wstring> >& modbusvaluesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_OPCUA(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsOPCUAPoint())
		 {
			 modbusvaluesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }


 void	CBEOPDataLinkManager::GetBacnetPyValueSets(vector<pair<wstring, wstring> >& modbusvaluesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_BacnetPy(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsBacnetPyPoint())
		 {
			 modbusvaluesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }

 //讀取modbusEquipment點的实时表，该表由另一个读取引擎维护
 void	CBEOPDataLinkManager::GetModbusEquipmentValueSets(vector<pair<wstring, wstring> >& modbusvaluesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_ModbusEquipment(tempAllList);

	 CString strLog;
	 strLog.Format(_T("GetModbusEquipmentValueSets : %d count points\r\n"), tempAllList.size());
	 PrintLog(strLog.GetString());

	 for(int i=0;i<tempAllList.size();i++)
	 {
		wstring wstrPointName =  tempAllList[i].first;
		DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		if(pentry.GetShortName().length()==0)
		{
			CString strLog;
		//	PrintLog(strLog.GetString());
		}
		else if(pentry.IsModbusEquipmentPoint())
		{
			modbusvaluesets.push_back(tempAllList[i]);
		}
		else
		{
		//	CString strLog;
		//	PrintLog(strLog.GetString());
		}
	 }
 }

 //讀取modbusEquipment點的实时表，该表由另一个读取引擎维护
 void	CBEOPDataLinkManager::GetPersagyControllerValueSets(vector<pair<wstring, wstring> >& valuesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_PersagyController(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsPersagyControllerPoint())
		 {
			 valuesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }


 void	CBEOPDataLinkManager::GetObixValueSets(vector<pair<wstring, wstring> >& valuesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_Obix(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsObixPoint())
		 {
			 valuesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }


 void	CBEOPDataLinkManager::GetLogixValueSets(vector<pair<wstring, wstring> >& valuesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_Logix(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsLogixPoint())
		 {
			 valuesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }



 void	CBEOPDataLinkManager::GetABSLCValueSets(vector<pair<wstring, wstring> >& valuesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_ABSLC(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsABSLCPoint())
		 {
			 valuesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }



 void	CBEOPDataLinkManager::GetMoxaTCPServerValueSets(vector<pair<wstring, wstring> >& valuesets)
 {
	 vector<pair<wstring, wstring> > tempAllList;
	 m_dbsession_realtime->ReadRealtimeData_Input_MoxaTCPServer(tempAllList);
	 for(int i=0;i<tempAllList.size();i++)
	 {
		 wstring wstrPointName =  tempAllList[i].first;
		 DataPointEntry pentry = m_pointmanager->GetEntry(wstrPointName);
		 if(pentry.GetShortName().length()==0)
		 {
			 CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
		 else if(pentry.IsMoxaTCPServerPoint())
		 {
			 valuesets.push_back(tempAllList[i]);
		 }
		 else
		 {
			 //	CString strLog;
			 //	PrintLog(strLog.GetString());
		 }
	 }
 }


 bool CBEOPDataLinkManager::WriteSqlitePoint(const Beopdatalink::CRealTimeDataEntry& entry)
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 wstring strSer = pentry.GetParam(0) + pentry.GetParam(2);
	 map<wstring,CSqliteEngine*>::iterator iter = m_mapSqliteEngine.find(strSer);
	 if(iter != m_mapSqliteEngine.end())
	 {
		 if(m_mapSqliteEngine[strSer]->SetValue(pointname, entry.GetValue()))
		 {
			 return true;
		 }
	 }

	 return false;
 }

 bool CBEOPDataLinkManager::InitSqliteEngine(vector<DataPointEntry> sqlitepoint)
 {
	 if(sqlitepoint.empty())
		 return false;

	 vector<vector<DataPointEntry>> vecServerList = GetSqliteServer(sqlitepoint);
	 for(int i=0; i<vecServerList.size(); i++)
	 {
		 if(!vecServerList[i].empty())
		 {
			 CSqliteEngine* pSqliteEngine = new CSqliteEngine(this);
			 pSqliteEngine->SetPointList(vecServerList[i]);
			 pSqliteEngine->SetLogSession(m_dbsession_log);
			 pSqliteEngine->SetIndex(i);
			 wstring strSer = vecServerList[i][0].GetParam(0) + vecServerList[i][0].GetParam(2);
			 if(!pSqliteEngine->Init())
			 {
				 CString strFailedInfo;
				 strFailedInfo.Format(L"ERROR: Init Sqlite Engine(%s) Failed.\r\n", strSer.c_str());
				 SendLogMsg(L"", strFailedInfo);
			 }
			 m_mapSqliteEngine[strSer] = pSqliteEngine;
		 }
	 }
	 return true;
 }

 vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetSqliteServer(vector<DataPointEntry> sqlitepoint)
 {
	 vector<vector<DataPointEntry>> veSqlitePList;
	 m_vecSqliteSer.clear();
	 for(int i=0; i<sqlitepoint.size(); i++)
	 {
		 wstring strPath = sqlitepoint[i].GetParam(1);
		 wstring strTName = sqlitepoint[i].GetParam(3);
		 if(strPath.empty() || strTName.empty())
			 continue;

		 int nPos = IsExistInVec(strPath,strTName,m_vecSqliteSer);
		 if(nPos==m_vecSqliteSer.size())	//不存在
		 {
			 SqliteServer pServer;
			 pServer.strFilePath = strPath;
			 pServer.strFileTable = strTName;
			 m_vecSqliteSer.push_back(pServer);

			 vector<DataPointEntry> vecPList;
			 veSqlitePList.push_back(vecPList);
		 }
		 veSqlitePList[nPos].push_back(sqlitepoint[i]);	
	 }
	 return veSqlitePList;
 }

 bool CBEOPDataLinkManager::RestartLogic()
 {
	 if(m_dbsession_history && m_dbsession_history->IsConnected())
	 {
		 wstring strS3dbPath =  m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"corefilepath");

		 if(strS3dbPath.length() <= 0)
			 return false;

		 CString strFolderPath = strS3dbPath.c_str();
		 strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
		 if(FindOrCreateFolder(strFolderPath.GetString()))
		 {
			 //找到更新文件
			 if(FindFile(strFolderPath,_T("domlogic.exe")))
			 {
				 CString strCurrentPath;
				 strCurrentPath.Format(_T("%s\\domlogic.exe"),strFolderPath);
				 //重启Watch
				 if(RestartByMaster(E_SLAVE_LOGIC,E_REASON_REMOTE,strCurrentPath.GetString()))
				 {
					 SYSTEMTIME st;
					 GetLocalTime(&st);
					 if(m_bdtuSuccess)
					 {
						 DTUSendInfo sDTUInfo;
						 sDTUInfo.tSendTime = st;
						 sDTUInfo.nType = 3;
						 sDTUInfo.nSubType = 11;
						 sDTUInfo.strSendBuf = "";
						 sDTUInfo.strPacketIndex = "12";
						 if(!m_bVecCopying)
							 m_vecSendBuffer.push_back(sDTUInfo);
						 SetSendDTUEventAndType(0,st);
					 }
					 CString strSendInfo = L"RECV : DTUServer Cmd: Restart Logic. \r\n";
					 SendLogMsg(L"DTUServer",strSendInfo);

					 return true;
				 }
			 }
		 }
	 }
	 return false;
 }

 bool CBEOPDataLinkManager::RestartByMaster( E_SLAVE_MODE nRestartExe, E_SLAVE_REASON nReason ,wstring strPath)
 {
	 if (CDogTcpCtrl::GetInstance()->GetDogRestartFunctonOK())		//dog连接成功 通过dog更新
	 {
		 if(CDogTcpCtrl::GetInstance()->SendRestart(nRestartExe,nReason))
		 {
			 //等待回应的机制 5秒
			 int nWaitingCount = 50; 
			 bool bRestartUpdate = false;
			 bool bRestartCore = false;
			 bool bRestartLogic = false;
			 bool bRestartMaster = false;
			 while(nWaitingCount>=0)
			 {
				 CDogTcpCtrl::GetInstance()->GetRestartSuccess(bRestartCore,bRestartLogic,bRestartUpdate,bRestartMaster);
				 switch(nRestartExe)
				 {
				 case E_SLAVE_CORE:
					 {
						 if(bRestartCore)
						 {
							 CDogTcpCtrl::GetInstance()->SetRestartCoreSuccess(false);
							 return true;
						 }
						 break;
					 }
				 case E_SLAVE_LOGIC:
					 {
						 if(bRestartLogic)
						 {
							 CDogTcpCtrl::GetInstance()->SetRestartLogicSuccess(false);
							 return true;
						 }
						 break;
					 }
				 case E_SLAVE_UPDATE:
					 {
						 if(bRestartUpdate)
						 {
							 CDogTcpCtrl::GetInstance()->SetRestartUpdateSuccess(false);
							 return true;
						 }
						 break;
					 }
				 case E_SLAVE_MASTER:
					 {
						 if(bRestartMaster)
						 {
							 CDogTcpCtrl::GetInstance()->SetRestartMasterSuccess(false);
							 return true;
						 }
						 break;
					 }
				 default:
					 break;
				 }
				 Sleep(100);
				 nWaitingCount--;
			 }
		 }
		 return false;
	 }
	 else
	 {
		 switch(nRestartExe)
		 {
		 case E_SLAVE_CORE:
			 {
				 m_PrsV.CloseApp(g_NAME_Core);
				 Sleep(1000);
				 return m_PrsV.OpenApp(strPath.c_str());
			 }
		 case E_SLAVE_LOGIC:
			 {
				 m_PrsV.CloseApp(g_NAME_Logic);
				 Sleep(1000);
				 return m_PrsV.OpenApp(strPath.c_str());
			 }
		 case E_SLAVE_UPDATE:
			 {
				 m_PrsV.CloseApp(g_NAME_Update);
				 Sleep(1000);
				 return m_PrsV.OpenApp(strPath.c_str());
			 }
		 case E_SLAVE_MASTER:
			 {
				 m_PrsV.CloseApp(g_NAME_Master);
				 Sleep(1000);
				 return m_PrsV.OpenApp(strPath.c_str());
			 }
		 default:
			 break;
		 }
		 return false;
	 }
	 return false;
 }

 void CBEOPDataLinkManager::GetSqlServerValueSets( vector<pair<wstring, wstring> >& sqlvaluesets )
 {
	 map<wstring,CSqlServerEngine*>::iterator iter = m_mapSqlServerEngine.begin();
	 while(iter != m_mapSqlServerEngine.end())
	 {
		 iter->second->GetValueSet(sqlvaluesets);
		 iter++;
	 }
 }

 bool CBEOPDataLinkManager::WriteSqlServerPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 wstring strSer = pentry.GetParam(3) + pentry.GetParam(8);
	 map<wstring,CSqlServerEngine*>::iterator iter = m_mapSqlServerEngine.find(strSer);
	 if(iter != m_mapSqlServerEngine.end())
	 {
		 if(m_mapSqlServerEngine[strSer]->SetValue(pointname, entry.GetValue()))
		 {
			 return true;
		 }
	 }

	 return false;
 }

 vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetSqlServer( vector<DataPointEntry> sqlpoint )
 {
	 vector<vector<DataPointEntry>> vecSqlPList;
	 m_vecSqlSer.clear();
	 for(int i=0; i<sqlpoint.size(); i++)
	 {
		 //
		 wstring strIP = sqlpoint[i].GetParam(3);
		 wstring strSchema = sqlpoint[i].GetParam(8);
		 if(strIP.empty() || strSchema.empty())
			 continue;

		 int nPos = IsExistInVec(strIP,strSchema,m_vecSqlSer);
		 if(nPos==m_vecSqlSer.size())	//不存在
		 {
			 SqlServer pServer;
			 pServer.strIP = strIP;
			 pServer.strSchema = strSchema;
			 m_vecSqlSer.push_back(pServer);

			 vector<DataPointEntry> vecPList;
			 vecSqlPList.push_back(vecPList);
		 }
		 vecSqlPList[nPos].push_back(sqlpoint[i]);		
	 }
	 return vecSqlPList;
 }

 bool CBEOPDataLinkManager::WriteWebPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);
	 return false;
 }

 bool CBEOPDataLinkManager::InitWebEngine( vector<DataPointEntry> webpoint )
 {
	 if(webpoint.empty())
		 return false;

	 vector<vector<DataPointEntry>> vecServerList = GetWebServer(webpoint);
	 for(int i=0; i<vecServerList.size(); i++)
	 {
		 if(!vecServerList[i].empty())
		 {
			 CWebEngine* pWebengine = new CWebEngine(this,this->m_dbsession_history);
			 pWebengine->SetPointList(vecServerList[i]);
			 pWebengine->SetLogSession(m_dbsession_log);
			 pWebengine->SetIndex(i);
			 wstring strSer = vecServerList[i][0].GetParam(3) + vecServerList[i][0].GetParam(4) + vecServerList[i][0].GetParam(5);
			 if(!pWebengine->Init())
			 {
				 CString strFailedInfo;
				 strFailedInfo.Format(L"ERROR: Init Web Engine(%s) Failed.\r\n", strSer.c_str());
				 SendLogMsg(L"", strFailedInfo);
			 }
			 m_mapWebEngine[strSer] = pWebengine;
		 }
	 }
	 return true;
 }

 vector<vector<DataPointEntry>> CBEOPDataLinkManager::GetWebServer( vector<DataPointEntry> webpoint )
 {
	 vector<vector<DataPointEntry>> vecWebPList;
	 vecWebPList.clear();
	 for(int i=0; i<webpoint.size(); i++)
	 {
		 //
		 wstring strIP = webpoint[i].GetParam(3);
		 wstring strUser = webpoint[i].GetParam(5);
		 wstring strPsw = webpoint[i].GetParam(6);
		 if(strIP.empty() || strUser.empty() || strPsw.empty())
			 continue;

		 int nPos = IsExistInVec(strIP,strUser,strPsw,m_vecWebSer);
		 if(nPos==m_vecWebSer.size())	//不存在
		 {
			 WebServer pServer;
			 pServer.strIP = strIP;
			 pServer.strUser = strUser;
			 pServer.strPsw = strPsw;
			 m_vecWebSer.push_back(pServer);

			 vector<DataPointEntry> vecPList;
			 vecWebPList.push_back(vecPList);
		 }
		 vecWebPList[nPos].push_back(webpoint[i]);		
	 }
	 return vecWebPList;
 }

 CDiagnoseLink* CBEOPDataLinkManager::GetDiagnoseEngine()
 {
	 return m_pDiagnoseEngine;
 }

 void CBEOPDataLinkManager::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
 {
	 _tprintf(strLog.c_str());
 }



 bool CBEOPDataLinkManager::WriteWirelessPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(pentry.GetSValue());

		 return m_dbsession_realtime->UpdatePointData_Wireless(realtimeEntry);
	 }

	 return false;
 }

 bool CBEOPDataLinkManager::WriteOPCUAPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_OPCUA(realtimeEntry);
	 }

	 return false;
 }

 bool    CBEOPDataLinkManager::WriteBacnetPyPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_BacnetPy(realtimeEntry);
	 }

	 return false;
 }


 bool CBEOPDataLinkManager::WriteModbusEquipmentPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_ModbusEquipment(realtimeEntry);
	 }

	 return false;
 }



 bool CBEOPDataLinkManager::WriteKNXPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_KNX(realtimeEntry);
	 }

	 return false;
 }



 bool CBEOPDataLinkManager::WriteDLT645Point( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_DLT645(realtimeEntry);
	 }

	 return false;
 }


 bool CBEOPDataLinkManager::WritePersagyControllerPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_PersagyController(realtimeEntry);
	 }

	 return false;
 }



 bool CBEOPDataLinkManager::WriteThirdPartyPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_ThirdParty(realtimeEntry);
	 }

	 return false;
 }



 bool CBEOPDataLinkManager::WriteObixPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_Obix(realtimeEntry);
	 }

	 return false;
 }



 bool CBEOPDataLinkManager::WriteLogixPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_Logix(realtimeEntry);
	 }

	 return false;
 }




 bool CBEOPDataLinkManager::WriteABSLCPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_ABSLC(realtimeEntry);
	 }

	 return false;
 }



 bool CBEOPDataLinkManager::WriteMoxaTCPServerPoint( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
	 DataPointEntry pentry = m_pointmanager->GetEntry(pointname);

	 if(m_dbsession_realtime)
	 {
		 CRealTimeDataEntry realtimeEntry;
		 std::string strName;
		 Project::Tools::WideCharToUtf8(pentry.GetShortName(), strName);
		 realtimeEntry.SetName(strName);
		 realtimeEntry.SetValue(entry.GetValue());

		 return m_dbsession_realtime->UpdatePointData_MoxaTCPServer(realtimeEntry);
	 }

	 return false;
 }
