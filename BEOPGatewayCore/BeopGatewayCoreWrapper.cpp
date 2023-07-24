#include "StdAfx.h"
#include "BeopGatewayCoreWrapper.h"
#include "DumpFile.h"
#include "../COMdll/ModbusTcp/DogTcpCtrl.h"
#include "../BEOPDataLink/MemoryLink.h"
#include "../LAN_WANComm/Tools/ToolsFunction/ToolsFunction.h"

const CString g_NAME_Logic = _T("domlogic.exe");
const int	  g_Restart_Count = 3;

//#define CORE_VERSION _T("2.2.3_2018_09_28")
//#define CORE_VERSION _T("2.2.4_2018_10_22")
//#define CORE_VERSION _T("2.2.5_2018_10_23")
//#define CORE_VERSION _T("2.2.6_2018_10_31")
//#define CORE_VERSION _T("2.2.7_2018_11_02")
//#define CORE_VERSION _T("2.2.8_2018_11_04")
//#define CORE_VERSION _T("2.2.9_2018_11_27")
//#define CORE_VERSION _T("2.2.10_2018_11_29")
//#define CORE_VERSION _T("2.2.11_2018_12_06")
//#define CORE_VERSION _T("2.2.12_2019_01_02")
//#define CORE_VERSION _T("2.2.13_2019_01_15")
//#define CORE_VERSION _T("2.2.14_2019_01_19")
//#define CORE_VERSION _T("2.2.15_2019_01_31")
//#define CORE_VERSION _T("2.2.16_2019_02_08")
//#define CORE_VERSION _T("2.2.17_2019_02_18")
//#define CORE_VERSION _T("2.2.18_2019_02_22")
//#define CORE_VERSION _T("2.2.19")
//#define CORE_VERSION _T("2.2.20")
//#define CORE_VERSION _T("2.3.1")
//2.3.2 增加了对报警详情的支持，动了数据库结构
//#define CORE_VERSION _T("2.3.2")
//2.3.3 增加了点名大小写冲突检测
//#define CORE_VERSION _T("2.3.3")
// 增加了不需要再设置后台是否开启存储!
//#define CORE_VERSION _T("2.3.4")
//#define CORE_VERSION _T("2.3.5")
//#define CORE_VERSION _T("2.3.6")

//去除了自动load点表
//#define CORE_VERSION _T("2.3.7")


//增加了sys_logic数据库
//#define CORE_VERSION _T("2.3.8") 
//#define CORE_VERSION _T("2.3.9")

//增加支持了报警的点值字段
//#define CORE_VERSION _T("2.3.10")
//#define CORE_VERSION _T("2.3.11")
//#define CORE_VERSION _T("2.3.12")

//增加在线热添加点以及屏蔽点选项
//#define CORE_VERSION _T("2.3.13")


//增加opc坏点的记录
//#define CORE_VERSION _T("2.3.14")


//解决mysql错误不断记录到4G的问题
////#define CORE_VERSION _T("2.3.15")
//#define CORE_VERSION _T("2.3.17")
//
//
////创建mode相关表
//#define CORE_VERSION _T("2.3.18")
//
////weather表
//#define CORE_VERSION _T("2.3.19")
//#define CORE_VERSION _T("2.3.20")
//#define CORE_VERSION _T("2.3.21")
//
////支持bacnet py点
//#define CORE_VERSION _T("2.3.22")
//
////修复bacnet-py
//#define CORE_VERSION _T("2.3.23")
////修复bacnet-py
//#define CORE_VERSION _T("2.3.24")
////修复数据库没有用户密码时的启动问题
//#define CORE_VERSION _T("2.3.25")
////bacnet支持优先级
//#define CORE_VERSION _T("2.3.26")
////支持西门子点的被倍率
//#define CORE_VERSION _T("2.3.27")
////支持西门子点的两段定义，格式 6553:0|32767:100
//#define CORE_VERSION _T("2.3.28")
//
////增加报修表
//#define CORE_VERSION _T("2.3.29")
//
////增加报修经验的标题
//#define CORE_VERSION _T("2.3.30")
//
//
////将西门子引擎分离
//#define CORE_VERSION _T("2.4.1")
//#define CORE_VERSION _T("2.4.2")
//#define CORE_VERSION _T("2.4.3")
//#define CORE_VERSION _T("2.4.4")
//
////将实时扫描进程优化，减轻对mysql的压力
//#define CORE_VERSION _T("2.4.5")
//#define CORE_VERSION _T("2.4.6")
//
//#define CORE_VERSION _T("2.4.7")
//
////报警表分离按月存储
//#define CORE_VERSION _T("2.4.8")
//
//
////支持AB Logix类型点
//#define CORE_VERSION _T("2.4.9")
//
//
////redis升级
//#define CORE_VERSION _T("2.4.10")
//
//
////零点历史数据机制修复（容易漏数据）
//#define CORE_VERSION _T("2.4.11")
//
////继续修复零点历史数据机制修复（容易漏数据）
//#define CORE_VERSION _T("2.4.12")
//
//
////mode_detail增加场景到点执行一次还是维持生效
//#define CORE_VERSION _T("2.4.13")
////log文件合并
//#define CORE_VERSION _T("2.4.14")
//
////增加MoxaTCPServer点支持
//#define CORE_VERSION _T("2.4.15")
//
////增加注册表检测
//#define CORE_VERSION _T("2.4.16")
//
//
////增加fdd_work_order的机制
//#define CORE_VERSION _T("2.4.17")
//
////增加点位长度检查
//#define CORE_VERSION _T("2.4.18")
//
////增加modbus 12,13数据类型(12为无符号，13为无符号逆序)
//#define CORE_VERSION _T("2.4.19")
//
////支持CoreStation引擎
//#define CORE_VERSION _T("2.4.20")
////支持CoreStation引擎修复bug
//#define CORE_VERSION _T("2.4.21")
//
////引擎默认modbus轮询间隔改为5秒
//#define CORE_VERSION _T("2.4.22")
//
//
////警报bool等级
//#define CORE_VERSION _T("2.4.23")
//
////支持KNX
//#define CORE_VERSION _T("2.4.24")
////支持KNX bug 修复
//#define CORE_VERSION _T("2.4.25")
//支持各类零散的opc还有mysql log统一归到core_log开头的文件中
#define CORE_VERSION _T("2.4.26")


//fdd_workorder表增加ownUser列
#define CORE_VERSION _T("2.4.27")


//数据库支持资产管理信息创建
#define CORE_VERSION _T("2.4.28")


//支持redis缓存及刷新机制
#define CORE_VERSION _T("2.4.29")


//支持redis刷新时的批量
#define CORE_VERSION _T("2.4.30")



//支持redis刷新时的批量2
#define CORE_VERSION _T("2.4.31")


//數據庫用戶分離為domcore,dompysite,domlogic,dompysite01,dompysite02
#define CORE_VERSION _T("2.4.31")


//數據庫用戶拥有create,drop权限
#define CORE_VERSION _T("2.4.32")


//增加资产管理其余表
#define CORE_VERSION _T("2.4.33")


//增加domtask用户

#define CORE_VERSION _T("2.4.34")


//【重大缺陷版本:DLT645无法读取】增加DLT645点
#define CORE_VERSION _T("2.4.35")

//DLT645点读取改进
#define CORE_VERSION _T("2.4.36")


//删去drop table所有操作可能
#define CORE_VERSION _T("2.4.37")


//第一次创建数据库时使用root用户名
#define CORE_VERSION _T("2.4.38")

//非本地数据库兼容创建用户和权限
#define CORE_VERSION _T("2.4.39")


//支持AB SLC协议，用于MicroLogix 1400系列等
#define CORE_VERSION _T("2.4.40")


//部分天M5表创建时没有插入全部数据，改为同M1的combine机制
#define CORE_VERSION _T("2.4.41")



//增加創建用戶對localhost有效，發現部分項目用戶不能登陸localhost
#define CORE_VERSION _T("2.4.42")


//增加創建用戶domjobs
#define CORE_VERSION _T("2.4.43")


//modbus改进重连机制
#define CORE_VERSION _T("2.4.44")


//modbus点增加值映射
#define CORE_VERSION _T("2.4.45")


//modbus AO容易被误写
#define CORE_VERSION _T("2.4.46")


//
#define CORE_VERSION _T("2.4.47")


//rebuild database 改进，兼容当domdb.4db错误时也能rebuild
#define CORE_VERSION _T("2.4.48")



//支持deeplogic_redis更新
#define CORE_VERSION _T("2.4.49")


//支持modbus写值倍率
#define CORE_VERSION _T("2.4.50")

//modbus重连机制优化
#define CORE_VERSION _T("2.4.51")

//modbus重连机制优化
#define CORE_VERSION _T("2.4.52")
//modbus重连机制优化，当掉线时不会卡太久
#define CORE_VERSION _T("2.4.53")


//history表的pointname扩充到255
#define CORE_VERSION _T("2.4.54")
//modbus引擎断线卡顿问题彻底优化
#define CORE_VERSION _T("2.4.55")


//去掉创建历史表时的ON UPDATE更新时间机制
#define CORE_VERSION _T("2.4.56")
/************************************************************************/
/* 
 
V2.2.3_2018_09_28： 增加了OPC写入后立即读取更新的支持，目前还差modbus了，bacnet之前就支持。
                    将西门子S7UDP读取默认改为了4个小数点，之前一直是两位小数
V2.2.4:
   补齐历史数据零点的修改

V2.2.5:
   将mysql 的log统一放置到log目录中

   V2.2.6:
   历史数据表改为非主键，防止存储历史时有时会冲突键导致存不上，现在解决后可能会同一时刻存多个值

   V2.2.7:
   mysql的log表里增加了[SYSTEM]的字段，记录插入的1分钟历史数据条数，跟踪零点不存储的问题

   V2.2.8:
   修复了零点问题，因为dompysite也会创建表，导致部分天时domcore就只在零点保存了部分数据
    修复方法是检测两次保存时间，如果天不同的话，那么强制保存。

   V2.2.12:
	默认实时数据表内容支持到20000长度字符串

   V2.2.15:
    升级支持数据库换库时的表创建bug

   V2.2.15:
   modbus写值后增加立即读取

   V2.2.18:
   modbus读取不到重连时的LOG

   V2.2.19:
   增加对persagy控制器的支持

   V2.2.20:
   增加对persagy控制器的支持bug修復

   V2.3.1
   空字符串写值时的重大隐患bug

   2.3.12
   支持报警的更多自定义属性

   2.4.3
   增加modbus Equipment表的讀取顯示日志

*/
/************************************************************************/

CBeopGatewayCoreWrapper::CBeopGatewayCoreWrapper(void)
{

	m_pDataEngineCore = NULL;
	m_hEventActiveStandby = NULL;
	m_Backupthreadhandle = NULL;
	 m_pDataAccess_Arbiter = NULL;
	m_pRedundencyManager = NULL;
	m_pLogicManager = NULL;
	m_pFileTransferManager = NULL;
	m_pFileTransferManagerSend = NULL;
	m_bFirstInit = true;
	m_bFitstStart = true;
	m_bExitCore = false;
	GetLocalTime(&m_stNow);
	m_oleStartTime = COleDateTime::GetCurrentTime();
	m_oleRunTime = COleDateTime::GetCurrentTime();
	m_oleCheckEveryHourTime = COleDateTime::GetCurrentTime()-COleDateTimeSpan(100,0,0,0);
	g_tLogAll.clear();
	g_strLogAll.clear();

	m_nDelayWhenStart = 30;

	m_nDBFileType = 0;
}

CBeopGatewayCoreWrapper::~CBeopGatewayCoreWrapper(void)
{
	g_tLogAll.clear();
	g_strLogAll.clear();
}

bool CBeopGatewayCoreWrapper::UpdateInputOutput()
{
	if(m_pDataEngineCore->m_bUpdateInputTableFromSite) //与现场保持连接时的处理
	{
		if(!m_pDataEngineCore->UpdateInputTable()) //从现场读取信号，存入realtime_input表中
		{
			PrintLog(L"ERROR: UpdateInputTable Failed. Will delay 60 seconds...\r\n",true);
			Sleep(60*1000);
			return false;
		}
	}
	else  //不与现场通讯，仅模拟测试
	{//
		vector<CRealTimeDataEntry>	inputentrylist;
		m_pDataEngineCore->GetRealTimeDataAccess()->ReadRealtimeData_Input(inputentrylist);
		unsigned int i = 0;
		for(i=0;i<inputentrylist.size();i++)
		{
			inputentrylist[i].SetTimestamp(m_stNow);
		}

		m_pDataEngineCore->GetDataLink()->UpdateRedis(COleDateTime::GetCurrentTime(), inputentrylist);

		m_pDataEngineCore->GetDataLink()->SetRealTimeDataEntryList(inputentrylist);
	}

	//output
	COleDateTimeSpan oleSpanTime = COleDateTime::GetCurrentTime() - m_oleStartTime;
	if(!m_bFitstStart && m_nDelayWhenStart <=oleSpanTime.GetTotalSeconds())			//启动超过1分钟后再执行策略输出结果
	{
		wstring strValueChanged;
		bool bUpdate = m_pDataEngineCore->GetDataLink()->UpdateOutputParams(strValueChanged);
		if(!bUpdate)
		{
			PrintLog(L"ERROR: Update Output Values Failed.\r\n",false);
		}
		else if(strValueChanged.length()>0)
		{
			PrintLog(strValueChanged.c_str(),false);
		}
	}
	else												//未满1分钟清空策略输出结果
	{
		if(m_nDelayWhenStart >oleSpanTime.GetTotalSeconds())
		{
			CString strPrintTemp;
			strPrintTemp.Format(_T("INFO : Core will enable set value after %d seconds.\r\n"), int(m_nDelayWhenStart-oleSpanTime.GetTotalSeconds()));
			PrintLog(strPrintTemp.GetString(),false);
		}

		m_pDataEngineCore->GetDataLink()->CleatOutputData();
		m_bFitstStart = false;
	}
	return true;
}

bool CBeopGatewayCoreWrapper::Run()
{
	
	//初始化Dump文件
	DeclareDumpFile();

	//检查注册表
	char szByteReadValue[MAX_PATH] = {0};
	LPBYTE lpByteReadValue = NULL;
	lpByteReadValue = new BYTE[MAX_PATH];
	memset(lpByteReadValue, NULL, MAX_PATH);
	CToolsFunction toolfunc;
	toolfunc.RegReadValue(HKEY_CURRENT_USER,  "Software\\Microsoft\\Windows\\Windows Error Reporting" , "DontshowUI", lpByteReadValue);
	strcpy_s(szByteReadValue, MAX_PATH, reinterpret_cast<const char*>(lpByteReadValue));

	// There are update files which have not been update.
	if (strcmp(szByteReadValue, "1") != 0)
	{
		toolfunc.RegWriteValue(HKEY_CURRENT_USER,  "Software\\Microsoft\\Windows\\Windows Error Reporting" , "DontshowUI", "1");
	}
	else
	{
		
	}

	delete[] lpByteReadValue;
	lpByteReadValue = NULL;

	HANDLE m_hmutex_instancelock = CreateMutex(NULL, FALSE, L"domcore");
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		PrintLog(L"ERROR: Another domcore is running, Start failed.\r\n",false);
		Sleep(10000);
		exit(0);
		return false;
	}

	wchar_t wcChar[1024];
	COleDateTime tnow = COleDateTime::GetCurrentTime();
	wsprintf(wcChar, L"---- domcore(%s) starting ----\r\n", CORE_VERSION);
	PrintLog(wcChar);

	m_strDBFilePath = PreInitFileDirectory_4db();
	if(m_strDBFilePath.length()==0)
	{
		m_strDBFilePath = PreInitFileDirectory();
		m_nDBFileType = 1;
	}
	

	while(!Init(m_strDBFilePath))
	{
		if(m_bExitCore)
		{
			return false;
		}
		Release();
		PrintLog(L"ERROR: Init Loading domdb.4db Failed, Please check file exist or valid.\r\n",false);
		PrintLog(L"ERROR: Init Loading domdb.4db Failed, Try again after 30 seconds automatic...\r\n",false);
		Sleep(30000);
	}

	OutPutLogString(_T("domcore.exe start"));

	m_bFirstInit = false;

	//启动伴生程序
	//InitDog();

	//FeedDog();

	//clear reload flag
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"core_reload_point_table", L"0");


	wstring strTimeServer;
	COleDateTime oleNow;
	oleNow = COleDateTime::GetCurrentTime();
	Project::Tools::OleTime_To_String(oleNow,strTimeServer);
	if(m_pDataAccess_Arbiter)
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corestarttime", strTimeServer);

	//统计重启次数
	wstring wstrRestartCount = L"";
	int nRestartCount = 0;
	m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"corerestartcount", wstrRestartCount);
	if(wstrRestartCount.length()>0)
	{
		nRestartCount = _wtoi(wstrRestartCount.c_str());
	}
	nRestartCount+=1;
	if(nRestartCount>=1e9)
		nRestartCount = 0;
	CString strRestartCount;
	strRestartCount.Format(_T("%d"), nRestartCount);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corerestartcount", strRestartCount.GetString());

	wstring wstrClearLogicOutputHoursBefore;
	int nClearLogicOutputHoursBefore = 4;
	m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"logic_record_reserve_hours", wstrClearLogicOutputHoursBefore);
	if(wstrClearLogicOutputHoursBefore.length()>0)
	{
		nClearLogicOutputHoursBefore = _wtoi(wstrClearLogicOutputHoursBefore.c_str());
	}

	//record restart, info

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"domcore", L"version", CORE_VERSION);

	wstring wstrValue = L"0";
	int nCurRestartCount = 0;
	if(m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"domcore", L"restartCount", wstrValue))
	{
		nCurRestartCount = _wtoi(wstrValue.c_str());
	}
	nCurRestartCount++;
	CString strTempRestartCount;
	strTempRestartCount.Format(_T("%d"), nCurRestartCount);

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"domcore", L"restartCount", strTempRestartCount.GetString());


	 wstrValue = L"";
	 CString strRestartTimeList;
	m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"domcore", L"restartTimeList", wstrValue);
	
	strRestartTimeList.Format(_T("%s,%s"), wstrValue.c_str(), strTimeServer.c_str());
	if(strRestartTimeList.GetLength()>=2000)
	{
		strRestartTimeList = strRestartTimeList.Right(2000);
	}

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"domcore", L"restartTimeList", strRestartTimeList.GetString());


	int nTimeCount = 15;  //30s更新一次ini文件
	while(!m_bExitCore)
	{
		Sleep(500);
		
		GetLocalTime(&m_stNow);
		//获取输入，存入input，执行输出
		if(!m_bExitCore)
			UpdateInputOutput();

		oleNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan oleSpan = oleNow - m_oleRunTime;

		//以下是30秒做一次的事情
		if(oleSpan.GetTotalSeconds() >= 30)
		{
			//更新服务器时间
			wstring strTimeServer;
			Project::Tools::OleTime_To_String(oleNow,strTimeServer);
			if(m_pDataAccess_Arbiter)
				m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"servertime", strTimeServer);

			//save log
			wstring strAlwaysLog = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"alwayslog");
			if(strAlwaysLog==L"1")
			{
				SaveAndClearLog();
			}
			
			//check the point table need reload
			/*
			wstring wstrNeedReploadPointTable = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"core_reload_point_table");
			if(wstrNeedReploadPointTable==L"1")
			{
				PrintLog(L"Point Table Reload Command Recved, Reload!\r\n", true);

				m_pDataEngineCore->InitPoints();
				m_pDataEngineCore->GetDataLink()->InitInputTable();
				m_pDataAccess_Arbiter->UpdateSavePoints(m_pDataEngineCore->GetDataPointManager()->GetPointList());

				m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"core_reload_point_table", L"0");
			}
			*/
			
			wstring wstrVpointAdd = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"point_vpoint_add");
			if(wstrVpointAdd!=L"0" && wstrVpointAdd.length()>0)
			{
				PrintLog(L"Point VPoint Add Command Recved, Start Add\r\n", true);

				vector<wstring> wstrPointNameList;
				Project::Tools::SplitStringByChar(wstrVpointAdd.c_str(), ',', wstrPointNameList);


				vector<CRealTimeDataEntry> realdataList;
				for(int i=0;i<wstrPointNameList.size();i++)
				{
					m_pDataEngineCore->GetDataPointManager()->AddVPoint(wstrPointNameList[i]);

					m_pDataEngineCore->GetDataLink()->GetMemoryEngine()->AddVPoint(wstrPointNameList[i]);

					CString strTemp;
					strTemp.Format(_T("Adding VPoint: %s\r\n"), wstrPointNameList[i].c_str());

					PrintLog(strTemp.GetString(), true);

					CRealTimeDataEntry tempEntry;
					string str_pointname_utf8;
					Project::Tools::WideCharToUtf8(wstrPointNameList[i], str_pointname_utf8);
					tempEntry.SetName(str_pointname_utf8);
					tempEntry.SetTimestamp(m_stNow);
					tempEntry.SetValue(_T(""));
					realdataList.push_back(tempEntry);
				}


				if(!m_pDataEngineCore->m_bUpdateInputTableFromSite) //仿真模式下直接插入实时表
				{
					m_pDataEngineCore->GetRealTimeDataAccess()->InsertRealTimeDatas_Input_NoClear(realdataList);
				}

				m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"point_vpoint_add", L"");
			}


			//删除点先不管
			/*
			wstring wstrVpointRemove = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"point_vpoint_remove");
			if(wstrVpointAdd!=L"0" && wstrVpointRemove.length()>0)
			{
				PrintLog(L"Point VPoint Remove Command Recved, Start Add\r\n", true);

				vector<wstring> wstrPointNameList;
				Project::Tools::SplitStringByChar(wstrVpointRemove.c_str(), ',', wstrPointNameList);
				for(int i=0;i<wstrPointNameList.size();i++)
				{
					m_pDataEngineCore->GetDataPointManager()->RemoveVPoint(wstrPointNameList[i]);
					m_pDataEngineCore->GetDataLink()->GetMemoryEngine()->RemoveVPoint(wstrPointNameList[i]);

					CString strTemp;
					strTemp.Format(_T("Removing VPoint: %s\r\n"), wstrPointNameList[i].c_str());

					PrintLog(strTemp.GetString(), true);
				}

				m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"point_vpoint_remove", L"");
			}
			*/

			if(!m_bExitCore)
				UpdateSaveLog();

			//update restart
			if(!m_bExitCore)
				UpdateResetCommand();

			//update file
			if(!m_bExitCore)
				UpdateS3DBDownloaded();

			//update file
			if(!m_bExitCore)
				UpdateS3DBUploaded();

			//check mode change
			if(!m_bExitCore)
				UpdateSiteModeChanged();

			m_oleRunTime = oleNow;
		}

		//每小时的任务（一开始也作一次）
		COleDateTimeSpan oleSpan2  = oleNow - m_oleCheckEveryHourTime;
		if(oleSpan2.GetTotalSeconds() >= 60*60)
		{
			//check warningrecord, 上上个月及以前的报警进行归档
			PrintLog(_T("Check And Save WarningRecord Start\r\n"), false);
			if(!m_pDataAccess_Arbiter->CheckAndSaveWarningRecord())
			{
				PrintLog(L"CheckAndSaveWarningRecord Found Problem, Repaired!!\r\n", true);
			}
			PrintLog(_T("Check And Save WarningRecord Finish\r\n"), false);

			//check history data table is healthy?
			if(!m_pDataAccess_Arbiter->CheckAndRepairHistoryDataTableHealthy())
			{
				PrintLog(L"CheckAndRepairHistoryDataTableHealthy Found Problem, Repaired!!\r\n", true);
			}

			COleDateTime tClear = oleNow - COleDateTimeSpan(0,nClearLogicOutputHoursBefore,0,0);
			wstring wstrClearBefore;
			Project::Tools::OleTime_To_String(tClear, wstrClearBefore);
			m_pDataAccess_Arbiter->ClearLogicOutputPointRecordBeforeTime(wstrClearBefore);

			m_oleCheckEveryHourTime = oleNow;
		}


	}

	return true;

}

bool CBeopGatewayCoreWrapper::UpdateResetCommand()
{
	if(m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"reset")==L"1")
	{
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"resetlogic",L"1");
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"reset", L"0");
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corestatus", L"busy");
		
		PrintLog(L"INFO : Reset Signal Recved, Restarting...\r\n");
		Reset();
		PrintLog(L"INFO : Core restarted, initializing...\r\n\r\n");
		
		OutPutLogString(_T("domcore.exe manual restart"));
		Init(m_strDBFilePath);
	}

	return true;
}

bool CBeopGatewayCoreWrapper::UpdateSiteModeChanged()
{
	wstring strMode;
	bool bRead = m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"sitemode", strMode);
	if(bRead &&m_pDataEngineCore->m_bUpdateInputTableFromSite && strMode==L"0")
	{//切入仿真模式，需要重启
		PrintLog(L"INFO : Mode Changed to Simulation, core is restarting...\r\n");
		Reset();
		PrintLog(L"INFO : Core restarted, initializing...\r\n\r\n");
		Init(m_strDBFilePath);

	}
	else if(bRead && !m_pDataEngineCore->m_bUpdateInputTableFromSite && strMode==L"1")
	{
		PrintLog(L"INFO : Mode changed to site, core is restarting...\r\n");
		Reset();
		PrintLog(L"INFO : Core restarted, initializing...\r\n\r\n");
		Init(m_strDBFilePath);

	}

	return true;
}

bool CBeopGatewayCoreWrapper::UpdateS3DBDownloaded()
{
	if(m_pFileTransferManager->m_bNewFileRecved)
	{
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"filerecved", L"finish");
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corestatus", L"busy");
		PrintLog(L"INFO : DB File Changed, core is restarting...\r\n");
		//backup current db file
		wstring strDir;
		Project::Tools::GetSysPath(strDir);
		CString strNewFileName, strBackupFilePath;

		int nFindIndex = m_pDataEngineCore->m_strDBFileName.find_last_of('\\');
		wstring strFileName = m_pDataEngineCore->m_strDBFileName.substr(nFindIndex+1);

		strBackupFilePath.Format(L"%s\\DBFileVersion\\%s",strDir.c_str(), strFileName.c_str());

		if(CopyFile(m_pDataEngineCore->m_strDBFileName.c_str(),strBackupFilePath.GetString(),FALSE))
		{
			if(!UtilFile::DeleteFile(m_pDataEngineCore->m_strDBFileName.c_str()))
			{
				PrintLog(L"ERROR: Delete Current DB Files Failed!\r\n");
			}
		}

		Reset();
		
		PrintLog(L"INFO : Core restarted, initializing...\r\n");
		m_strDBFilePath = PreInitFileDirectory_4db();
		if(m_strDBFilePath.length()==0)
		{
			m_strDBFilePath = PreInitFileDirectory();
			m_nDBFileType = 1;
		}
		Init(m_strDBFilePath);
		m_pFileTransferManager->m_bNewFileRecved = false;

	}

	return true;
}

bool CBeopGatewayCoreWrapper::Reset()
{
	PrintLog(L"INFO : Start Unloading Logic Threads...\r\n",false);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"resetlogic", L"0");

	PrintLog(L"INFO : Exit DataEngineCore...\r\n",false);
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->SetDBAcessState(false);
		m_pDataEngineCore->TerminateScanWarningThread();
		m_pDataEngineCore->TerminateReconnectThread();

		m_pDataEngineCore->Exit();
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		PrintLog(L"Successfully.\r\n",false);
	}
	else
		PrintLog(L"ERROR.\r\n",false);
	

	if(m_pFileTransferManager)
	{
		m_pFileTransferManager->m_pDataAccess = NULL;
		delete(m_pFileTransferManager);
		m_pFileTransferManager = NULL;
	}
	
	if(m_pFileTransferManagerSend)
	{
		m_pFileTransferManagerSend->m_pDataAccess = NULL;
		delete(m_pFileTransferManagerSend);
		m_pFileTransferManagerSend = NULL;

	}

	PrintLog(L"INFO : Disconnect Database for Logic...\r\n",false);
	if(m_pDataAccess_Arbiter)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
		PrintLog(L"Successfully",false);
	}
	else
		PrintLog(L"ERROR.\r\n",false);

	if(m_pRedundencyManager)
	{
		delete(m_pRedundencyManager);
		m_pRedundencyManager = NULL;

	}

	return true;

}


wstring CBeopGatewayCoreWrapper::PreInitFileDirectory_4db()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	
	wstring strDBFileName;

	CFileFind s3dbfilefinder;
	wstring strSearchPath = cstrFile + L"\\domdb.4db";
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

	//log目录
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


wstring CBeopGatewayCoreWrapper::PreInitFileDirectory()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);

	wstring strDBFileName;

	CFileFind s3dbfilefinder;
	wstring strSearchPath = cstrFile + L"\\domdb.s3db";
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

	//log目录
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
	exepath = cstrFile + L"\\domcore.ini";
	wchar_t wcChar[1024];
	wchar_t charDBBuild[1024];

	GetPrivateProfileString(L"core", L"dbbuild", L"", charDBBuild, 1024, exepath.c_str());
	wstring wstDBBuild = charDBBuild;
	WritePrivateProfileString(L"core", L"dbbuild", wstDBBuild.c_str(), exepath.c_str());
	
	//读数据库
	CString strLog;
	strLog.Format(_T("INFO : Connect to MySQL Database: %s\r\n"),wstrServerIP.c_str());
	PrintLog(strLog.GetString(),false);
	
	

	bool bMySQLbeopDBExist = m_pDataEngineCore->CheckDBExist(wstrDBName);
	if(bMySQLbeopDBExist)
		strLog.Format(_T("INFO : Check MySQL Database: %s...Success\r\n"), wstrDBName.c_str());
	else
		strLog.Format(_T("INFO : Check MySQL Database: %s...Failed\r\n"), wstrDBName.c_str());
	PrintLog(strLog.GetString(),false);


	double fMySQLbeopCurVersion = m_pDataEngineCore->GetMySQLDBVersion(wstrDBName);


	
	if(wstDBBuild!=L"1")
	{
		if(!bMySQLbeopDBExist)
		{

			PrintLog(L"INFO : Rebuild Database when check not found MySQL DB in the first starting...\r\n",true);
			m_pDataEngineCore->ReBuildDatabase(wstrDBName);

			bMySQLbeopDBExist = m_pDataEngineCore->CheckDBExist(wstrDBName);
			if(bMySQLbeopDBExist)
			{
				PrintLog(L"INFO : Rebuild MySQL Database Successfully and Check OK...\r\n",true);
			}
			else
			{
				CString cmdParamstr;  
				cmdParamstr.Format(_T("mysqladmin -u root password RNB.beop-2013"));
				// 调用命令行解压原apk包
				CString cmd = cmdParamstr;//"java -jar " + currentPath + "\\lib\\apktool.jar d -f " + m_csTargetApk + " " + m_csTargetDir;//这里加入要执行的DOS指令
				STARTUPINFO si;
				ZeroMemory(&si,sizeof(si));
				PROCESS_INFORMATION pi;
				ZeroMemory(&pi,sizeof(pi));
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = true;
				BOOL bRet = ::CreateProcess(NULL,
					(LPWSTR)(LPCTSTR)cmd,
					NULL,NULL,
					false,
					CREATE_NEW_CONSOLE,
					NULL,NULL,
					&si,&pi);

				Sleep(5000);

				PrintLog(L"INFO : Again Rebuild Database when check not found db in the first starting...\r\n",true);
				m_pDataEngineCore->ReBuildDatabase(wstrDBName);

				bMySQLbeopDBExist = m_pDataEngineCore->CheckDBExist(wstrDBName);
				if(bMySQLbeopDBExist)
				{
					PrintLog(L"INFO : Again Rebuild Database Successfully and Check OK...\r\n",true);
				}
				else
				{
					strLog.Format(_T("ERROR: Again DataBase:%s Rebuild Failed! Program will exit!\r\n"),wstrDBName.c_str());
					PrintLog(strLog.GetString());
					return false;
				}
			}
		}
		else
		{
			PrintLog(L"INFO : Database Check passed.\r\n");
		}
	}
	else if(!bMySQLbeopDBExist)
	{
		strLog.Format(_T("ERROR: DataBase:%s Connect Failed or UnExist.\r\n"),wstrDBName.c_str());
		PrintLog(strLog.GetString());
		return false;
	}

	//数据库自动升级 2.2->2.3
	if(Project::Tools::IsDoubleEqual(fMySQLbeopCurVersion, 2.2))
	{
		PrintLog(L"INFO : Update Database From 2.2 To 2.3.\r\n");
		m_pDataEngineCore->DatabaseUpdateFrom22To23(wstrDBName);
	}

	//数据库自动升级 2.3->2.4
	fMySQLbeopCurVersion = m_pDataEngineCore->GetMySQLDBVersion(wstrDBName);
	if(Project::Tools::IsDoubleEqual(fMySQLbeopCurVersion, 2.3))
	{
		PrintLog(L"INFO : Update Database From 2.3 To 2.4.\r\n");
		m_pDataEngineCore->DatabaseUpdateFrom23To24(wstrDBName);
	}

	//check new db
	m_pDataEngineCore->DatabaseCreateNewInputOutputTable(wstrDBName);




	//连接成功一次就可以设置dbbuild
	if(bMySQLbeopDBExist)
		WritePrivateProfileString(L"core", L"dbbuild", L"1", exepath.c_str());

	return true;
}

bool CBeopGatewayCoreWrapper::Init(wstring strDBFilePath)
{
	CString strTemp;

	if(m_pFileTransferManager && m_pFileTransferManager->m_bIsTransmitting)
		return false;

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

	CString strLog;
	strLog.Format(_T("INFO : Reading DBFile:%s\r\n"),m_pDataEngineCore->m_strDBFileName.c_str());
	PrintLog(strLog.GetString(),false);

	//读取服务器数据库
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\domcore.ini";
	m_strIniFilePath = exepath;
	m_strLogFilePath  = cstrFile + L"\\domwatch.log";

	//init dataaccess
	gDataBaseParam realDBParam;
	bool bReadSuccess = m_pDataEngineCore->ReadRealDBparam(realDBParam);

	if(!bReadSuccess)
	{
		PrintLog(L"ERROR: Read domdb.4db Config Failed, Please Check domdb.4db file!\r\n");
	}

	if(realDBParam.strDBIP.length()<=0 ||
		realDBParam.strUserName.length()<=0)
	{
		PrintLog(L"ERROR: Read domdb.4db Info(IP,Name) Failed, Please Check the domdb.4db File.\r\n");	//edit 2016-03-11 S3db数据表project_config信息不见了？
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}

	//更新ini文件 服务器时间
	GetLocalTime(&m_stNow);
	wstring strTimeServer;
	Project::Tools::SysTime_To_String(m_stNow, strTimeServer);

	//wchar_t charServerIP[1024];
	//GetPrivateProfileString(L"core", L"server", L"", charServerIP, 1024, exepath.c_str());
	//wstring wstrServerIP = charServerIP;
	//if(wstrServerIP.length()<=7)
	//{
	//	wstrServerIP = L"localhost";
	//}
	//WritePrivateProfileString(L"core", L"server", wstrServerIP.c_str(), exepath.c_str());

	//realDBParam.strDBIP = Project::Tools::WideCharToAnsi(wstrServerIP.c_str());

	//读取选项
	wchar_t charDelayWhenStart[1024];
	GetPrivateProfileString(L"beopgateway", L"delaywhenstart", L"", charDelayWhenStart, 1024, exepath.c_str());
	wstring wstrDelayWhenStart = charDelayWhenStart;
	if(wstrDelayWhenStart.length()<=0)
	{
		wstrDelayWhenStart = L"15";
	}

	WritePrivateProfileString(L"beopgateway", L"delaywhenstart", wstrDelayWhenStart.c_str(), exepath.c_str());
	 m_nDelayWhenStart = _wtoi(wstrDelayWhenStart.c_str());


	 wchar_t charRootPassword[1024];
	 GetPrivateProfileString(L"core", L"dbpassword", L"", charRootPassword, 1024, exepath.c_str());
	 wstring wstrDBPassword = charRootPassword;
	 if(wstrDBPassword == L"")
		 wstrDBPassword = L"RNB.beop-2013";

	 realDBParam.strDBPsw = Project::Tools::WideCharToAnsi(wstrDBPassword.c_str());


	 wstring wstrDBName;
	 Project::Tools::UTF8ToWideChar(realDBParam.strRealTimeDBName, wstrDBName);
	 wstring wstrServerIP ;
	 Project::Tools::UTF8ToWideChar(realDBParam.strDBIP, wstrServerIP);
	 
	 if(m_bFirstInit && !InitCheckMySQLDB(wstrServerIP, wstrDBName ))
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

	m_pDataAccess_Arbiter->CreateHistoryDataStatusTableIfNotExist();

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"servertime",strTimeServer);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"starttime",strTimeServer);		//Core 启动时间
	Project::Tools::SysTime_To_OleTime(m_stNow,m_oleStartTime);

	//检测文件分表存储配置项
	wstring wstrFilePerTable = L"";
	if(m_pDataAccess_Arbiter->ReadMysqlVariable(L"innodb_file_per_table",wstrFilePerTable))
	{
		if(wstrFilePerTable == L"OFF")
		{
			PrintLog(L"ERROR : Check MySQL Setting(innodb_file_per_table(OFF))\r\n");
		}
	}

	//初始化COM 启用进程安全 因此不能线程读取OPC点
	wstring wstrEnableSecurity = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"enableSecurity",L"0");
	if(wstrEnableSecurity == L"1")
	{
		HRESULT hrinitcode = CoInitialize(NULL);
		if(hrinitcode != RPC_E_CHANGED_MODE)
		{
			hrinitcode = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		}		
		hrinitcode = CoInitializeSecurity(NULL, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_NONE,
				RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL);

		if(FAILED(hrinitcode))
		{
			PrintLog(L"ERROR: CoInitializeSecurity...\r\n");
		}
	}

	//写入Core的版本号
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"version",CORE_VERSION);

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runs7",L"0");//初始置为0

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corestatus", L"busy");

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"redundecymode",L"0"); 

	//将运行的s3db文件路径写入mysql的unit01的corefilepath
	bool bReplace = false;
	wstring strSavePath = Replace(strDBFilePath,L"\\",L"\\\\",bReplace);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corefilepath",strSavePath);
	
	//伴生程序
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runlogic",L"0");				//在冗余待机前先置0
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"rundtuengine",L"0");			//在冗余待机前先置0
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runwatch",L"0");				//在冗余待机前先置0
	WritePrivateProfileString(L"beopmaster", L"EnableCheckWatch",L"0",exepath.c_str());
	WritePrivateProfileString(L"beopmaster", L"EnableCheckLogic",L"0",exepath.c_str());
	WritePrivateProfileString(L"beopmaster", L"EnableCheckDTUEngine",L"0",exepath.c_str());

	//检测冗余
	wstring strIP = Project::Tools::AnsiToWideChar(m_pDataEngineCore->m_dbset.strDBIP.data());
	wchar_t charServerIP[1024];
	wsprintf(charServerIP, L"%s", wstrServerIP.c_str());

	GetPrivateProfileString(L"beopmaster", L"RedundencyIP", L"", charServerIP, 1024, exepath.c_str());
	wstring strIP2 = charServerIP;
	WritePrivateProfileString(L"beopgateway", L"RedundencyIP", strIP2.c_str(), exepath.c_str()); //第一次启动默认是仿真模式，1才是现场


	strLog.Format(_T("INFO : Check Redundency(IP: %s)...\r\n"), strIP2.c_str());
	PrintLog(strLog.GetString(),false);

	m_pDataEngineCore->m_dbset.strDBIP2 = Project::Tools::WideCharToAnsi(strIP2.c_str()) ;
	//m_strIP2不为空，传递冗余数据库连接

	if(strIP2.length()>=7)
	{
		PrintLog(L"INFO : Init Redundency Thread...\r\n",false);
		if(m_pRedundencyManager == NULL)
			m_pRedundencyManager = new CRedundencyManager;
		m_pRedundencyManager->Init(m_pDataEngineCore->m_nMainDropInterval, strIP.c_str() , strIP2.c_str());
	}

	//m_strIP2不为空，启动备份数据库线程，若能网络正常，传输文件
	wstring wstrCurRecordActiveIp = L"";
	m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"activeip", wstrCurRecordActiveIp);
	std::vector<string> strIPList;
	GetHostIPs(strIPList);
	if(strIPList.size()>0)
	{
		bool bExist = false;
		wstring wstrNewActive = L"";
		for(int j=0;j<strIPList.size();j++)
		{
			string strActiveIP = strIPList[j];
			wstrNewActive = Project::Tools::AnsiToWideChar(strActiveIP.c_str());
			if(wstrNewActive==wstrCurRecordActiveIp)
			{
				bExist = true;
				break;
			}		
		}
		if(!bExist)
		{
			m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"activeip",wstrNewActive);
		}
	}

	bool bSetSuccess = m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"reset", L"0");
	m_pDataEngineCore->m_bUpdateInputTableFromSite = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"sitemode",L"1"))==L"1"; //第一次启动默认是仿真模式，1才是现场
	if(m_pDataEngineCore->m_bUpdateInputTableFromSite)
		PrintLog(L"INFO : Init Engine(Site Mode)...\r\n",false);
	else
		PrintLog(L"INFO : Init Engine(Simulation Mode)...\r\n",false);

	///DTU发送///////////////////////////////////////////////////////////////////////
	wstring wstrDTUType = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUType", L"0");
	wstring wstrRunDTUEngine = L"0";
	if(_wtoi(wstrDTUType.c_str()) >= 4)			//需要启动DTUEngine
	{
		wstrRunDTUEngine = L"1";
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"rundtuengine",L"1");
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runwatch",L"0");
		WritePrivateProfileString(L"beopmaster", L"EnableCheckWatch",L"0",exepath.c_str());
		WritePrivateProfileString(L"beopmaster", L"EnableCheckDTUEngine",L"1",exepath.c_str());
	}
	else
	{
		wstrRunDTUEngine = L"0";
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"dtusendtime",strTimeServer);
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"rundtuengine",L"0");
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runwatch",L"1");
		WritePrivateProfileString(L"beopmaster", L"EnableCheckWatch",L"1",exepath.c_str());
		WritePrivateProfileString(L"beopmaster", L"EnableCheckDTUEngine",L"0",exepath.c_str());
	}

	///初始化DTU参数///////////////////////////////////////////////////////////////////////
	wstring wstrDTUport = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUComPort");		//读DTU Com口   默认为串口1
	if(wstrDTUport == L"" || wstrDTUport == L"0")
		wstrDTUport = L"1";

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"DTUComPort", wstrDTUport);
	wstring wstrDTUSendMinType = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUMinType");		//读DTU最小发送类型间隔 2:1分钟点  3:5分钟 默认为3
	if(wstrDTUSendMinType == L"" || wstrDTUSendMinType == L"0")
		wstrDTUSendMinType = L"3";

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"DTUMinType", wstrDTUSendMinType);

	bool bWithDTU = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUEnabled"))==L"1";
	bool bDTUChecked = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUChecked"))==L"1";	
	bool bDTURecCmd = bWithDTU;
	bool bDisableSendAll = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUDisableSendAll"))==L"1";		//是否禁用零点发送
	if(bWithDTU)
	{
		if(bDTURecCmd)
		{
			if(bDTUChecked)
			{
				strLog.Format(_T("INFO : DTU(With Receive And Check Fun) Setting Info: ComPort %s,SendMinType %s ...\r\n"), wstrDTUport.c_str(),wstrDTUSendMinType.c_str());
				PrintLog(strLog.GetString(),false);
			}
			else
			{
				strLog.Format(_T("INFO : DTU(With Receive Fun) Setting Info: ComPort %s,SendMinType %s ...\r\n"), wstrDTUport.c_str(),wstrDTUSendMinType.c_str());
				PrintLog(strLog.GetString(),false);
			}
		}
		else
		{
			strLog.Format(_T("INFO : DTU(Without Receive Fun) Setting Info: ComPort %s,SendMinType %s ...\r\n"), wstrDTUport.c_str(),wstrDTUSendMinType.c_str());
			PrintLog(strLog.GetString(),false);
		}
	}
	
	//TCP网络发送模式
	bool bWithTcpSender = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"TcpSenderEnabled"))==L"1";

	wstring wstrTcpSenderport = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderPort",L"9500");		//读TCP端口   默认为9500
	if(wstrTcpSenderport == L"" || wstrTcpSenderport == L"0")
		wstrTcpSenderport = L"9500";

	wstring wstrTcpSenderIP = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderIP",L"");		//读TCP IP   默认为114.215.172.232
	if(wstrTcpSenderIP == L"" || wstrTcpSenderIP == L"0")
		wstrTcpSenderIP = L"";

	wstring wstrTcpSenderName = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderName",L"");		//读TCP Name  用于标识项目
	if(bWithTcpSender)
	{
		if(wstrTcpSenderName == L"" || wstrTcpSenderName == L"0")
		{
			PrintLog(L"ERROR: TCPSender ERROR:TCPSender Name not valid, will not upload data...\r\n");
			bWithTcpSender = false;
		}
		else
		{
			strLog.Format(_T("INFO : TCPSender Setting Info: Name %s,IP %s,Port %s ...\r\n"), wstrTcpSenderName.c_str(),wstrTcpSenderIP.c_str(),wstrTcpSenderport.c_str());
			PrintLog(strLog.GetString(),false);
		}
	}

	//是否启动DTU
	wstring strRunDtu = L"0";
	if(bWithDTU || bWithTcpSender)
		strRunDtu = L"1";
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"rundtu", strRunDtu);
	
	//modbus配置参数
	bool bModbusreadonebyone = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"modbusreadonebyone",L"0"))==L"1";
	wstring wstrMobusInterval = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"ModbusReadIntervalMS",L"500");		//modbus轮询间隔  默认100ms
	if(wstrMobusInterval == L"" || wstrMobusInterval == L"0")
	{
		wstrMobusInterval = L"500";
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"ModbusReadIntervalMS",wstrMobusInterval);
	}

	//sendall
	bool bSendAllData = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"sendall",L"1"))==L"1";
	
	m_pDataEngineCore->Init(bWithDTU,bDTUChecked,bDTURecCmd,_wtoi(wstrDTUport.c_str()),_wtoi(wstrDTUSendMinType.c_str()),bModbusreadonebyone,_wtoi(wstrMobusInterval.c_str()),bDisableSendAll,bWithTcpSender,wstrTcpSenderName,wstrTcpSenderIP,_wtoi(wstrTcpSenderport.c_str()),bSendAllData,_wtoi(wstrRunDTUEngine.c_str()));
	if(!m_pDataEngineCore->InitDBConnection())
	{
		PrintLog(L"ERROR: InitDBConnection Failed.\r\n");
		return false;
	}

	wstring wstrIgnoreListDef = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"point_ignore_list");
	vector<wstring> wstrIgnoreList;
	if(wstrIgnoreListDef!=_T("0"))
	{
		Project::Tools::SplitStringByChar(wstrIgnoreListDef.c_str(), ',', wstrIgnoreList);

	}

	strTemp.Format(_T("INFO : Reading Ignore Points Count:%d...\r\n"), wstrIgnoreList.size());
	PrintLog(strTemp.GetString(), false);

	PrintLog(L"INFO : Reading Points in domdb.4db file...\r\n",false);

	if(!m_pDataEngineCore->InitPoints(wstrIgnoreList))
	{
		PrintLog(L"ERROR: Reading Points Failed...\r\n");
		return false;
	}
	CDataPointManager* pPointManager = m_pDataEngineCore->GetDataPointManager();
	if(pPointManager->GetAllPointCount() > 0)
	{
		std::ostringstream sqlstream;
		sqlstream << "INFO : Points List:";
		if(pPointManager->GetOPCPointCount() > 0)
			sqlstream << " OPC:" << pPointManager->GetOPCPointCount() << ",";
		if(pPointManager->GetModbusPointCount() > 0)
			sqlstream << " Modbus:" << pPointManager->GetModbusPointCount() << ",";
		if(pPointManager->GetBacnetPointCount() > 0)
			sqlstream << " Bacnet:" << pPointManager->GetBacnetPointCount() << ",";
		if(pPointManager->GetMemoryPointCount() > 0)
			sqlstream << " Virtual:" << pPointManager->GetMemoryPointCount() << ",";
		if(pPointManager->GetWebPointCount() > 0)
			sqlstream << " Web:" << pPointManager->GetWebPointCount() << ",";
		if(pPointManager->GetMySQLPointCount() > 0)
			sqlstream << " MySQL:" << pPointManager->GetMySQLPointCount() << ",";
		if(pPointManager->GetSqlServerPointCount() > 0)
			sqlstream << " SqlServer:" << pPointManager->GetSqlServerPointCount() << ",";
		if(pPointManager->GetProtocal104PointCount() > 0)
			sqlstream << " Protocol104:" << pPointManager->GetProtocal104PointCount() << ",";
		if(pPointManager->GetSiemensPointCount() > 0)
			sqlstream << " SiemensTCP:" << pPointManager->GetSiemensPointCount() << ",";
		if(pPointManager->GetFCbusPointCount() > 0)
			sqlstream << " FCbus:" << pPointManager->GetFCbusPointCount() << ",";
		if(pPointManager->GetSqlitePointCount() > 0)
			sqlstream << " Sqlite:" << pPointManager->GetSqlitePointCount() << ",";
		if(pPointManager->GetCO3PPointCount() > 0)
			sqlstream << " CO3P:" << pPointManager->GetCO3PPointCount() << ",";

		string strPointLog = sqlstream.str();
		strPointLog.erase(--strPointLog.end());

		sqlstream.str("");
		sqlstream << strPointLog << ".\r\n";
		PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));
	}
	else
	{
		PrintLog(L"ERROR: domdb.4db Points List Empty.\r\n");
		return false;
	}

	//插入点表总数
	CString strPointCount;
	strPointCount.Format(_T("%d"),pPointManager->GetAllPointCount());
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"pointcount",strPointCount.GetString());		//Core点表总数

	//save points into unit04
	if(m_pDataAccess_Arbiter->UpdateSavePoints(pPointManager->GetPointList()))
		PrintLog(L"INFO : Updating Online Points Success.\r\n",false);
	else
		PrintLog(L"ERROR: Updating Online Points Failed.\r\n");

	//清除
	//2019-08-24 golding: 这里为什么要清除？
	pPointManager->ClearAllPoint();

	PrintLog(L"INFO : Starting Engine...\r\n");
	if(!m_pDataEngineCore->InitEngine())
	{
		PrintLog(L"ERROR: InitEngine Failed!\r\n");
		return false;
	}

	//清除
	pPointManager->ClearAllPointListExceptOPC();

	//Sleep(800000);
	m_pDataEngineCore->EnableLog(FALSE);

	//init dataenginecore
	m_pDataEngineCore->SetDataAccess(m_pDataAccess_Arbiter);

	m_pDataEngineCore->SetDBAcessState(true);

	m_pDataEngineCore->ReadEquipmentEnableOption();

	m_hEventActiveStandby = CreateEvent(NULL, TRUE, FALSE, NULL);

	//S3DB 文件下载控制线程启动
	InitS3DBDownloadManager();

	// InitAfterlogin
	if(m_pDataEngineCore->m_bUpdateInputTableFromSite)
	{
		m_pDataEngineCore->GetDataLink()->InitOPC();
	}

	//清除
	pPointManager->ClearOPCList();


	COleDateTime tnow = COleDateTime::GetCurrentTime();	
	strLog.Format(_T("INFO : %s engine started successfully!\r\n"), tnow.Format(L"%Y-%m-%d %H:%M:%S "));
	PrintLog(strLog.GetString());
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corestatus", L"free");

	return true;
}

bool CBeopGatewayCoreWrapper::InitS3DBDownloadManager()
{
	if(m_pFileTransferManager==NULL)
	{
		PrintLog(L"INFO : Starting Download Monitoring...\r\n");
		m_pFileTransferManager = new CCoreFileTransfer(m_pDataAccess_Arbiter,  3456);
		m_pFileTransferManager->m_strPath = m_pDataEngineCore->m_strDBFileName.c_str();
		m_pFileTransferManager->StartAsReceiver();
	}
	else
		m_pFileTransferManager->m_pDataAccess = m_pDataAccess_Arbiter;

	if(m_pFileTransferManagerSend==NULL)
	{
		m_pFileTransferManagerSend = new CCoreFileTransfer(m_pDataAccess_Arbiter, 3457);
	}
	else
		m_pFileTransferManagerSend->m_pDataAccess = m_pDataAccess_Arbiter;

	return true;
}


bool CBeopGatewayCoreWrapper::SumRestartTime( int nMinute /*= 40*/ )
{
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	if(m_vecReStartTime.size() >= g_Restart_Count)
	{
		m_vecReStartTime.pop_back();
	}
	m_vecReStartTime.push_back(oleNow);
	int nSize = m_vecReStartTime.size();
	if(nSize == g_Restart_Count && g_Restart_Count>0)
	{
		COleDateTimeSpan oleSpan = oleNow - m_vecReStartTime[0];
		if(oleSpan.GetTotalMinutes() <= nMinute)
		{
			m_vecReStartTime.clear();
			Sleep(60*1000);
			return true;
		}
	}
	return false;
}

void CBeopGatewayCoreWrapper::SaveAndClearLog()
{
	if(g_strLogAll.size()<=0)
		return;

	//if(m_pDataAccess_Arbiter)
	//	m_pDataAccess_Arbiter->InsertLog(g_tLogAll, g_strLogAll);

	CString strLogAll;
	for(int i=0;i<g_tLogAll.size();i++)
	{
		if(i>=g_strLogAll.size())
			continue;

		string time_utf8 = Project::Tools::SystemTimeToString(g_tLogAll[i]);

		wstring wstrTime ;
		Project::Tools::UTF8ToWideChar(time_utf8, wstrTime);

		CString strOneItem;
		strOneItem.Format(_T("%s    %s"), wstrTime.c_str(), g_strLogAll[i].c_str());
		strLogAll+=strOneItem;
	}

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


			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			setlocale( LC_ALL, "chs" );
			//记录Log
			CStdioFile	ff;
			if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			{
				ff.Seek(0,CFile::end);
				ff.WriteString(strLogAll);
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
	

	g_strLogAll.clear();
	g_tLogAll.clear();

}

//bSaveLog: 强制立即写入log文件
void CBeopGatewayCoreWrapper::PrintLog(const wstring &strLog,bool bSaveLog)
{
	_tprintf(strLog.c_str());

	if(bSaveLog==false)
		return;

	SYSTEMTIME st;
	GetLocalTime(&st);
	g_tLogAll.push_back(st);
	g_strLogAll.push_back(strLog);
	if(bSaveLog)
	{
		SaveAndClearLog();
	}
	else if(g_strLogAll.size()>=100)
	{
		// 超过一定条数量，将记录到数据库，清空
		SaveAndClearLog();
	}
	
}

void CBeopGatewayCoreWrapper::GetHostIPs(vector<string> & IPlist)    
{    
    WORD wVersionRequested = MAKEWORD(2, 2);    
    WSADATA wsaData;    
    if (WSAStartup(wVersionRequested, &wsaData) != 0)    
    {  
        return;  
    }    
    char local[255] = {0};    
    gethostname(local, sizeof(local));    
    hostent* ph = gethostbyname(local);    
    if (ph == NULL)  
    {  
        return ;  
    }  
    in_addr addr;    
    HOSTENT *host=gethostbyname(local);    
    string localIP;    
    //当有多个ip时,j就是所有ip的个数   inet_ntoa(*(IN_ADDR*)host->h_addr_list[i] 这里的i就是对应的每个ip   
    for(int i=0;;i++)   
    {  
        memcpy(&addr, ph->h_addr_list[i], sizeof(in_addr)); //    
        localIP=inet_ntoa(addr);   
        IPlist.push_back(localIP);  
        if(host->h_addr_list[i]+host->h_length>=host->h_name)   
        {  
            break; //如果到了最后一条  
        }  
    }    
    WSACleanup();    
    
    return;  
}    

string CBeopGatewayCoreWrapper::GetHostIP()
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

	//记录Log
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
	if(m_pRedundencyManager)		//退出待机
		m_pRedundencyManager->ActiveSlaveEvent();
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->Exit();
		delete m_pDataEngineCore;
		m_pDataEngineCore = NULL;
	}
	g_tLogAll.clear();
	g_strLogAll.clear();
	return true;
}


bool CBeopGatewayCoreWrapper::UpdateS3DBUploaded()
{
	if(m_pFileTransferManagerSend)
	{
		wstring wstrUpLoadIP = L"";
		wstrUpLoadIP = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"uploadip",wstrUpLoadIP);
		if(wstrUpLoadIP.length() >0)
		{
			m_pFileTransferManagerSend->m_strPath = m_pDataEngineCore->m_strDBFileName.c_str();
			m_pFileTransferManagerSend->m_bFileSent	= false;
			m_pFileTransferManagerSend->StartAsSender(wstrUpLoadIP.c_str(),3458);
			m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"uploadip",L"");
		}
	}
	return true;
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

	if(m_pFileTransferManager)
	{
		m_pFileTransferManager->m_pDataAccess = NULL;
		delete(m_pFileTransferManager);
		m_pFileTransferManager = NULL;
	}

	if(m_pFileTransferManagerSend)
	{
		m_pFileTransferManagerSend->m_pDataAccess = NULL;
		delete(m_pFileTransferManagerSend);
		m_pFileTransferManagerSend = NULL;
	}

	if(m_pDataAccess_Arbiter)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
	}

	if(m_pRedundencyManager)
	{
		delete(m_pRedundencyManager);
		m_pRedundencyManager = NULL;
	}

	return true;
}



bool CBeopGatewayCoreWrapper::UpdateSaveLog()
{
	bool bSaveLog = true;
	if(m_pDataAccess_Arbiter)
	{
		wstring wstrSaveLogTable = L"1"; 
		wstrSaveLogTable = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"SaveLogTable",wstrSaveLogTable);
		m_pDataAccess_Arbiter->SetSaveLogFlag(_wtoi(wstrSaveLogTable.c_str()));
		return true;
	}
	return false;
}
