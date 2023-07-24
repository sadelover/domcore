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
//2.3.2 �����˶Ա��������֧�֣��������ݿ�ṹ
//#define CORE_VERSION _T("2.3.2")
//2.3.3 �����˵�����Сд��ͻ���
//#define CORE_VERSION _T("2.3.3")
// �����˲���Ҫ�����ú�̨�Ƿ����洢!
//#define CORE_VERSION _T("2.3.4")
//#define CORE_VERSION _T("2.3.5")
//#define CORE_VERSION _T("2.3.6")

//ȥ�����Զ�load���
//#define CORE_VERSION _T("2.3.7")


//������sys_logic���ݿ�
//#define CORE_VERSION _T("2.3.8") 
//#define CORE_VERSION _T("2.3.9")

//����֧���˱����ĵ�ֵ�ֶ�
//#define CORE_VERSION _T("2.3.10")
//#define CORE_VERSION _T("2.3.11")
//#define CORE_VERSION _T("2.3.12")

//������������ӵ��Լ����ε�ѡ��
//#define CORE_VERSION _T("2.3.13")


//����opc����ļ�¼
//#define CORE_VERSION _T("2.3.14")


//���mysql���󲻶ϼ�¼��4G������
////#define CORE_VERSION _T("2.3.15")
//#define CORE_VERSION _T("2.3.17")
//
//
////����mode��ر�
//#define CORE_VERSION _T("2.3.18")
//
////weather��
//#define CORE_VERSION _T("2.3.19")
//#define CORE_VERSION _T("2.3.20")
//#define CORE_VERSION _T("2.3.21")
//
////֧��bacnet py��
//#define CORE_VERSION _T("2.3.22")
//
////�޸�bacnet-py
//#define CORE_VERSION _T("2.3.23")
////�޸�bacnet-py
//#define CORE_VERSION _T("2.3.24")
////�޸����ݿ�û���û�����ʱ����������
//#define CORE_VERSION _T("2.3.25")
////bacnet֧�����ȼ�
//#define CORE_VERSION _T("2.3.26")
////֧�������ӵ�ı�����
//#define CORE_VERSION _T("2.3.27")
////֧�������ӵ�����ζ��壬��ʽ 6553:0|32767:100
//#define CORE_VERSION _T("2.3.28")
//
////���ӱ��ޱ�
//#define CORE_VERSION _T("2.3.29")
//
////���ӱ��޾���ı���
//#define CORE_VERSION _T("2.3.30")
//
//
////���������������
//#define CORE_VERSION _T("2.4.1")
//#define CORE_VERSION _T("2.4.2")
//#define CORE_VERSION _T("2.4.3")
//#define CORE_VERSION _T("2.4.4")
//
////��ʵʱɨ������Ż��������mysql��ѹ��
//#define CORE_VERSION _T("2.4.5")
//#define CORE_VERSION _T("2.4.6")
//
//#define CORE_VERSION _T("2.4.7")
//
////��������밴�´洢
//#define CORE_VERSION _T("2.4.8")
//
//
////֧��AB Logix���͵�
//#define CORE_VERSION _T("2.4.9")
//
//
////redis����
//#define CORE_VERSION _T("2.4.10")
//
//
////�����ʷ���ݻ����޸�������©���ݣ�
//#define CORE_VERSION _T("2.4.11")
//
////�����޸������ʷ���ݻ����޸�������©���ݣ�
//#define CORE_VERSION _T("2.4.12")
//
//
////mode_detail���ӳ�������ִ��һ�λ���ά����Ч
//#define CORE_VERSION _T("2.4.13")
////log�ļ��ϲ�
//#define CORE_VERSION _T("2.4.14")
//
////����MoxaTCPServer��֧��
//#define CORE_VERSION _T("2.4.15")
//
////����ע�����
//#define CORE_VERSION _T("2.4.16")
//
//
////����fdd_work_order�Ļ���
//#define CORE_VERSION _T("2.4.17")
//
////���ӵ�λ���ȼ��
//#define CORE_VERSION _T("2.4.18")
//
////����modbus 12,13��������(12Ϊ�޷��ţ�13Ϊ�޷�������)
//#define CORE_VERSION _T("2.4.19")
//
////֧��CoreStation����
//#define CORE_VERSION _T("2.4.20")
////֧��CoreStation�����޸�bug
//#define CORE_VERSION _T("2.4.21")
//
////����Ĭ��modbus��ѯ�����Ϊ5��
//#define CORE_VERSION _T("2.4.22")
//
//
////����bool�ȼ�
//#define CORE_VERSION _T("2.4.23")
//
////֧��KNX
//#define CORE_VERSION _T("2.4.24")
////֧��KNX bug �޸�
//#define CORE_VERSION _T("2.4.25")
//֧�ָ�����ɢ��opc����mysql logͳһ�鵽core_log��ͷ���ļ���
#define CORE_VERSION _T("2.4.26")


//fdd_workorder������ownUser��
#define CORE_VERSION _T("2.4.27")


//���ݿ�֧���ʲ�������Ϣ����
#define CORE_VERSION _T("2.4.28")


//֧��redis���漰ˢ�»���
#define CORE_VERSION _T("2.4.29")


//֧��redisˢ��ʱ������
#define CORE_VERSION _T("2.4.30")



//֧��redisˢ��ʱ������2
#define CORE_VERSION _T("2.4.31")


//�������Ñ����x��domcore,dompysite,domlogic,dompysite01,dompysite02
#define CORE_VERSION _T("2.4.31")


//�������Ñ�ӵ��create,dropȨ��
#define CORE_VERSION _T("2.4.32")


//�����ʲ����������
#define CORE_VERSION _T("2.4.33")


//����domtask�û�

#define CORE_VERSION _T("2.4.34")


//���ش�ȱ�ݰ汾:DLT645�޷���ȡ������DLT645��
#define CORE_VERSION _T("2.4.35")

//DLT645���ȡ�Ľ�
#define CORE_VERSION _T("2.4.36")


//ɾȥdrop table���в�������
#define CORE_VERSION _T("2.4.37")


//��һ�δ������ݿ�ʱʹ��root�û���
#define CORE_VERSION _T("2.4.38")

//�Ǳ������ݿ���ݴ����û���Ȩ��
#define CORE_VERSION _T("2.4.39")


//֧��AB SLCЭ�飬����MicroLogix 1400ϵ�е�
#define CORE_VERSION _T("2.4.40")


//������M5����ʱû�в���ȫ�����ݣ���ΪͬM1��combine����
#define CORE_VERSION _T("2.4.41")



//���ӄ����Ñ�localhost��Ч���l�F�����Ŀ�Ñ����ܵ��localhost
#define CORE_VERSION _T("2.4.42")


//���ӄ����Ñ�domjobs
#define CORE_VERSION _T("2.4.43")


//modbus�Ľ���������
#define CORE_VERSION _T("2.4.44")


//modbus������ֵӳ��
#define CORE_VERSION _T("2.4.45")


//modbus AO���ױ���д
#define CORE_VERSION _T("2.4.46")


//
#define CORE_VERSION _T("2.4.47")


//rebuild database �Ľ������ݵ�domdb.4db����ʱҲ��rebuild
#define CORE_VERSION _T("2.4.48")



//֧��deeplogic_redis����
#define CORE_VERSION _T("2.4.49")


//֧��modbusдֵ����
#define CORE_VERSION _T("2.4.50")

//modbus���������Ż�
#define CORE_VERSION _T("2.4.51")

//modbus���������Ż�
#define CORE_VERSION _T("2.4.52")
//modbus���������Ż���������ʱ���Ῠ̫��
#define CORE_VERSION _T("2.4.53")


//history���pointname���䵽255
#define CORE_VERSION _T("2.4.54")
//modbus������߿������⳹���Ż�
#define CORE_VERSION _T("2.4.55")


//ȥ��������ʷ��ʱ��ON UPDATE����ʱ�����
#define CORE_VERSION _T("2.4.56")
/************************************************************************/
/* 
 
V2.2.3_2018_09_28�� ������OPCд���������ȡ���µ�֧�֣�Ŀǰ����modbus�ˣ�bacnet֮ǰ��֧�֡�
                    ��������S7UDP��ȡĬ�ϸ�Ϊ��4��С���㣬֮ǰһֱ����λС��
V2.2.4:
   ������ʷ���������޸�

V2.2.5:
   ��mysql ��logͳһ���õ�logĿ¼��

   V2.2.6:
   ��ʷ���ݱ��Ϊ����������ֹ�洢��ʷʱ��ʱ���ͻ�����´治�ϣ����ڽ������ܻ�ͬһʱ�̴���ֵ

   V2.2.7:
   mysql��log����������[SYSTEM]���ֶΣ���¼�����1������ʷ����������������㲻�洢������

   V2.2.8:
   �޸���������⣬��ΪdompysiteҲ�ᴴ�������²�����ʱdomcore��ֻ����㱣���˲�������
    �޸������Ǽ�����α���ʱ�䣬����첻ͬ�Ļ�����ôǿ�Ʊ��档

   V2.2.12:
	Ĭ��ʵʱ���ݱ�����֧�ֵ�20000�����ַ���

   V2.2.15:
    ����֧�����ݿ⻻��ʱ�ı���bug

   V2.2.15:
   modbusдֵ������������ȡ

   V2.2.18:
   modbus��ȡ��������ʱ��LOG

   V2.2.19:
   ���Ӷ�persagy��������֧��

   V2.2.20:
   ���Ӷ�persagy��������֧��bug�ޏ�

   V2.3.1
   ���ַ���дֵʱ���ش�����bug

   2.3.12
   ֧�ֱ����ĸ����Զ�������

   2.4.3
   ����modbus Equipment����xȡ�@ʾ��־

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
	if(m_pDataEngineCore->m_bUpdateInputTableFromSite) //���ֳ���������ʱ�Ĵ���
	{
		if(!m_pDataEngineCore->UpdateInputTable()) //���ֳ���ȡ�źţ�����realtime_input����
		{
			PrintLog(L"ERROR: UpdateInputTable Failed. Will delay 60 seconds...\r\n",true);
			Sleep(60*1000);
			return false;
		}
	}
	else  //�����ֳ�ͨѶ����ģ�����
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
	if(!m_bFitstStart && m_nDelayWhenStart <=oleSpanTime.GetTotalSeconds())			//��������1���Ӻ���ִ�в���������
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
	else												//δ��1������ղ���������
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
	
	//��ʼ��Dump�ļ�
	DeclareDumpFile();

	//���ע���
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

	//������������
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

	//ͳ����������
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


	int nTimeCount = 15;  //30s����һ��ini�ļ�
	while(!m_bExitCore)
	{
		Sleep(500);
		
		GetLocalTime(&m_stNow);
		//��ȡ���룬����input��ִ�����
		if(!m_bExitCore)
			UpdateInputOutput();

		oleNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan oleSpan = oleNow - m_oleRunTime;

		//������30����һ�ε�����
		if(oleSpan.GetTotalSeconds() >= 30)
		{
			//���·�����ʱ��
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


				if(!m_pDataEngineCore->m_bUpdateInputTableFromSite) //����ģʽ��ֱ�Ӳ���ʵʱ��
				{
					m_pDataEngineCore->GetRealTimeDataAccess()->InsertRealTimeDatas_Input_NoClear(realdataList);
				}

				m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"point_vpoint_add", L"");
			}


			//ɾ�����Ȳ���
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

		//ÿСʱ������һ��ʼҲ��һ�Σ�
		COleDateTimeSpan oleSpan2  = oleNow - m_oleCheckEveryHourTime;
		if(oleSpan2.GetTotalSeconds() >= 60*60)
		{
			//check warningrecord, ���ϸ��¼���ǰ�ı������й鵵
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
	{//�������ģʽ����Ҫ����
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
	exepath = cstrFile + L"\\domcore.ini";
	wchar_t wcChar[1024];
	wchar_t charDBBuild[1024];

	GetPrivateProfileString(L"core", L"dbbuild", L"", charDBBuild, 1024, exepath.c_str());
	wstring wstDBBuild = charDBBuild;
	WritePrivateProfileString(L"core", L"dbbuild", wstDBBuild.c_str(), exepath.c_str());
	
	//�����ݿ�
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
				// ���������н�ѹԭapk��
				CString cmd = cmdParamstr;//"java -jar " + currentPath + "\\lib\\apktool.jar d -f " + m_csTargetApk + " " + m_csTargetDir;//�������Ҫִ�е�DOSָ��
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

	//���ݿ��Զ����� 2.2->2.3
	if(Project::Tools::IsDoubleEqual(fMySQLbeopCurVersion, 2.2))
	{
		PrintLog(L"INFO : Update Database From 2.2 To 2.3.\r\n");
		m_pDataEngineCore->DatabaseUpdateFrom22To23(wstrDBName);
	}

	//���ݿ��Զ����� 2.3->2.4
	fMySQLbeopCurVersion = m_pDataEngineCore->GetMySQLDBVersion(wstrDBName);
	if(Project::Tools::IsDoubleEqual(fMySQLbeopCurVersion, 2.3))
	{
		PrintLog(L"INFO : Update Database From 2.3 To 2.4.\r\n");
		m_pDataEngineCore->DatabaseUpdateFrom23To24(wstrDBName);
	}

	//check new db
	m_pDataEngineCore->DatabaseCreateNewInputOutputTable(wstrDBName);




	//���ӳɹ�һ�ξͿ�������dbbuild
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

	//��ȡ���������ݿ�
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
		PrintLog(L"ERROR: Read domdb.4db Info(IP,Name) Failed, Please Check the domdb.4db File.\r\n");	//edit 2016-03-11 S3db���ݱ�project_config��Ϣ�����ˣ�
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}

	//����ini�ļ� ������ʱ��
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

	//��ȡѡ��
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
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"starttime",strTimeServer);		//Core ����ʱ��
	Project::Tools::SysTime_To_OleTime(m_stNow,m_oleStartTime);

	//����ļ��ֱ�洢������
	wstring wstrFilePerTable = L"";
	if(m_pDataAccess_Arbiter->ReadMysqlVariable(L"innodb_file_per_table",wstrFilePerTable))
	{
		if(wstrFilePerTable == L"OFF")
		{
			PrintLog(L"ERROR : Check MySQL Setting(innodb_file_per_table(OFF))\r\n");
		}
	}

	//��ʼ��COM ���ý��̰�ȫ ��˲����̶߳�ȡOPC��
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

	//д��Core�İ汾��
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"version",CORE_VERSION);

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runs7",L"0");//��ʼ��Ϊ0

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corestatus", L"busy");

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"redundecymode",L"0"); 

	//�����е�s3db�ļ�·��д��mysql��unit01��corefilepath
	bool bReplace = false;
	wstring strSavePath = Replace(strDBFilePath,L"\\",L"\\\\",bReplace);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corefilepath",strSavePath);
	
	//��������
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runlogic",L"0");				//���������ǰ����0
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"rundtuengine",L"0");			//���������ǰ����0
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runwatch",L"0");				//���������ǰ����0
	WritePrivateProfileString(L"beopmaster", L"EnableCheckWatch",L"0",exepath.c_str());
	WritePrivateProfileString(L"beopmaster", L"EnableCheckLogic",L"0",exepath.c_str());
	WritePrivateProfileString(L"beopmaster", L"EnableCheckDTUEngine",L"0",exepath.c_str());

	//�������
	wstring strIP = Project::Tools::AnsiToWideChar(m_pDataEngineCore->m_dbset.strDBIP.data());
	wchar_t charServerIP[1024];
	wsprintf(charServerIP, L"%s", wstrServerIP.c_str());

	GetPrivateProfileString(L"beopmaster", L"RedundencyIP", L"", charServerIP, 1024, exepath.c_str());
	wstring strIP2 = charServerIP;
	WritePrivateProfileString(L"beopgateway", L"RedundencyIP", strIP2.c_str(), exepath.c_str()); //��һ������Ĭ���Ƿ���ģʽ��1�����ֳ�


	strLog.Format(_T("INFO : Check Redundency(IP: %s)...\r\n"), strIP2.c_str());
	PrintLog(strLog.GetString(),false);

	m_pDataEngineCore->m_dbset.strDBIP2 = Project::Tools::WideCharToAnsi(strIP2.c_str()) ;
	//m_strIP2��Ϊ�գ������������ݿ�����

	if(strIP2.length()>=7)
	{
		PrintLog(L"INFO : Init Redundency Thread...\r\n",false);
		if(m_pRedundencyManager == NULL)
			m_pRedundencyManager = new CRedundencyManager;
		m_pRedundencyManager->Init(m_pDataEngineCore->m_nMainDropInterval, strIP.c_str() , strIP2.c_str());
	}

	//m_strIP2��Ϊ�գ������������ݿ��̣߳��������������������ļ�
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
	m_pDataEngineCore->m_bUpdateInputTableFromSite = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"sitemode",L"1"))==L"1"; //��һ������Ĭ���Ƿ���ģʽ��1�����ֳ�
	if(m_pDataEngineCore->m_bUpdateInputTableFromSite)
		PrintLog(L"INFO : Init Engine(Site Mode)...\r\n",false);
	else
		PrintLog(L"INFO : Init Engine(Simulation Mode)...\r\n",false);

	///DTU����///////////////////////////////////////////////////////////////////////
	wstring wstrDTUType = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"DTUType", L"0");
	wstring wstrRunDTUEngine = L"0";
	if(_wtoi(wstrDTUType.c_str()) >= 4)			//��Ҫ����DTUEngine
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

	///��ʼ��DTU����///////////////////////////////////////////////////////////////////////
	wstring wstrDTUport = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUComPort");		//��DTU Com��   Ĭ��Ϊ����1
	if(wstrDTUport == L"" || wstrDTUport == L"0")
		wstrDTUport = L"1";

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"DTUComPort", wstrDTUport);
	wstring wstrDTUSendMinType = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUMinType");		//��DTU��С�������ͼ�� 2:1���ӵ�  3:5���� Ĭ��Ϊ3
	if(wstrDTUSendMinType == L"" || wstrDTUSendMinType == L"0")
		wstrDTUSendMinType = L"3";

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"DTUMinType", wstrDTUSendMinType);

	bool bWithDTU = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUEnabled"))==L"1";
	bool bDTUChecked = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUChecked"))==L"1";	
	bool bDTURecCmd = bWithDTU;
	bool bDisableSendAll = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"DTUDisableSendAll"))==L"1";		//�Ƿ������㷢��
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
	
	//TCP���緢��ģʽ
	bool bWithTcpSender = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"TcpSenderEnabled"))==L"1";

	wstring wstrTcpSenderport = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderPort",L"9500");		//��TCP�˿�   Ĭ��Ϊ9500
	if(wstrTcpSenderport == L"" || wstrTcpSenderport == L"0")
		wstrTcpSenderport = L"9500";

	wstring wstrTcpSenderIP = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderIP",L"");		//��TCP IP   Ĭ��Ϊ114.215.172.232
	if(wstrTcpSenderIP == L"" || wstrTcpSenderIP == L"0")
		wstrTcpSenderIP = L"";

	wstring wstrTcpSenderName = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"TcpSenderName",L"");		//��TCP Name  ���ڱ�ʶ��Ŀ
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

	//�Ƿ�����DTU
	wstring strRunDtu = L"0";
	if(bWithDTU || bWithTcpSender)
		strRunDtu = L"1";
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"rundtu", strRunDtu);
	
	//modbus���ò���
	bool bModbusreadonebyone = (m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"modbusreadonebyone",L"0"))==L"1";
	wstring wstrMobusInterval = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"ModbusReadIntervalMS",L"500");		//modbus��ѯ���  Ĭ��100ms
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

	//����������
	CString strPointCount;
	strPointCount.Format(_T("%d"),pPointManager->GetAllPointCount());
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"pointcount",strPointCount.GetString());		//Core�������

	//save points into unit04
	if(m_pDataAccess_Arbiter->UpdateSavePoints(pPointManager->GetPointList()))
		PrintLog(L"INFO : Updating Online Points Success.\r\n",false);
	else
		PrintLog(L"ERROR: Updating Online Points Failed.\r\n");

	//���
	//2019-08-24 golding: ����ΪʲôҪ�����
	pPointManager->ClearAllPoint();

	PrintLog(L"INFO : Starting Engine...\r\n");
	if(!m_pDataEngineCore->InitEngine())
	{
		PrintLog(L"ERROR: InitEngine Failed!\r\n");
		return false;
	}

	//���
	pPointManager->ClearAllPointListExceptOPC();

	//Sleep(800000);
	m_pDataEngineCore->EnableLog(FALSE);

	//init dataenginecore
	m_pDataEngineCore->SetDataAccess(m_pDataAccess_Arbiter);

	m_pDataEngineCore->SetDBAcessState(true);

	m_pDataEngineCore->ReadEquipmentEnableOption();

	m_hEventActiveStandby = CreateEvent(NULL, TRUE, FALSE, NULL);

	//S3DB �ļ����ؿ����߳�����
	InitS3DBDownloadManager();

	// InitAfterlogin
	if(m_pDataEngineCore->m_bUpdateInputTableFromSite)
	{
		m_pDataEngineCore->GetDataLink()->InitOPC();
	}

	//���
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
			//��¼Log
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

//bSaveLog: ǿ������д��log�ļ�
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
		// ����һ��������������¼�����ݿ⣬���
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
    //���ж��ipʱ,j��������ip�ĸ���   inet_ntoa(*(IN_ADDR*)host->h_addr_list[i] �����i���Ƕ�Ӧ��ÿ��ip   
    for(int i=0;;i++)   
    {  
        memcpy(&addr, ph->h_addr_list[i], sizeof(in_addr)); //    
        localIP=inet_ntoa(addr);   
        IPlist.push_back(localIP);  
        if(host->h_addr_list[i]+host->h_length>=host->h_name)   
        {  
            break; //����������һ��  
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
	if(m_pRedundencyManager)		//�˳�����
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
