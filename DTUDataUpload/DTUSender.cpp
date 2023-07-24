#include "StdAfx.h"
#include "DTUSender.h"
#include <afx.h>
#include "Tools/zlib.h"
#include "Tools/CustomTools/CustomTools.h"
#include "Tools/Zip/unzip.h"

#pragma comment(lib,"zlib.lib") 

//#include "../BEOPDataLink/BEOPDataLinkManager.h"

#include "DatabaseDataFetcher.h"
#include "PackageTypeDefine.h"

#include <string>
#include <iostream>
#include <sstream>
using std::wstring;
using std::string;

const int DTU_SEND_PACKAGE_INTERNAL = 2*1000;	//dtu连续发包的间隔时间
const int DTU_SEND_UNIT01_INTERNAL = 2*1000;	//dtu连续发包的间隔时间(间隔时间过短会崩溃)
const int DTU_PACKAGE_SIZE = 900;				//分包时最大包长。
const int MAX_POINT_LENGTH = 120;				//点名最大长度

const int FILE_UPDATE_LENGTH = 4000;			//文件升级固定长度为4000
const int TCP_SEND_INTERNAL = 200;				//200ms发送一帧
const int FILE_SIZE = 1024;						//小的读取文件的空间总数
const int ONE_FILE_SIZE = 4096;					//每个小的读取文件的空间

#define STR_SEP_MARK ","
#define STR_SEPGROUP_MARK ";"

const char* RDP_AT = "AT";
const char* RDP_ATOK = "AT\r\nOK\r\n";
const char* RDP_ATERROR = "AT\r\n\r\ERROR\r\n";
const char* RDP_OK = "OK";
const char* RDP_ERROR = "ERROR";
const char* RDP_LOGIN = "AT+LOGIN=admin";
const char* RDP_DTUID = "AT+GETPARAM=OPPDTUI";
const char* RDP_DSCIP = "AT+GETPARAM=CHPDSCIP";
const char* RDP_CSQ	  = "AT+CSQ";

const string DTU_PACKAGE_INDEX = "DTU_P_INDEX";	//实时数据发包编号
const string DTU_PACKAGE_BASE = "DTU_P_BASE";	//实时数据发包基数
const int	MAX_STORE_CONTENT = 1000;		//存储最近1000个包的内容

//#include "Tools/vld.h"

CDTUSender::CDTUSender(CCommonDBAccess* dbsession_history,bool bDTUChecked,bool bDTURecCmd,string strTCPName,string strHost,u_short strport,bool bDTUMode,
						int portnumber /*= 1*/, int baudrate /*= 9600*/ )
	:m_portnumber(portnumber)
	,m_baudrate(baudrate)
	,m_porthandler(new CSerialPort())
	,m_tcphandler(new CTCPDataSender())
	,m_bDTUChecked(bDTUChecked)
	,m_bDTURecCmd(bDTURecCmd)
	,m_nDTUPackageNumber(0)
	,m_nDTUPackageBase(1)
	,m_porthandler2(new CSerialPort())
	,m_bRunReserverPort(false)
	,m_nRunReserverPort(0)
	,m_strDTUID("")
	,m_strCSQ("")
	,m_strDSCIP("")
	,m_strRecFileName("")
	,m_strCoreFileName("")
	,m_nRTUAnswerMode(0)
	,m_nDTUFilePackageNumber(0)
	,m_nFileCount(0)
	,m_nRecFileCount(0)
	,m_nCoreFileCount(0)
	,m_nRecCoreFileCount(0)
	,m_recthread(NULL)
	,m_bHasLostFileFlag(false)
	,m_strUpdateExeAck("")
	,m_nLastUpdateExeSize(0)
	,m_bDTUMode(bDTUMode)
	,m_host(strHost)
	,m_port(strport)
	,m_bdtuSuccess(false)
	,m_strTcpName(strTCPName)				//名称	
	,m_dbsession_history(dbsession_history)
	,m_nTCPSendFilePackageSize(1000)
	,m_nTCPSendFilePackageInterval(2000)
{
	m_sendthread = NULL;
	m_readthread = NULL;
	m_bsendthreadexit = false;
	m_breadthreadexit = false;
	m_brecthreadexit = false;
	m_bDTU_To_RTU = false;
	m_bDTUConnectOK = false;
	m_bReceiveDTU = false;
	m_bReceiveDTUServerEvent = false;
	m_nReceiveDTUCmdType = 0;
	m_sendbuffer = "";
	m_oleUpdateExeTime = COleDateTime::GetCurrentTime();
	memset(&m_sendtime, 0x00, sizeof(SYSTEMTIME));
	m_oleLastWriteTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(1,0,0,0);
	m_vecDTUCmd.clear();
	memset(m_cRXBuff,0,sizeof(m_cRXBuff));
	m_nReceCount = 0;

	wstring wstrReserverSerial = L"0";
	wstring wstrReserverSerialPort = L"0";
	wstring wstrTCPSendFilePackageSize = L"4000";
	wstring wstrTCPSendFilePackageInterval = L"200";
	if(m_dbsession_history)
	{
		wstrReserverSerial = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"ReserverSerial",L"0");
		wstrReserverSerialPort = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"ReserverSerialPort",L"0");
		wstrTCPSendFilePackageSize = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"TCPSendFilePackageSize",L"4000"); 
		wstrTCPSendFilePackageInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"TCPSendFilePackageInterval",L"1000"); 
	}
	if(wstrReserverSerial == L"1")
		m_bRunReserverPort = true;
	m_nRunReserverPort = _wtoi(wstrReserverSerialPort.c_str());
	m_nTCPSendFilePackageSize = _wtoi(wstrTCPSendFilePackageSize.c_str());
	m_nTCPSendFilePackageInterval = _wtoi(wstrTCPSendFilePackageInterval.c_str());
}

CDTUSender::~CDTUSender(void)
{
	m_porthandler->ClosePort();
	m_porthandler2->ClosePort();
	m_tcphandler->CloseTcp();
	//===== free thread handle.
	if (m_sendthread){
		CloseHandle(m_sendthread);
		m_sendthread = NULL;
	}

	if (m_readthread){
		CloseHandle(m_readthread);
		m_readthread = NULL;
	}
}

bool CDTUSender::InitDTU()
{
	// 初始化临界区
	InitializeCriticalSection(&m_csDTUDataSync);

	if(m_bDTUMode)
	{
		wstring wstrTCPSendFilePackageSize = L"1000";
		wstring wstrTCPSendFilePackageInterval = L"2000";
		if(m_dbsession_history)
		{
			m_dbsession_history->WriteCoreDebugItemValue(L"TCPSendFilePackageSize",L"1000"); 
			m_dbsession_history->WriteCoreDebugItemValue(L"TCPSendFilePackageInterval",L"2000"); 
		}
		m_nTCPSendFilePackageSize = _wtoi(wstrTCPSendFilePackageSize.c_str());
		m_nTCPSendFilePackageInterval = _wtoi(wstrTCPSendFilePackageInterval.c_str());

		m_porthandler->SetHistoryDbCon(m_dbsession_history);
		m_bdtuSuccess = m_porthandler->InitPort(m_portnumber, m_baudrate);
		if (!m_bdtuSuccess)
		{
			CString strLog;
			strLog.Format(_T("ERROR: DTU Serial COM:%d Do Not Find Or Be Using...\r\n"),m_portnumber);
			PrintLog(strLog.GetString(),false);
			return m_bdtuSuccess;
		}

		if(m_bRunReserverPort)
		{
			m_porthandler2->SetHistoryDbCon(m_dbsession_history);
			m_bdtuSuccess = m_porthandler2->InitPort(m_nRunReserverPort, m_baudrate);
			if (!m_bdtuSuccess)
			{
				CString strLog;
				strLog.Format(_T("ERROR: DTU Reserver Serial COM:%d Do Not Find Or Be Using...\r\n"),m_nRunReserverPort);
				PrintLog(strLog.GetString(),false);
				return m_bdtuSuccess;
			}
		}

		if(m_bdtuSuccess)
		{
			m_porthandler->StartMonitoring(OnRecData,(DWORD)this);
			if(m_bRunReserverPort)
			{
				m_porthandler2->StartMonitoring(NULL,(DWORD)this);
			}

			if(m_bDTURecCmd)
			{
				m_readthread = (HANDLE)_beginthreadex(NULL, 0, DTUThreadHandRecCmd, this, NORMAL_PRIORITY_CLASS, NULL);
				//激活读
				m_porthandler->SetReceiveEvent(true);

				if(m_bDTUChecked)
				{
					//解析RDP数据
					m_bDTU_To_RTU = true;

					//进入AT模式
					SendRDP_AT();
					
					for(int i=0; i<60; ++i)
					{						
						if(m_bDTUConnectOK)
						{
							break;
						}

						Sleep(1000);
					}

					//退出AT模式
					SendRDP_Reset();

					if(!m_bReceiveDTU)
					{
						PrintLog(L"ERROR: RTU-DTU:Can't Connect to DTU...\r\n");
						PrintLog(L"ERROR: Init DTUEngine Failed...\r\n");
					}
					else if(m_bReceiveDTU && !m_bDTUConnectOK)
					{
						PrintLog(L"ERROR: RTU-DTU:Get DTU Param Failed...\r\n");
					}
					else if(m_bDTUConnectOK)
					{
						PrintLog(L"INFO : Init DTUEngine(With Receive And Check Fun) Success...\r\n",false);
					}
				}
				else
				{
					PrintLog(L"INFO : Init DTUEngine(With Receive Fun) Success...\r\n",false);
				}
			}
			else
			{
				PrintLog(L"INFO : Init DTUEngine(Without Receive Fun) Success...\r\n",false);
			}
		}
		return m_bdtuSuccess;
	}
	else			//TCPMode
	{
		m_tcphandler->SetHistoryDbCon(m_dbsession_history);
		m_bdtuSuccess = m_tcphandler->InitTcp(m_host,m_port,m_strTcpName);
		m_tcphandler->StartMonitoring(OnRecData,(DWORD)this);
		m_readthread = (HANDLE)_beginthreadex(NULL, 0, DTUThreadHandRecCmd, this, NORMAL_PRIORITY_CLASS, NULL);
		if (!m_bdtuSuccess){
			return m_bdtuSuccess;
		}

		if(m_bdtuSuccess)
		{
			PrintLog(L"INFO : Init TCPDataSender Engine Success...\r\n",false);
		}
		return m_bdtuSuccess;
	}
}

bool CDTUSender::SendData(char* buffer, int buffersize)
{
	////发送之前压缩
	//unsigned long destsize = compressBound(buffersize);
	//unsigned char* desbuff = new unsigned char[destsize];
	//memset(desbuff,0x00,destsize);
	//int com_result = compress(desbuff, &destsize, (unsigned char*)buffer, buffersize);
	
	//不压缩
	if(m_bDTUMode)
	{
		if(m_bRunReserverPort)
		{
			m_porthandler2->WriteToPort(buffer, buffersize+1);
		}
		return m_porthandler->WriteToPort(buffer, buffersize+1);
	}
	else
	{
		return m_tcphandler->WriteToPort(buffer, buffersize+1);
	}
}

bool CDTUSender::SendDataByWrap( char* buffer, int buffersize )
{
	if(m_bDTUMode)
	{
		//不压缩
		if(m_bRunReserverPort)
		{
			m_porthandler2->WriteToPortByWrap(buffer, buffersize+2);
		}
		return m_porthandler->WriteToPortByWrap(buffer, buffersize+2);
	}
	else
	{
		return m_tcphandler->WriteToPortByWrap(buffer, buffersize+2);
	}
}

UINT  CDTUSender::DTUThreadFunc(LPVOID lparam)
{
	CDTUSender* pthis = (CDTUSender*)lparam;
	COleDateTime TimeStart = COleDateTime::GetCurrentTime();
	COleDateTimeSpan timespan;

	//避免所有DTU都同时在整点发送数据，所以生成随机数 ，让DTU
	//在随机秒数内发送数据，避免拥堵。
	srand((unsigned) time(NULL));
	int randomsecond = rand()%60; 
	if (randomsecond == 60){
		randomsecond = 0;
	}

	while(true){
		SYSTEMTIME st;
		GetLocalTime(&st);
		pthis->SendData();
		Sleep(5*1000);
	}
}

bool CDTUSender::SendData()
{
	//ADEBUG(_T("enter build and send data func"));
	vector<string> bufferlist;
	SYSTEMTIME st;
	m_lock.Lock();
	BuildPackage(m_sendbuffer, bufferlist);
	st = m_sendtime;
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	string pointdataprefix = "pdp:time:";
	string timestring = Project::Tools::SystemTimeToString(st);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = pointdataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		SendDataByWrap((char*)strdata.data(), strdata.size());
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return true;
}

bool CDTUSender::SendData(const SYSTEMTIME tSendTime,  const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string pointdataprefix = "pdp:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	std::ostringstream sqlstream;
	//一批次包发个确认
	std::ostringstream sqlstream_Cfg;
	sqlstream_Cfg << "cfg:time:" << timestring << ";" << m_nDTUPackageBase << ";";
	int nCount = 0;
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		sqlstream.str("");
		sqlstream << pointdataprefix << timestring << ";" << bufferlist[i] 
					<< "0," << DTU_PACKAGE_BASE << "," << m_nDTUPackageBase			
					<< ";0," << DTU_PACKAGE_INDEX << "," << m_nDTUPackageNumber  << ";";
		sqlstream_Cfg << m_nDTUPackageNumber << ",";
		nCount++;
		if(nCount >= 20)						//100组为一回复包
		{
			string strCfg = sqlstream_Cfg.str();
			strCfg.erase(--strCfg.end());
			strCfg += ";";
			bResult = SendDataByWrap((char*)strCfg.data(), strCfg.size()) && bResult;

			nCount = 0;
			sqlstream_Cfg.str("");
			sqlstream_Cfg << "cfg:time:" << timestring << ";" << m_nDTUPackageBase << ";";
			Sleep(DTU_SEND_PACKAGE_INTERNAL);
		}

		string strdata = sqlstream.str();
		StoreSendInfo(strdata);
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}

	string strCfg = sqlstream_Cfg.str();
	strCfg.erase(--strCfg.end());
	strCfg += ";";
	bResult = SendDataByWrap((char*)strCfg.data(), strCfg.size()) && bResult;
	Sleep(DTU_SEND_PACKAGE_INTERNAL);
	return bResult;
}

void CDTUSender::BuildPackage(const string& buffer, vector<string>& resultlist)
{
	int buffersize = buffer.size();
	if (buffersize == 0)
	{
		return;
	}
	if(buffersize <= DTU_PACKAGE_SIZE)
		resultlist.push_back(buffer);
	else
	{
		string strcopy = buffer;
		while(strcopy.size() >  DTU_PACKAGE_SIZE)
		{
			int i = DTU_PACKAGE_SIZE;
			int j = 0;	//比一个点更大
			while(j < MAX_POINT_LENGTH){
				if( ((i+j) >= strcopy.size()) || strcopy[i+j] == ';')
					break;
				j++;
			}

			resultlist.push_back(strcopy.substr(0,i+j+1));
			if((i+j) < strcopy.size())
				strcopy = strcopy.substr(i+j+1);
		}
		resultlist.push_back(strcopy);
	}
}

void CDTUSender::BuildAndSendOperationRecord()
{
	std::vector<string> bufferlist;
	string buffer;
	CDatabaseDataFetcher::GetLatestOperationRecord(buffer);
	if (buffer.empty()){
		return;
	}

	BuildPackage(buffer, bufferlist);
	
	string operationrecorddataprefix = "orp:";
	for(unsigned int i = 0; i < bufferlist.size(); i++){
		string senddata = operationrecorddataprefix + bufferlist[i];
		SendDataByWrap((char*)senddata.data(), senddata.size());
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}

	CString senddatainfo;
	int packagecount = bufferlist.size();
	senddatainfo.Format(L"Send Data To DTU,  Total package Count: %d", packagecount);
	//AddLog(senddatainfo.GetString());
}

void CDTUSender::ExitThreads()
{
	m_bsendthreadexit = true;
	m_breadthreadexit = true;

	TerminateThread(m_readthread, 0);
	TerminateThread(m_sendthread, 0);
	WaitForSingleObject(m_readthread, INFINITE);
	WaitForSingleObject(m_sendthread, INFINITE);
}

UINT WINAPI CDTUSender::DTUThreadHandRecCmd( LPVOID lparam )
{
	CDTUSender* pthis = (CDTUSender*)lparam;
	if (!pthis){
		return 0;
	}

	while(!pthis->GetReadThreadExit()){
		pthis->HandRecCmd();
		Sleep(1000);
	}
	return 0;
}

bool CDTUSender::GetReadThreadExit() const
{
	return m_breadthreadexit;
}

bool CDTUSender::GetSendThreadExit() const
{
	return m_bsendthreadexit;
}

bool CDTUSender::GetRecThreadExit() const
{
	return m_brecthreadexit;
}

//void CDTUSender::BuildData( const SYSTEMTIME& st )
//{
//	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
//	m_sendbuffer.clear();
//	vector<Beopdatalink::CRealTimeDataEntry> entrylist;
//	if (m_pBeopdatalinker){
//		m_pBeopdatalinker->GetRealTimeDataEntryList(entrylist);
//	}
//	if (entrylist.empty()){
//		return;
//	}
//	for (unsigned int i = 0; i < entrylist.size(); i++)
//	{
//		char str_value[256];
//		memset(str_value, 0x00, 32);
//		sprintf_s(str_value, "%.1f", _wtof(entrylist[i].GetValue().c_str()));
//		m_sendbuffer += entrylist[i].GetName();
//		m_sendbuffer += STR_SEP_MARK;
//		m_sendbuffer += str_value;
//		m_sendbuffer += STR_SEPGROUP_MARK;
//	}
//	int pointcount = entrylist.size();
//	CString debuginfo;
//	debuginfo.Format(L"Update DTU Data In Mem, Total Points: %d", pointcount);
//	AddLog(debuginfo.GetString());
//
//	m_sendtime = st;
//}

//void CDTUSender::AddLog( const wchar_t* loginfo )
//{
//	if (m_pBeopdatalinker)
//	{
//		m_pBeopdatalinker->AddLog(loginfo);
//		m_pBeopdatalinker->SendLogMsg(L"DTU", loginfo);
//	}
//}

//将接收内容改为char*存储  20150402 robert
//LRESULT CDTUSender::OnRecData( const unsigned char* pRevData, DWORD length ,DWORD userdata)
//{
//	//
//	CDTUSender *pDTUCtrl=(CDTUSender *)userdata;
//	if(pDTUCtrl != NULL)
//	{
//		string strReceive = (char*)pRevData;
//		pDTUCtrl->m_reclock.Lock();
//		pDTUCtrl->m_qRecDTUCmd.push(strReceive);
//		pDTUCtrl->m_reclock.UnLock();
//	}
//	return 0;
//}

LRESULT CDTUSender::OnRecData( const unsigned char* pRevData, DWORD length ,DWORD userdata)
{
	//
	CDTUSender *pDTUCtrl=(CDTUSender *)userdata;
	if(pDTUCtrl != NULL)
	{
		DTUServerCmdInfo data;
		memset(data.cData,0,g_nPackageSize);
		length = (length>g_nPackageSize)?g_nPackageSize:length;
		memcpy(data.cData,pRevData,length);
		data.nLength = length;
		pDTUCtrl->StoreDTUCmd(data);
	}
	return 0;
}

void CDTUSender::GetOutputByRecData( const char* newbuffer,vector<Beopdatalink::CRealTimeDataEntry>& entrylist )
{
	string strtime;
	char* p = strtok((char*)newbuffer, ";");
	if (p){
		strtime = p;
	}
	SYSTEMTIME st;
	String_To_SysTime(p,st);

	p = strtok(NULL, ";");
	while(p){
		char name[256], value[256];
		memset(name, 0, 256);
		memset(value, 0, 256);
		int i = 0;
		char* q = p;
		while(*q++){
			i++;
			if((*q) == ','){
				break;
			}
		}
		strcpy(value, q+1);
		*q = '\0';
		strcpy(name, p);

		CRealTimeDataEntry entry;
		entry.SetName(name);
		entry.SetTimestamp(st);
		entry.SetValue(AnsiToWideChar(value));
		entrylist.push_back(entry);
		p = strtok(NULL, ";");
	}
}

bool CDTUSender::SendWarningData(const SYSTEMTIME tSendTime,  const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string pointdataprefix = "wdp:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = pointdataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::SendWarnigOperationData(const SYSTEMTIME tSendTime,  const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string pointdataprefix = "wop:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = pointdataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::SendQueryState()
{
	char cAdmin[6] = {"admin"};
	char sendBuffer[17];
	memset(sendBuffer,0,17);
	sendBuffer[0] = 0x7D;
	sendBuffer[1] = 0x7D;
	sendBuffer[2] = 0x7D;
	sendBuffer[3] = 0x00;
	sendBuffer[4] = 0x10;
	sendBuffer[5] = 0x00;
	sendBuffer[6] = 0x00;
	memcpy(sendBuffer+7,cAdmin,5);
	sendBuffer[12] = 0x00;
	sendBuffer[13] = 0x7F;
	sendBuffer[14] = 0x7F;
	sendBuffer[15] = 0x7F;
	sendBuffer[16] = 0xFF;
	bool bResult = SendData(sendBuffer,17);

	//char cAdmin[4] = {"+++"};
	//char sendBuffer[4];
	//sendBuffer[0] = 0x2b;
	//sendBuffer[1] = 0x2b;
	//sendBuffer[2] = 0x2b;
	//sendBuffer[3] = 0x00;
	////memset(sendBuffer,0,4);
	////memcpy(sendBuffer,cAdmin,3);
	//bool bResult = SendData(sendBuffer,3);

	//char sendBuffer[14];
	//memset(sendBuffer,0,14);
	//sendBuffer[0] = 0x7D;
	//sendBuffer[1] = 0x7D;
	//sendBuffer[2] = 0x7D;
	//sendBuffer[3] = 0x00;
	//sendBuffer[4] = 0x0D;
	//sendBuffer[5] = 0x07;		//功能标识
	//sendBuffer[6] = 0x00;
	//sendBuffer[7] = 0xF0;
	//sendBuffer[8] = 0x04;
	//sendBuffer[9] = 0x00;
	//sendBuffer[10] = 0x7F;
	//sendBuffer[11] = 0x7F;
	//sendBuffer[12] = 0x7F;
	//sendBuffer[13] = 0x00;
	//bool bResult = SendData(sendBuffer,14);
	return bResult;
}

bool CDTUSender::SendLogin()
{

	char cAdmin[4] = {"+++"};
	char sendBuffer[4];

	memset(sendBuffer,0,4);
	memcpy(sendBuffer,cAdmin,3);
	bool bResult = SendData(sendBuffer,3);
	return bResult;
}

bool CDTUSender::SendLogin1()
{
	char sendBuffer[6];
	memset(sendBuffer,0,6);
	sendBuffer[0] = 0x2B;
	sendBuffer[1] = 0x2B;
	sendBuffer[2] = 0x2B;
	sendBuffer[3] = 0x0D;
	sendBuffer[4] = 0x0A;
	bool bResult = SendData(sendBuffer,4);

	//41 54 2B 4C 4F 47 49 4E 3D 61 64 6D 69 6E 0D 0A 
	Sleep(5000);
	char sendBuffer1[16];
	memset(sendBuffer1,0,16);
	sendBuffer1[0] = 0x41;
	sendBuffer1[1] = 0x54;
	sendBuffer1[2] = 0x2B;
	sendBuffer1[3] = 0x4C;
	sendBuffer1[4] = 0x4F;
	sendBuffer1[5] = 0x47;
	sendBuffer1[6] = 0x49;
	sendBuffer1[7] = 0x4E;
	sendBuffer1[8] = 0x3D;
	sendBuffer1[9] = 0x61;
	sendBuffer1[10] = 0x64;
	sendBuffer1[11] = 0x6D;
	sendBuffer1[12] = 0x69;
	sendBuffer1[13] = 0x6E;
	sendBuffer1[14] = 0x0D;
	sendBuffer1[15] = 0x0A;
	bResult = SendData(sendBuffer1,15);

	//41 54 2B 47 45 54 50 41 52 41 4D 3D 4F 50 50 44 54 55 49 0D 0A 
	Sleep(5000);
	char sendBuffer2[21];
	memset(sendBuffer2,0,21);
	sendBuffer2[0] = 0x41;
	sendBuffer2[1] = 0x54;
	sendBuffer2[2] = 0x2B;
	sendBuffer2[3] = 0x47;
	sendBuffer2[4] = 0x45;
	sendBuffer2[5] = 0x54;
	sendBuffer2[6] = 0x50;
	sendBuffer2[7] = 0x41;
	sendBuffer2[8] = 0x52;
	sendBuffer2[9] = 0x41;
	sendBuffer2[10] = 0x4D;
	sendBuffer2[11] = 0x3D;
	sendBuffer2[12] = 0x4F;
	sendBuffer2[13] = 0x50;
	sendBuffer2[14] = 0x50;
	sendBuffer2[15] = 0x44;
	sendBuffer2[16] = 0x54;
	sendBuffer2[17] = 0x55;
	sendBuffer2[18] = 0x49;
	sendBuffer2[19] = 0x0D;
	sendBuffer2[20] = 0x0A;
	bResult = SendData(sendBuffer2,20);

	return bResult;
}

bool CDTUSender::SendRDP_AT()
{
	//+++
	char sendBuffer[4];
	memset(sendBuffer,0,4);
	sendBuffer[0] = 0x2B;
	sendBuffer[1] = 0x2B;
	sendBuffer[2] = 0x2B;
	bool bResult = SendData(sendBuffer,2);
	return bResult;


	////AT

	//char sendBuffer[5];
	//memset(sendBuffer,0,5);
	//sendBuffer[0] = 0x41;
	//sendBuffer[1] = 0x54;
	//sendBuffer[2] = 0x0D;
	//sendBuffer[3] = 0x0A;
	//bool bResult = SendData(sendBuffer,3);
	//return bResult;
}

bool CDTUSender::SendRDP_Login()
{
	//AT+LOGIN=admin
	//41 54 2B 4C 4F 47 49 4E 3D 61 64 6D 69 6E 0D 0A 
	char sendBuffer1[16];
	memset(sendBuffer1,0,16);
	sendBuffer1[0] = 0x41;
	sendBuffer1[1] = 0x54;
	sendBuffer1[2] = 0x2B;
	sendBuffer1[3] = 0x4C;
	sendBuffer1[4] = 0x4F;
	sendBuffer1[5] = 0x47;
	sendBuffer1[6] = 0x49;
	sendBuffer1[7] = 0x4E;
	sendBuffer1[8] = 0x3D;
	sendBuffer1[9] = 0x61;
	sendBuffer1[10] = 0x64;
	sendBuffer1[11] = 0x6D;
	sendBuffer1[12] = 0x69;
	sendBuffer1[13] = 0x6E;
	sendBuffer1[14] = 0x0D;
	sendBuffer1[15] = 0x0A;
	bool bResult = SendData(sendBuffer1,15);
	return bResult;
}

bool CDTUSender::SendRDP_QueryDTUID()
{
	//AT+GETPARAM=OPPDTUI
	//41 54 2B 47 45 54 50 41 52 41 4D 3D 4F 50 50 44 54 55 49 0D 0A 
	char sendBuffer2[21];
	memset(sendBuffer2,0,21);
	sendBuffer2[0] = 0x41;
	sendBuffer2[1] = 0x54;
	sendBuffer2[2] = 0x2B;
	sendBuffer2[3] = 0x47;
	sendBuffer2[4] = 0x45;
	sendBuffer2[5] = 0x54;
	sendBuffer2[6] = 0x50;
	sendBuffer2[7] = 0x41;
	sendBuffer2[8] = 0x52;
	sendBuffer2[9] = 0x41;
	sendBuffer2[10] = 0x4D;
	sendBuffer2[11] = 0x3D;
	sendBuffer2[12] = 0x4F;
	sendBuffer2[13] = 0x50;
	sendBuffer2[14] = 0x50;
	sendBuffer2[15] = 0x44;
	sendBuffer2[16] = 0x54;
	sendBuffer2[17] = 0x55;
	sendBuffer2[18] = 0x49;
	sendBuffer2[19] = 0x0D;
	sendBuffer2[20] = 0x0A;
	bool bResult = SendData(sendBuffer2,20);
	return bResult;
}

bool CDTUSender::SendRDP_QueryDSCIP()
{
	//AT+GETPARAM=CHPDSCIP
	//41 54 2B 47 45 54 50 41 52 41 4D 3D 43 48 50 44 53 43 49 50 0D 0A
	char sendBuffer2[22];
	memset(sendBuffer2,0,22);
	sendBuffer2[0] = 0x41;
	sendBuffer2[1] = 0x54;
	sendBuffer2[2] = 0x2B;
	sendBuffer2[3] = 0x47;
	sendBuffer2[4] = 0x45;
	sendBuffer2[5] = 0x54;
	sendBuffer2[6] = 0x50;
	sendBuffer2[7] = 0x41;
	sendBuffer2[8] = 0x52;
	sendBuffer2[9] = 0x41;
	sendBuffer2[10] = 0x4D;
	sendBuffer2[11] = 0x3D;
	sendBuffer2[12] = 0x43;
	sendBuffer2[13] = 0x48;
	sendBuffer2[14] = 0x50;
	sendBuffer2[15] = 0x44;
	sendBuffer2[16] = 0x53;
	sendBuffer2[17] = 0x43;
	sendBuffer2[18] = 0x49;
	sendBuffer2[19] = 0x50;
	sendBuffer2[20] = 0x0D;
	sendBuffer2[21] = 0x0A;
	bool bResult = SendData(sendBuffer2,21);
	return bResult;
}

bool CDTUSender::SendRDP_QueryCSQ()
{
	//AT+CSQ
	//41 54 2B 43 53 51 0D 0A 
	char sendBuffer1[8];
	memset(sendBuffer1,0,8);
	sendBuffer1[0] = 0x41;
	sendBuffer1[1] = 0x54;
	sendBuffer1[2] = 0x2B;
	sendBuffer1[3] = 0x43;
	sendBuffer1[4] = 0x53;
	sendBuffer1[5] = 0x51;
	sendBuffer1[6] = 0x0D;
	sendBuffer1[7] = 0x0A;
	bool bResult = SendData(sendBuffer1,7);
	return bResult;
}

bool CDTUSender::SendRDP_Reset()
{
	//AT+RESET
	//41 54 2B 52 45 53 45 54 0D 0A 
	char sendBuffer1[10];
	memset(sendBuffer1,0,10);
	sendBuffer1[0] = 0x41;
	sendBuffer1[1] = 0x54;
	sendBuffer1[2] = 0x2B;
	sendBuffer1[3] = 0x52;
	sendBuffer1[4] = 0x45;
	sendBuffer1[5] = 0x53;
	sendBuffer1[6] = 0x45;
	sendBuffer1[7] = 0x54;
	sendBuffer1[8] = 0x0D;
	sendBuffer1[9] = 0x0A;
	bool bResult = SendData(sendBuffer1,9);
	return bResult;
}

////未以换行结束时候的处理
//void CDTUSender::GetDTU_TO_RTU( const unsigned char* pRevData, DWORD length )
//{	
//	if(length <= 0)
//		return;
//
//	memset(m_cRTUBuffer,0,MAX_REC_COUNT);
//	memcpy(m_cRTUBuffer,(char*)pRevData,length);
//
//	if(strstr(m_cRTUBuffer,RDP_ATOK) != NULL)			//AT
//	{		
//		SendRDP_Login();
//		PrintLog(L"RTU-DTU:RUN AT OK...\r\n");
//		m_bReceiveDTU = true;
//		return ;
//	}
//	else if(strstr(m_cRTUBuffer,RDP_ATERROR) != NULL)
//	{
//		SendRDP_AT();
//		m_bReceiveDTU = true;
//		return ;
//	}
//
//	if(strstr(m_cRTUBuffer,RDP_LOGIN) != NULL)			//AT+LOGIN=admin
//	{
//		if(strstr(m_cRTUBuffer,RDP_OK) != NULL)		//OK
//		{
//			SendRDP_QueryDTUID();
//			
//			PrintLog(L"RTU-DTU:AT+LOGIN=admin OK...\r\n");
//		}
//		else if(strstr(m_cRTUBuffer,RDP_ERROR) != NULL)	//ERRor
//		{
//			SendRDP_Login();
//		}
//		return ;
//	}
//
//	if(strstr(m_cRTUBuffer,RDP_DTUID) != NULL)			//AT+GETPARAM=OPPDTUI
//	{
//		if(strstr(m_cRTUBuffer,RDP_OK) != NULL)		//OK
//		{
//			char* p = strtok(m_cRTUBuffer, "\n");			
//			p = strtok(NULL, "\r");
//
//			m_strDTUID = p;
//			wstring str = Project::Tools::AnsiToWideChar(p);
//			CString strOut;
//			strOut.Format(_T("RTU-DTU:%s...\r\n"),str.c_str());
//			_tprintf(strOut);
//
//			SendRDP_QueryCSQ();
//		}
//		else if(strstr(m_cRTUBuffer,RDP_ERROR) != NULL)	//ERRor
//		{
//			SendRDP_QueryDTUID();
//		}
//		return ;
//	}
//	
//	if(strstr(m_cRTUBuffer,RDP_CSQ) != NULL)			//AT+CSQ
//	{
//		if(strstr(m_cRTUBuffer,RDP_OK) != NULL)		//OK
//		{
//			char* p = strtok(m_cRTUBuffer, "\n");			
//			p = strtok(NULL, ",");
//			m_strCSQ = p;
//			wstring str = Project::Tools::AnsiToWideChar(p);
//			CString strOut;
//			strOut.Format(_T("RTU-DTU:%s...\r\n"),str.c_str());
//			_tprintf(strOut);
//
//			SendRDP_QueryDSCIP();
//		}
//		else if(strstr(m_cRTUBuffer,RDP_ERROR) != NULL)	//ERRor
//		{
//			SendRDP_QueryCSQ();
//		}
//		return ;
//	}
//
//	if(strstr(m_cRTUBuffer,RDP_DSCIP) != NULL)			//AT+GETPARAM=CHPDSCIP
//	{	
//		if(strstr(m_cRTUBuffer,RDP_OK) != NULL)		//OK
//		{
//			char* p = strtok(m_cRTUBuffer, "\n");			
//			p = strtok(NULL, "\r");
//			m_strDSCIP = p;
//			wstring str = Project::Tools::AnsiToWideChar(p);
//			CString strOut;
//			strOut.Format(_T("RTU-DTU:%s...\r\n"),str.c_str());
//			_tprintf(strOut);
//			m_bDTUConnectOK = true;
//		}
//		return ;
//	}
//}

void CDTUSender::GetDTU_TO_RTU( const unsigned char* pRevData, DWORD length )
{	
	if(length <= 0)
		return;

	memset(m_cRTUBuffer,0,MAX_REC_COUNT);
	memcpy(m_cRTUBuffer,(char*)pRevData,length);

	if(strstr(m_cRTUBuffer,"AT\r\n") != NULL)
	{
		m_nRTUAnswerMode = 1;			//AT命令
	}
	else if(strstr(m_cRTUBuffer,"AT+LOGIN=admin\r\n") != NULL)
	{
		m_nRTUAnswerMode = 2;			//AT+LOGIN=admin
	}
	else if(strstr(m_cRTUBuffer,"AT+GETPARAM=OPPDTUI\r\n") != NULL)
	{
		m_nRTUAnswerMode = 3;			//AT+GETPARAM=OPPDTUI
	}
	else if(strstr(m_cRTUBuffer,"AT+CSQ\r\n") != NULL)
	{
		m_nRTUAnswerMode = 4;			//AT+CSQ
	}
	else if(strstr(m_cRTUBuffer,"AT+GETPARAM=CHPDSCIP\r\n") != NULL)
	{
		m_nRTUAnswerMode = 5;			//AT+GETPARAM=CHPDSCIP
	}
	else if(strstr(m_cRTUBuffer,"OPPDTUI=") != NULL)
	{
		char* p = strtok(m_cRTUBuffer, "\r");
		if(p)
		{
			m_strDTUID = p;
			wstring str = Project::Tools::AnsiToWideChar(p);
			CString strOut;
			strOut.Format(_T("INFO : RTU-DTU:%s...\r\n"),str.c_str());
			PrintLog(strOut.GetString(),false);
		}
	}
	else if(strstr(m_cRTUBuffer,"CHPDSCIP=") != NULL)
	{
		char* p = strtok(m_cRTUBuffer, "\r");
		if(p)
		{
			m_strDSCIP = p;
			wstring str = Project::Tools::AnsiToWideChar(p);
			CString strOut;
			strOut.Format(_T("INFO : RTU-DTU:%s...\r\n"),str.c_str());
			PrintLog(strOut.GetString(),false);
		}
		m_bDTUConnectOK = true;
	}
	else if(strstr(m_cRTUBuffer,"+CSQ") != NULL)
	{
		if(strstr(m_cRTUBuffer,"+CSQ: 99,99") != NULL)
		{
			SendRDP_QueryCSQ();				//差错
		}
		else
		{
			char* p = strtok(m_cRTUBuffer, ",");
			if(p)
			{
				m_strCSQ = p;
				wstring str = Project::Tools::AnsiToWideChar(p);
				CString strOut;
				strOut.Format(_T("INFO : RTU-DTU:%s...\r\n"),str.c_str());
				PrintLog(strOut.GetString(),false);
			}
			SendRDP_QueryDSCIP();
		}
	}
	else if(strstr(m_cRTUBuffer,"AT+RESET") != NULL)
	{
		//退出
		m_bDTU_To_RTU = false;

	}
	else if(strstr(m_cRTUBuffer,"OK") != NULL)
	{
		switch(m_nRTUAnswerMode)
		{
		case 1:
			{
				SendRDP_Login();
				PrintLog(L"INFO : RTU-DTU:RUN AT OK...\r\n",false);
				m_bReceiveDTU = true;
			}
			break;
		case 2:
			{
				SendRDP_QueryDTUID();
				PrintLog(L"INFO : RTU-DTU:AT+LOGIN=admin OK...\r\n",false);
			}
			break;
		case 3:
			{
				SendRDP_QueryCSQ();
			}
			break;
		case 4:
			{
				//SendRDP_QueryDSCIP();
			}
			break;
		default:
			break;
		}
	}
	else if(strstr(m_cRTUBuffer,"ERROR") != NULL)
	{
		switch(m_nRTUAnswerMode)
		{
		case 1:
			{
				SendRDP_AT();
				m_bReceiveDTU = true;
			}
			break;
		case 2:
			SendRDP_Login();
			break;
		case 3:
			SendRDP_QueryDTUID();
			break;
		case 4:
			SendRDP_QueryCSQ();
			break;
		default:
			break;
		}
	}
}

bool CDTUSender::GetReceiveDTUServerEvent(vector<vector<string>>& vecCmd)
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_reveivelock);
	if(m_bReceiveDTUServerEvent)
	{
		vecCmd = m_vecDTUCmd;
		m_vecDTUCmd.clear();
		m_bReceiveDTUServerEvent = false;
		return true;
	}
	return false;
}

bool CDTUSender::SetReceiveDTUServerEvent( bool bReceiveDTUServerEvent )
{
	m_bReceiveDTUServerEvent = bReceiveDTUServerEvent;
	return true;
}

////处理中原本就不会含多个\n  20150402
//bool CDTUSender::AnalyzeReceive( const unsigned char* pRevData, DWORD length )
//{
//	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_reveivelock);
//	char* buffer_copy = (char*)pRevData;
//	vector<string>	vecCmd;
//	SplitString(buffer_copy,";;\n",vecCmd);
//	for(int i=0; i<vecCmd.size(); ++i)
//	{
//		if(vecCmd[i].length()<=0)
//			continue;
//
//		//解析数据
//		char* buffer = (char*)vecCmd[i].c_str();
//		rnbtech::CPackageType::DTU_DATATYPE type = rnbtech::CPackageType::GetPackageType(buffer);
//		switch(type)
//		{
//		case rnbtech::CPackageType::Type_DTUServerCmd:
//			{
//				rnbtech::CPackageType::RemovePrefix(buffer);
//				UpdateReceiveBuffer(buffer);
//			}
//			break;
//		case rnbtech::CPackageType::Type_DTUWriteTime:
//			{
//				rnbtech::CPackageType::RemovePrefix(buffer);
//				UpdateDTUWriteTime(buffer);
//			}
//			break;
//		default:
//			{
//				OutPutLogErrString(buffer);
//				_tprintf(_T("Error: DTUServer Cmd Analyze Failed.\r\n"));
//			}
//			break;
//		}
//	}
//	return true;
//}

//处理中原本就不会含多个\n  20150402
bool CDTUSender::AnalyzeReceive( const unsigned char* pRevData, DWORD length )
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_reveivelock);
	if(length <= 3)
		return false;

	//;;\n结尾
	if(pRevData[length-1] == '\n' && pRevData[length-2] == ';')
	{
		char buffer_copy[g_nPackageSize] = {0};
		length = length-3;
		length = (length>g_nPackageSize)?g_nPackageSize:length;
		memcpy(buffer_copy,pRevData,length);

		rnbtech::CPackageType::DTU_DATATYPE type = rnbtech::CPackageType::GetPackageType(buffer_copy);
		switch(type)
		{
		case rnbtech::CPackageType::Type_DTUServerCmd:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy,length);
				UpdateReceiveBuffer(buffer_copy);
			}
			break;
		case rnbtech::CPackageType::Type_DTUServerFup:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy,length);
				//处理接收升级文件
				AnalyzeUpdateFile((unsigned char*)buffer_copy);
			}
			break;
		case rnbtech::CPackageType::Type_DTUServerFrl:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy,length);
				//处理匹配文件接收
				AnalyzeRecLogFile((unsigned char*)buffer_copy);
			}
			break;
		case rnbtech::CPackageType::Type_DTUWriteTime:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy);
				UpdateDTUWriteTime(buffer_copy);
			}
			break;
		default:
			{
				OutPutLogErrString(buffer_copy);
				PrintLog(L"ERROR: DTUServer Cmd Analyze Failed.\r\n");
			}
			break;
		}
		return true;
	}
	return false;
}

void CDTUSender::UpdateReceiveBuffer( char* pBuffer )
{
	m_pReceiveDTUServer = strtok((char*)pBuffer, ";");
	OutPutLogString(m_pReceiveDTUServer);
	vector<string>	vecCmd;
	SplitString(m_pReceiveDTUServer,"|",vecCmd);
	m_vecDTUCmd.push_back(vecCmd);
	m_bReceiveDTUServerEvent = true;
}

void CDTUSender::SplitString( const std::string& strsource, const char* sep, std::vector<string>& resultlist )
{
	if( !sep)
		return;
	resultlist.clear();
	memset(m_cSpiltBuffer,0,MAX_REC_COUNT);
	memcpy(m_cSpiltBuffer,strsource.c_str(),strsource.length());
	char* token = NULL;
	char* nexttoken = NULL;
	token = strtok_s(m_cSpiltBuffer, sep, &nexttoken);

	while(token != NULL)
	{
		string str = token;
		resultlist.push_back(str);
		token = strtok_s(NULL, sep, &nexttoken);
	}
}

bool CDTUSender::SendUnit01( const SYSTEMTIME tSendTime, const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string dataprefix = "udp:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = dataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_UNIT01_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::SendCmdState(const SYSTEMTIME tSendTime, int nCmdType, int nCmdState , string strPacketIndex)
{
	bool bResult = true;

	std::ostringstream sqlstream;
	sqlstream << "ack:time:" << Project::Tools::SystemTimeToString(tSendTime) << ";" << nCmdType << ","<< nCmdState <<"," << strPacketIndex << ";";
	string strdata = sqlstream.str();
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	return bResult;
}

bool CDTUSender::Exit()
{
	m_tcphandler->Exit();
	m_porthandler->ClosePort();
	if(m_bRunReserverPort)
	{
		m_porthandler2->ClosePort();
	}
	
	m_bsendthreadexit = true;
	m_breadthreadexit = true;

	if(m_readthread)
	{
		TerminateThread(m_readthread, 0);
		WaitForSingleObject(m_readthread, INFINITE);
	}
	if(m_sendthread)
	{
		TerminateThread(m_sendthread, 0);
		WaitForSingleObject(m_sendthread, INFINITE);
	}

	return true;
}

std::string CDTUSender::GetSuccessBuffer()
{
	if(m_bDTUMode)
	{
		return m_porthandler->GetSuccessBuffer();
	}
	else
	{
		return m_tcphandler->GetSuccessBuffer();
	}
}

void CDTUSender::UpdateDTUWriteTime( char* pBuffer )			//更新写时间
{
	char*   pTime = strtok((char*)pBuffer, ";");
	if(pTime != NULL)
	{
		COleDateTime oleNow;
		wstring strItemValue = Project::Tools::AnsiToWideChar(pTime);
		Project::Tools::String_To_OleTime(strItemValue,oleNow);
		COleDateTimeSpan oleSpan = oleNow - m_oleLastWriteTime;
		if(oleSpan.GetTotalMinutes()>=10)
		{
			if(m_dbsession_history)
			{
				m_dbsession_history->WriteCoreDebugItemValue(L"dtusendtime",strItemValue);
			}
			m_oleLastWriteTime = oleNow;
		}
	}
}

bool CDTUSender::SendHeartBeat( const SYSTEMTIME tSendTime )
{

	bool bResult = true;
	string dataprefix = "hba:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	string strdata = dataprefix;
	strdata += timestring;
	strdata += ";";
	strdata += timestring;
	strdata += ";";
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	Sleep(DTU_SEND_UNIT01_INTERNAL);
	return bResult;
}

bool CDTUSender::SendExecuteLog( const SYSTEMTIME tSendTime, const string& strSendBuff )
{
	bool bResult = true;
	string strdata = "log:time:";
	strdata += Project::Tools::SystemTimeToString(tSendTime);
	strdata += ";";
	strdata += strSendBuff;
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
	Sleep(DTU_SEND_PACKAGE_INTERNAL);
	return bResult;
}

bool CDTUSender::StoreSendInfo( string strSendContent )
{
	m_mapStoreSendPackageBase[m_nDTUPackageNumber] = m_nDTUPackageBase;
	if(m_nDTUPackageNumber > MAX_STORE_CONTENT)
	{
		m_nDTUPackageNumber = 0;
		m_nDTUPackageBase++;
		if(m_nDTUPackageBase > MAX_STORE_CONTENT)
			m_nDTUPackageBase = 1;
	}
	m_mapStoreSendPackage[m_nDTUPackageNumber] = strSendContent;
	m_nDTUPackageNumber++;
	return true;
}

string CDTUSender::GetStoreInfo( int nIndex ,int& nBase)
{
	nBase = 0;
	map<int, int>::iterator iter1 =  m_mapStoreSendPackageBase.find(nIndex);
	if(iter1 != m_mapStoreSendPackageBase.end())
	{
		nBase = (*iter1).second;
	}

	map<int, string>::iterator iter =  m_mapStoreSendPackage.find(nIndex);
	if(iter != m_mapStoreSendPackage.end())
	{
		return (*iter).second;
	}
	return "";
}

bool CDTUSender::ReSendRealData( string strIndex )
{
	if(strIndex.length()<=0)
		return false;

	//发确认
	SYSTEMTIME tSendTime;
	GetLocalTime(&tSendTime);
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	std::ostringstream sqlstream_Cfg;
	sqlstream_Cfg << "cfg:time:" << timestring << ";";

	vector<string> vecIndex;
	SplitString(strIndex.c_str(),",",vecIndex);

	std::ostringstream sqlstream_Cfg_;
	bool bResult = true;
	int nCount = 0;
	int nDTUPackageBase = 0;
	for(int i=0; i<vecIndex.size(); ++i)
	{
		string strSendBuf = GetStoreInfo(atoi(vecIndex[i].c_str()),nDTUPackageBase);
		if(strSendBuf.length()>0)
		{
			sqlstream_Cfg_ << vecIndex[i] << ",";
			nCount++;
			bResult = SendDataByWrap((char*)strSendBuf.data(), strSendBuf.size()) && bResult;
			Sleep(DTU_SEND_PACKAGE_INTERNAL);
		}
	}

	sqlstream_Cfg << nDTUPackageBase << ";" << sqlstream_Cfg_.str();

	if(nCount>0)
	{
		string strCfg = sqlstream_Cfg.str();
		strCfg.erase(--strCfg.end());
		strCfg += ";";
		bResult = SendDataByWrap((char*)strCfg.data(), strCfg.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::OutPutLogString( char* pOut )
{
	char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
	setlocale( LC_ALL, "chs" );

	CString strLog;
	SYSTEMTIME st;
	GetLocalTime(&st);
	wstring strTime;
	Project::Tools::SysTime_To_String(st,strTime);
	strLog += strTime.c_str();
	strLog += _T(" - ");

	strLog += Project::Tools::AnsiToWideChar(pOut).c_str();
	strLog += _T("\n");

	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		CString strLogPath;
		strLogPath.Format(_T("%s\\dtuservercmd_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
	}
	setlocale( LC_CTYPE, old_locale ); 
	free( old_locale ); 
	return false;
}

////处理队列为string  20150402
//bool CDTUSender::HandRecCmd()
//{
//	while(!m_qRecDTUCmd.empty())
//	{
//		m_reclock.Lock();
//		string strRXBuff = m_qRecDTUCmd.front();
//		m_reclock.UnLock();
//		char* RXBuff = (char*)strRXBuff.data();
//		if(RXBuff != NULL)
//		{
//			int dwBytesRead = strlen(RXBuff);
//			if(m_nReceCount + dwBytesRead >= MAX_REC_COUNT)
//			{
//				if(m_bDTU_To_RTU)
//				{
//					GetDTU_TO_RTU((const unsigned char *)m_cRXBuff,m_nReceCount);
//				}
//				else
//				{
//					AnalyzeReceive((const unsigned char *)m_cRXBuff,m_nReceCount);
//				}
//				memset(m_cRXBuff,0,sizeof(m_cRXBuff));
//				m_nReceCount = 0;
//			}
//
//			if(dwBytesRead>0)
//			{
//				if(RXBuff[dwBytesRead-1] == '\n')		//DTUServer命令\n作为结束标志
//				{
//					memcpy(m_cRXBuff+m_nReceCount,RXBuff,dwBytesRead);
//					m_nReceCount = m_nReceCount + dwBytesRead;
//
//					if(m_bDTU_To_RTU)
//					{
//						GetDTU_TO_RTU((const unsigned char *)m_cRXBuff,m_nReceCount);
//					}
//					else
//					{
//						AnalyzeReceive((const unsigned char *)m_cRXBuff,m_nReceCount);
//					}
//					memset(m_cRXBuff,0,sizeof(m_cRXBuff));
//					m_nReceCount = 0;
//				}
//				else
//				{
//					char* token = NULL;
//					char* nexttoken = NULL;
//					token = strtok_s((char*)RXBuff, "\n", &nexttoken);
//					if(token == NULL)
//					{
//						memcpy(m_cRXBuff+m_nReceCount,nexttoken,strlen(nexttoken));
//						m_nReceCount = m_nReceCount + strlen(nexttoken);
//					}
//					else
//					{
//						int length = strlen(token);
//						memcpy(m_cRXBuff+m_nReceCount,token,length);
//						m_nReceCount = m_nReceCount + length;
//
//						int rlength = strlen(nexttoken);
//						if(rlength>0)
//						{
//							//处理
//							if(m_bDTU_To_RTU)
//							{
//								GetDTU_TO_RTU((const unsigned char *)m_cRXBuff,m_nReceCount);
//							}
//							else
//							{
//								AnalyzeReceive((const unsigned char *)m_cRXBuff,m_nReceCount);
//							}
//							//清空接收
//							memset(m_cRXBuff,0,sizeof(m_cRXBuff));
//							m_nReceCount = 0;
//						}
//
//						memcpy(m_cRXBuff+m_nReceCount,nexttoken,rlength);
//						m_nReceCount = m_nReceCount + rlength;
//					}
//				}
//			}
//		}
//		m_reclock.Lock();
//		m_qRecDTUCmd.pop();
//		m_reclock.UnLock();
//	}
//	return true;
//}

//处理队列为char*  20150403  命令以;;\n结尾
bool CDTUSender::HandRecCmd()
{
	while(!m_queCmdPaket.empty())
	{
		// 开始进入临界区
		EnterCriticalSection(&m_csDTUDataSync);
		bool bHasData = false;
		DTUServerCmdInfo data;
		if(!m_queCmdPaket.empty())
		{
			data = m_queCmdPaket.front();
			m_queCmdPaket.pop();
			bHasData = true;
		}
		LeaveCriticalSection(&m_csDTUDataSync);

		if(bHasData)
		{
			if(m_bDTU_To_RTU)
			{
				GetDTU_TO_RTU((const unsigned char *)data.cData,data.nLength);
			}
			else
			{
				char* RXBuff = data.cData;
				if(RXBuff != NULL)
				{		
					int dwBytesRead = data.nLength;
					if(m_nReceCount + dwBytesRead >= MAX_REC_COUNT)
					{
						memset(m_cRXBuff,0,sizeof(m_cRXBuff));
						m_nReceCount = 0;
					}

					//拷贝
					memcpy(m_cRXBuff+m_nReceCount,RXBuff,dwBytesRead);
					m_nReceCount = m_nReceCount + dwBytesRead;

					if(m_nReceCount > 3)
					{
						if(m_cRXBuff[m_nReceCount-1] == '\n' && m_cRXBuff[m_nReceCount-2] == ';' && m_cRXBuff[m_nReceCount-3] == ';')		//;;\n结尾
						{
							AnalyzeReceive((const unsigned char *)m_cRXBuff,m_nReceCount);
							memset(m_cRXBuff,0,sizeof(m_cRXBuff));
							m_nReceCount = 0;
						}
					}
				}	
			}
		}
	}
	return true;
}

bool CDTUSender::OutPutLogErrString( char* pOut )
{
	char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
	setlocale( LC_ALL, "chs" );

	CString strLog;
	SYSTEMTIME st;
	GetLocalTime(&st);
	wstring strTime;
	Project::Tools::SysTime_To_String(st,strTime);
	strLog += strTime.c_str();
	strLog += _T(" - ");

	strLog += Project::Tools::AnsiToWideChar(pOut).c_str();
	strLog += _T("\n");

	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		CString strLogPath;
		strLogPath.Format(_T("%s\\core_err_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);
		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
	}
	setlocale( LC_CTYPE, old_locale ); 
	free( old_locale ); 
	return false;
}

bool CDTUSender::SendCoreStart( const string strProjectName )
{
	bool bResult = true;
	string strdata = "reg:";
	strdata += strProjectName;
	strdata += ";";
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	Sleep(DTU_SEND_UNIT01_INTERNAL);
	return bResult;
}

bool CDTUSender::SendFile( string strFilePath )
{
	bool bResult = true;
	std::ostringstream sqlstream;
	if(strFilePath == "")		//查找失败
	{
		sqlstream << "fdp:3;";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	}
	else
	{
		//读文件
		gzFile gz = gzopen(strFilePath.c_str(),"rb");
		if(!gz)
			return false;

		char **cFile = new char *[FILE_SIZE];
		for(int i=0;i<FILE_SIZE;++i)
		{
			cFile[i] = new char[ONE_FILE_SIZE];
		}

		int nLastLen = 0;
		int nCount = 0;
		//开始传送文件
		for(;;)
		{
			//一次读取BLOCKSIZE大小的文件内容
			nLastLen = gzread(gz,cFile[nCount], m_nTCPSendFilePackageSize);
			cFile[nCount][nLastLen] = ';';			//加一个分号
			nCount++;
			if(nLastLen < m_nTCPSendFilePackageSize)
				break;		
		}
		//关闭文件
		gzclose(gz);

		int pos = strFilePath.find_last_of('\\');
		string strFileName(strFilePath.substr(pos + 1));

		//开始
		sqlstream.str("");
		sqlstream << "fdp:0|" << strFileName << ";";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
		Sleep(m_nTCPSendFilePackageInterval);

		string strSend = "fdp:2|";
		for(int i=0;i<nCount; ++i)
		{
			char cSend[ONE_FILE_SIZE] = {0};
			memcpy(cSend,strSend.data(),6);
			unsigned int length = 0;
			if(i==nCount-1)
			{
				length = nLastLen+1;
			}
			else
			{
				length = m_nTCPSendFilePackageSize+1;
			}
			memcpy(cSend+11,&length,4);
			memcpy(cSend+6,&i,4);
			cSend[10] = '|';
			cSend[15] = '|';
			memcpy(cSend+16,cFile[i],length);
			bResult = SendDataByWrap(cSend, length+16);
			Sleep(m_nTCPSendFilePackageInterval);
		}

		//释放
		for(int i=0;i<FILE_SIZE;++i)  
		{  
			delete cFile[i];  
			cFile[i] = NULL;  
		}  
		delete [ONE_FILE_SIZE]cFile;  
		cFile = NULL;     

		//结束
		char cSend1[256] = {0};
		strSend = "fdp:1|";
		memcpy(cSend1,strSend.data(),6);
		memcpy(cSend1+11,&nLastLen,4);
		memcpy(cSend1+6,&nCount,4);
		cSend1[10] = '|';
		cSend1[15] = ';';

		bResult = SendDataByWrap(cSend1,16);
		Sleep(m_nTCPSendFilePackageInterval);
	}

	return bResult;
}

string CDTUSender::GetDTUCSQInfo()
{
	return m_strCSQ;
}

bool CDTUSender::SendRealDataFile( string strFilePath ,int nPointCount)
{
	bool bResult = true;
	std::ostringstream sqlstream,sqlstream_index;
	if(strFilePath == "")		//查找失败
	{
		sqlstream << "frp:3;";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	}
	else
	{
		//读文件
		gzFile gz = gzopen(strFilePath.c_str(),"rb");
		if(!gz)
			return false;

		char **cFile = new char *[FILE_SIZE];
		for(int i=0;i<FILE_SIZE;++i)
		{
			cFile[i] = new char[ONE_FILE_SIZE];
		}

		int nLastLen = 0;
		int nCount = 0;
		//开始传送文件
		for(;;)
		{
			//一次读取BLOCKSIZE大小的文件内容
			nLastLen = gzread(gz,cFile[nCount], m_nTCPSendFilePackageSize);
			cFile[nCount][nLastLen] = ';';			//加一个分号
			nCount++;
			if(nLastLen < m_nTCPSendFilePackageSize)
				break;		
		}
		//关闭文件
		gzclose(gz);

		int pos = strFilePath.find_last_of('\\');
		string strFileName(strFilePath.substr(pos + 1));

		//开始
		sqlstream.str("");
		sqlstream << "frp:0|" << strFileName << ";" << nPointCount << ";";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
		Sleep(m_nTCPSendFilePackageInterval);

		string strSend = "frp:2|";
		for(int i=0;i<nCount; ++i)
		{
			char cSend[ONE_FILE_SIZE] = {0};
			memcpy(cSend,strSend.data(),6);
			unsigned int length = 0;
			if(i==nCount-1)
			{
				length = nLastLen+1;
			}
			else
			{
				length = m_nTCPSendFilePackageSize+1;
			}
			memcpy(cSend+11,&length,4);
			if(m_nDTUFilePackageNumber==59)			//因为长度位置会变成; 影响解析
				m_nDTUFilePackageNumber++;
			memcpy(cSend+6,&m_nDTUFilePackageNumber,4);
			sqlstream_index << m_nDTUFilePackageNumber << ",";
			m_nDTUFilePackageNumber++;
			if(m_nDTUFilePackageNumber>=1000)
			{
				m_nDTUFilePackageNumber = 0;
			}

			cSend[10] = '|';
			cSend[15] = '|';
			memcpy(cSend+16,cFile[i],length);
			bResult = SendDataByWrap(cSend, length+16);
			Sleep(m_nTCPSendFilePackageInterval);
		}

		//释放
		for(int i=0;i<FILE_SIZE;++i)  
		{  
			delete cFile[i];  
			cFile[i] = NULL;  
		}  
		delete [ONE_FILE_SIZE]cFile;  
		cFile = NULL;     
	
		//结束
		char cSend1[256] = {0};
		strSend = "frp:1|";
		memcpy(cSend1,strSend.data(),6);
		memcpy(cSend1+11,&nLastLen,4);
		memcpy(cSend1+6,&nCount,4);

		string strFileIndex = sqlstream_index.str();
		strFileIndex.erase(--strFileIndex.end());
		strFileIndex += ";";
		memcpy(cSend1+16,strFileIndex.data(),strFileIndex.size());
		cSend1[10] = '|';
		cSend1[15] = ';';

		bResult = SendDataByWrap(cSend1,16+strFileIndex.size());
		Sleep(m_nTCPSendFilePackageInterval);
	}

	return bResult;
}

bool CDTUSender::StoreDTUCmd( DTUServerCmdInfo data )
{
	// 开始进入临界区
	EnterCriticalSection(&m_csDTUDataSync);
	m_queCmdPaket.push(data);
	LeaveCriticalSection(&m_csDTUDataSync);
	return true;
}

void CDTUSender::AnalyzeUpdateFile( const unsigned char* buffer )
{
	if(buffer != NULL)
	{
		m_oleUpdateExeTime = COleDateTime::GetCurrentTime();

		if(buffer[0] == '0')		//开始
		{
			m_nCoreFileCount = 0;
			m_nLastUpdateExeSize = 0;
			m_mapFileUpdateState.clear();
			char cFileName[256] = {0};
			memcpy(cFileName,buffer+2,256);
			char* p = strtok(cFileName, "|");
			if(p)
			{
				CString str;
				str.Format(_T("INFO : Receive Core Update File :%s...\r\n"),Project::Tools::AnsiToWideChar(p).c_str());
				PrintLog(str.GetString(),false);
				m_strCoreFileName = p;

				p = strtok(NULL, ";");
				if(p)
				{
					m_nCoreFileCount = ATOI(p);

					for(int i=0; i<m_nCoreFileCount; ++i)
					{
						m_mapFileUpdateState[i] = 0;
					}
				}
			}
			m_nRecCoreFileCount = 0;
		}
		else if(buffer[0] == '1')
		{
			m_nCoreFileCount = BufferToInt(buffer+2);
			int nLastLen =   BufferToInt(buffer+7);
			m_nLastUpdateExeSize = nLastLen;
			if(m_nCoreFileCount == m_nRecCoreFileCount)
			{
				wstring wstrFileFolder;
				Project::Tools::GetSysPath(wstrFileFolder);
				SYSTEMTIME sNow;
				GetLocalTime(&sNow);
				wstrFileFolder += L"\\fileupdate";
				if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
				{				
					if(m_strCoreFileName.length() <= 0)
						return;
					CString strFilePath;
					strFilePath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
					CFile file(strFilePath, CFile::modeCreate|CFile::modeWrite);
					file.Seek(0,CFile::begin);

					for(int i=0; i<m_nCoreFileCount; i++)
					{						
						if(i==m_nCoreFileCount-1)
						{
							file.Write(m_cFileUpdateBuffer[i],nLastLen);
						}
						else
						{
							file.Write(m_cFileUpdateBuffer[i],950);
						}
						file.Seek(0,CFile::end);
					}
					file.Close(); 
					CString str;
					str.Format(_T("INFO : Receive Core Update File(%s)...\r\n"),Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
					PrintLog(str.GetString(),false);

					//解压文件 根据;划分值
					HZIP hz = OpenZip(strFilePath,0);
					SetUnzipBaseDir(hz,wstrFileFolder.c_str());
					ZIPENTRY ze;
					GetZipItem(hz,-1,&ze); 
					int numitems=ze.index;
					for (int j=0; j<numitems; ++j)
					{
						GetZipItem(hz,j,&ze);
						ZRESULT zr = UnzipItem(hz,j,ze.name);
						if(zr == ZR_OK)
						{
							//处理解压出来的.log文件
							CString strCSVPath;
							strCSVPath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),ze.name);
							CString strExeName = ze.name;
							if(strExeName.Find(_T("domcore.exe")))
							{
								//更新ini文件
								if(m_dbsession_history)
								{
									m_dbsession_history->WriteCoreDebugItemValue(L"updatecore",L"1");
								}
							}

							if(strExeName.Find(_T("domcore.exe")))
							{
								//更新ini文件
								if(m_dbsession_history)
								{
									m_dbsession_history->WriteCoreDebugItemValue(L"updatedog",L"1");
								}
							}

							//发送回执
							m_strUpdateExeAck = Project::Tools::WideCharToAnsi(ze.name);
						}
					}
					CloseZip(hz);
				}
				m_strRecFileName = "";
			}
			else		//有丢包   启动线程去像服务器要丢失的包  直到接收时间超过1h，本次更新失败
			{
				//
				m_bHasLostFileFlag = true;
			}
		} 
		else if(buffer[0] == '2')
		{
			int nPackageIndex = BufferToInt(buffer+2);
			int nLength =   BufferToInt(buffer+7);
			if(nPackageIndex>=0 && nPackageIndex<=1000 && nLength <= g_nPackageSize)
			{
				memset(m_cFileUpdateBuffer[nPackageIndex],0,sizeof(m_cFileUpdateBuffer[nPackageIndex]));
				memcpy(m_cFileUpdateBuffer[nPackageIndex],buffer+12,nLength);
				m_mapFileUpdateState[nPackageIndex] = 1;
				m_nRecCoreFileCount++;
				CString str;
				str.Format(_T("INFO : Receive Core Update File Package:%d/%d.\r\n"),nPackageIndex,m_nCoreFileCount);
				PrintLog(str.GetString(),false);
			}
			else
			{
				int a = 0;
			}
		}
		else if(buffer[0] == '4')		//补包后确认
		{
			if(ScanAndSendLostPackage())
			{
				m_bHasLostFileFlag = true;
			}
			else
			{
				wstring wstrFileFolder;
				Project::Tools::GetSysPath(wstrFileFolder);
				SYSTEMTIME sNow;
				GetLocalTime(&sNow);
				wstrFileFolder += L"\\fileupdate";
				if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
				{				
					if(m_strCoreFileName.length() <= 0)
						return;
					CString strFilePath;
					strFilePath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
					CFile file(strFilePath, CFile::modeCreate|CFile::modeWrite);
					file.Seek(0,CFile::begin);

					for(int i=0; i<m_nCoreFileCount; i++)
					{						
						if(i==m_nCoreFileCount-1)
						{
							file.Write(m_cFileUpdateBuffer[i],m_nLastUpdateExeSize);
						}
						else
						{
							file.Write(m_cFileUpdateBuffer[i],950);
						}
						file.Seek(0,CFile::end);
					}
					file.Close(); 
					CString str;
					str.Format(_T("INFO : Receive Core Update File Package(%s)...\r\n"),Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
					PrintLog(str.GetString(),false);

					//解压文件 根据;划分值
					HZIP hz = OpenZip(strFilePath,0);
					SetUnzipBaseDir(hz,wstrFileFolder.c_str());
					ZIPENTRY ze;
					GetZipItem(hz,-1,&ze); 
					int numitems=ze.index;
					for (int j=0; j<numitems; ++j)
					{
						GetZipItem(hz,j,&ze);
						ZRESULT zr = UnzipItem(hz,j,ze.name);
						if(zr == ZR_OK)
						{
							//处理解压出来的.log文件
							CString strCSVPath;
							strCSVPath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),ze.name);
							//ReadCSVFile(Project::Tools::WideCharToAnsi(strCSVPath).c_str());

							//发送回执
							m_strUpdateExeAck = Project::Tools::WideCharToAnsi(strCSVPath);
						}
					}
					CloseZip(hz);
				}
			}
		}
	}
}

void CDTUSender::AnalyzeRecLogFile( const unsigned char* buffer )
{
	if(buffer != NULL)
	{
		if(buffer[0] == '0')		//开始
		{
			char cFileName[256] = {0};
			memcpy(cFileName,buffer+2,256);
			char* p = strtok(cFileName, ";");
			if(p)
			{
				CString str;
				str.Format(_T("INFO : Receive Lost File :%s...\r\n"),Project::Tools::AnsiToWideChar(p).c_str());
				PrintLog(str.GetString(),false);(str);
				m_strRecFileName = p;
			}
			m_nFileCount = 0;
			m_nRecFileCount = 0;
		}
		else if(buffer[0] == '1')
		{
			m_nFileCount = BufferToInt(buffer+2);
			int nLastLen =   BufferToInt(buffer+7);
			if(m_nFileCount == m_nRecFileCount)
			{
				wstring wstrFileFolder;
				Project::Tools::GetSysPath(wstrFileFolder);
				SYSTEMTIME sNow;
				GetLocalTime(&sNow);
				wstrFileFolder += L"\\filelog";
				if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
				{				
					if(m_strRecFileName.length() <= 0)
						return;
					CString strFilePath;
					strFilePath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),Project::Tools::AnsiToWideChar(m_strRecFileName.c_str()).c_str());
					CFile file(strFilePath, CFile::modeCreate|CFile::modeWrite);
					file.Seek(0,CFile::begin);

					for(int i=0; i<m_nFileCount; i++)
					{						
						if(i==m_nFileCount-1)
						{
							file.Write(m_cFileBuffer[i],nLastLen);
						}
						else
						{
							file.Write(m_cFileBuffer[i],950);
						}
						file.Seek(0,CFile::end);
					}
					file.Close(); 
					CString str;
					str.Format(_T("INFO : Receive Lost File Package(%s)...\r\n"),Project::Tools::AnsiToWideChar(m_strRecFileName.c_str()).c_str());
					PrintLog(str.GetString(),false);

					//解压文件 根据;划分值
					HZIP hz = OpenZip(strFilePath,0);
					SetUnzipBaseDir(hz,wstrFileFolder.c_str());
					ZIPENTRY ze;
					GetZipItem(hz,-1,&ze); 
					int numitems=ze.index;
					for (int j=0; j<numitems; ++j)
					{
						GetZipItem(hz,j,&ze);
						ZRESULT zr = UnzipItem(hz,j,ze.name);
						if(zr == ZR_OK)
						{
							//处理解压出来的.csv文件
							CString strCSVPath;
							strCSVPath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),ze.name);
							//ReadCSVFile(Project::Tools::WideCharToAnsi(strCSVPath).c_str());
							vector<string> vec;
							vec.push_back("3");
							vec.push_back(Project::Tools::WideCharToAnsi(strCSVPath).c_str());
							m_vecDTUCmd.push_back(vec);
							m_bReceiveDTUServerEvent = true;
						}
					}
					CloseZip(hz);
				}
				m_strRecFileName = "";
			}
		} 
		else if(buffer[0] == '2')
		{
			int nPackageIndex = BufferToInt(buffer+2);
			int nLength =   BufferToInt(buffer+7);
			if(nPackageIndex>=0 && nPackageIndex<=1000 && nLength <= g_nPackageSize)
			{
				memset(m_cFileBuffer[nPackageIndex],0,sizeof(m_cFileBuffer[nPackageIndex]));
				memcpy(m_cFileBuffer[nPackageIndex],buffer+12,nLength);
				m_nRecFileCount++;
				CString str;
				str.Format(_T("INFO : Receive Lost File Package:%d.\r\n"),nPackageIndex);
				PrintLog(str.GetString(),false);
			}
			else
			{
				int a = 0;
			}
		}
	}
}

unsigned int CDTUSender::BufferToInt( const unsigned char* buffer )
{
	return buffer[0] + buffer[1]*256 + buffer[2]*65536 + buffer[3]*256*65536;
}

bool CDTUSender::ScanAndSendLostPackage()
{
	map<int,int>::iterator itert = m_mapFileUpdateState.begin();
	while(itert != m_mapFileUpdateState.end())
	{
		int nState = itert->second;
		if(nState != 1)
			return true;
		++itert;
	}
	return false;
}

bool CDTUSender::ScanAndSendLostPackage( string& strUpdateExeAck,string& strLostFile )
{
	if(m_bHasLostFileFlag)
	{
		std::ostringstream sqlstream;
		vector<int> vecLost;
		map<int,int>::iterator itert = m_mapFileUpdateState.begin();
		while(itert != m_mapFileUpdateState.end())
		{
			int nState = itert->second;
			if(nState != 1)
				sqlstream << nState << ",";
			++itert;
		}

		strLostFile = sqlstream.str();
		if(strLostFile.length()>0)
			strLostFile.erase(--strLostFile.end());

		strUpdateExeAck = "";
		m_bHasLostFileFlag = false;
		return true;
	}
	else
	{
		strUpdateExeAck = m_strUpdateExeAck;
		m_strUpdateExeAck = "";
		if(strUpdateExeAck.length() > 0)
			return true;
	}
	return false;
}

bool CDTUSender::SendUpdateFileAck( int nState, string strContetn )
{
	bool bResult = true;
	std::ostringstream sqlstream;
	sqlstream << "fdp:4|" << nState << "|" << strContetn << ";";
	string strdata = sqlstream.str();
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	return bResult;
}

bool CDTUSender::GetSendIdle( int nSeconds/*=4*/ )
{
	if(m_bDTUMode)
	{
		return m_porthandler->GetDTUIdleState(nSeconds);
	}
	else
	{
		return m_tcphandler->GetDTUIdleState(nSeconds);
	}
}

bool CDTUSender::SendHistoryDataFile( string strFilePath ,int nPointCount)
{
	bool bResult = true;
	std::ostringstream sqlstream,sqlstream_index;
	if(strFilePath == "")		//查找失败
	{
		sqlstream << "fhp:3;";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	}
	else
	{
		//读文件
		gzFile gz = gzopen(strFilePath.c_str(),"rb");
		if(!gz)
			return false;

		char **cFile = new char *[FILE_SIZE];
		for(int i=0;i<FILE_SIZE;++i)
		{
			cFile[i] = new char[ONE_FILE_SIZE];
		}

		int nLastLen = 0;
		int nCount = 0;
		//开始传送文件
		for(;;)
		{
			//一次读取BLOCKSIZE大小的文件内容
			nLastLen = gzread(gz,cFile[nCount], m_nTCPSendFilePackageSize);
			cFile[nCount][nLastLen] = ';';			//加一个分号
			nCount++;
			if(nLastLen < m_nTCPSendFilePackageSize)
				break;		
		}
		//关闭文件
		gzclose(gz);

		int pos = strFilePath.find_last_of('\\');
		string strFileName(strFilePath.substr(pos + 1));

		//开始
		sqlstream.str("");
		sqlstream << "fhp:0|" << strFileName << ";" << nPointCount << ";";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
		Sleep(m_nTCPSendFilePackageInterval);

		string strSend = "fhp:2|";
		for(int i=0;i<nCount; ++i)
		{
			char cSend[ONE_FILE_SIZE] = {0};
			memcpy(cSend,strSend.data(),6);
			unsigned int length = 0;
			if(i==nCount-1)
			{
				length = nLastLen+1;
			}
			else
			{
				length = m_nTCPSendFilePackageSize+1;
			}
			memcpy(cSend+11,&length,4);
			if(m_nDTUFilePackageNumber==59)			//因为长度位置会变成; 影响解析
				m_nDTUFilePackageNumber++;
			memcpy(cSend+6,&m_nDTUFilePackageNumber,4);
			sqlstream_index << m_nDTUFilePackageNumber << ",";
			m_nDTUFilePackageNumber++;
			if(m_nDTUFilePackageNumber>=1000)
			{
				m_nDTUFilePackageNumber = 0;
			}

			cSend[10] = '|';
			cSend[15] = '|';
			memcpy(cSend+16,cFile[i],length);
			bResult = SendDataByWrap(cSend, length+16);
			Sleep(m_nTCPSendFilePackageInterval);
		}

		//释放
		for(int i=0;i<FILE_SIZE;++i)  
		{  
			delete cFile[i];  
			cFile[i] = NULL;  
		}  
		delete [ONE_FILE_SIZE]cFile;  
		cFile = NULL;     
					
		//结束
		char cSend1[256] = {0};
		strSend = "fhp:1|";
		memcpy(cSend1,strSend.data(),6);
		memcpy(cSend1+11,&nLastLen,4);
		memcpy(cSend1+6,&nCount,4);

		string strFileIndex = sqlstream_index.str();
		strFileIndex.erase(--strFileIndex.end());
		strFileIndex += ";";
		memcpy(cSend1+16,strFileIndex.data(),strFileIndex.size());
		cSend1[10] = '|';
		cSend1[15] = ';';

		bResult = SendDataByWrap(cSend1,16+strFileIndex.size());
		Sleep(m_nTCPSendFilePackageInterval);
	}

	return bResult;
}

bool CDTUSender::RestartDTU()
{
	//保存原设置
	bool bSaveReceiveDTU = m_bReceiveDTU;
	bool bSaveDTUConnectOK = m_bDTUConnectOK;

	m_bDTU_To_RTU = true;
	m_bReceiveDTU = false;
	m_bDTUConnectOK = false;

	//进入AT模式
	SendRDP_AT();
	bool bresult = false;
	for(int i=0; i<60; ++i)
	{
		if(m_bReceiveDTU)   
		{
			bresult = true;
		}

		if(m_bDTUConnectOK)
		{
			break;
		}

		Sleep(1000);
	}

	//退出AT模式
	SendRDP_Reset();

	bresult = m_bDTUConnectOK;

	//还原
	//m_bDTU_To_RTU = false;
	m_bDTUConnectOK = bSaveDTUConnectOK;
	m_bReceiveDTU = bSaveReceiveDTU;

	return bresult;
}

void CDTUSender::SetDTUToRTU( bool bDTU_To_RTU )
{
	m_bDTU_To_RTU = bDTU_To_RTU;
}

bool CDTUSender::GetDTUSuccess()
{
	if(!m_bDTUMode)
	{
		m_bdtuSuccess = m_tcphandler->GetTCPConnectOK();
	}
	return m_bdtuSuccess;
}

bool CDTUSender::SendUserOperationData( const SYSTEMTIME tSendTime, const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string pointdataprefix = "orp:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = pointdataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return bResult;
}

void CDTUSender::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}
