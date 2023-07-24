#pragma once

#include "windef.h"
#include "Equipment.h"

class COPCEngine;
class CModbusEngine;
class CMemoryLink;
class CBacnetEngine;
class CDataPointManager;
class CProtocol104Engine;
class CMysqlEngine;
class CSqlServerEngine;
//class CDTUSender;
class CBacnetServerEngine;
class CSiemensLink;
class CFCBusEngine;
class CSqliteEngine;
class CCO3PEngine;
class CWebEngine;
class CDiagnoseLink;

#include "DB_BasicIO/RealTimeDataAccess.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "../DTUDataUpload/DTUSender.h"
#include "../COMdll/ModbusTcp/TCPDataSender.h"
#include "SiemensLink.h"
#include "../COMdll/ProcessView.h"
#include "../COMdll/ModbusTcp/DogTcpCtrl.h"
#include "RedisManager.h"

#include <map>
using std::map;
using namespace Beopdatalink;

#include "DataEngineMode.h"

#define DATA_BUFFER_SIZE 2048		//�����С

struct MysqlServer
{
	wstring strIP;
	wstring strSchema;
};

struct SqlServer
{
	wstring strIP;
	wstring strSchema;
};

struct SqliteServer
{
	wstring strFilePath;
	wstring strFileTable;
};

struct FCServer
{
	wstring strIP;
	wstring strPort;
};

struct OPCServer
{
	wstring strIP;
	wstring strServerName;
	int		nCount;		
};

struct WebServer
{
	wstring strIP;
	wstring strUser;
	wstring strPsw;
};

inline bool new_time(wstring const& lhs, wstring const& rhs)
{
	return lhs < rhs; //��С��������
}

class CBEOPDataLinkManager
{
public:
	CBEOPDataLinkManager();
	CBEOPDataLinkManager(CDataPointManager* pointmanager, Beopdatalink::CRealTimeDataAccess* pdbcon, bool bRealSite, bool bDTUEnabled,bool bDTUChecked,bool bDTURecCmd,int nDTUSendMinType =3,int nDTUPort = 1,bool bModbusReadOne=false,int nModbusInterval=100,bool bDTUDisableSendAllData=false,bool bTCPEnable=false,wstring strTCPName=L"",wstring strTCPServerIP=L"114.215.172.232",int nTCPServerPort=9500,int nSendAllData=1,bool bRunDTUEngine=false);
	~CBEOPDataLinkManager(void);

	virtual bool Init();
	virtual bool Exit();
	virtual bool InitOPC();
	virtual bool InitDTUEngine(bool bDTUEnabled, bool bTCPEnable);
	virtual bool InitBacnetEngine(vector<DataPointEntry>& bacnetpoint);
	
	static	unsigned int WINAPI	ThreadFuncHistoryData(LPVOID lparam);
	static	unsigned int WINAPI	ThreadFuncHistoryData_S5(LPVOID lparam);
	static  unsigned int WINAPI SendDTUThread(LPVOID lpVoid);
	static	unsigned int WINAPI ScanWarningAndOperation(LPVOID lpVoid);
	static  unsigned int WINAPI SendDTUReceiveHandleThread(LPVOID lpVoid);
	static	unsigned int WINAPI ThreadProjectDB(LPVOID lpVoid);		//��ʱɨ����̿ռ䣬�Զ�������ݿ��

	virtual bool    InitInputTable();
	virtual bool	UpdateInputTable();
	virtual bool	UpdateOutputParams(wstring &strChangedValues);

	virtual void	GetOPCValueSets(vector<pair<wstring, wstring> >& opcvaluesets);
	virtual void    GetSiemensTCPValueSets(vector<pair<wstring, wstring> >& vSets);
	virtual	void	GetModbusValueSets(vector<pair<wstring, wstring> >& modbusvaluesets);
	virtual	void	GetCO3PValueSets(vector<pair<wstring, wstring> >& co3pvaluesets);
	virtual	void	GetProtocol104ValueSets(vector<pair<wstring, wstring> >& protocol104valuesets);
	virtual	void	GetMysqlValueSets(vector<pair<wstring, wstring> >& mysqlvaluesets);
	virtual	void	GetSqlServerValueSets(vector<pair<wstring, wstring> >& sqlvaluesets);
	virtual	void	GetSqliteValueSets(vector<pair<wstring, wstring> >& sqlitevaluesets);
	virtual	void	GetBacnetValueSets(vector<pair<wstring, wstring> >& bacnetvaluesets);
	virtual	void	GetFCbusValueSets(vector<pair<wstring, wstring> >& fcbusvaluesets);
	virtual	void	GetWebValueSets(vector<pair<wstring, wstring> >& fcbusvaluesets);
	virtual	void	GetModbusEquipmentValueSets(vector<pair<wstring, wstring> >& modbusvaluesets);
	virtual	void	GetCoreStationValueSets(vector<pair<wstring, wstring> >& coreStationvaluesets);
	virtual	void	GetKNXValueSets(vector<pair<wstring, wstring> >& KNXvaluesets);
	virtual	void	GetDLT645ValueSets(vector<pair<wstring, wstring> >& DLT645valuesets);
	virtual	void	GetPersagyControllerValueSets(vector<pair<wstring, wstring> >& valuesets);
	virtual	void	GetObixValueSets(vector<pair<wstring, wstring> >& valuesets);
	virtual	void	GetLogixValueSets(vector<pair<wstring, wstring> >& valuesets);
	virtual	void	GetMoxaTCPServerValueSets(vector<pair<wstring, wstring> >& valuesets);
	virtual	void	GetThirdPartyValueSets(vector<pair<wstring, wstring> >& valuesets);
	virtual	void	GetOPCUAValueSets(vector<pair<wstring, wstring> >& modbusvaluesets);
	virtual	void	GetBacnetPyValueSets(vector<pair<wstring, wstring> >& modbusvaluesets);
	virtual	void	GetABSLCValueSets(vector<pair<wstring, wstring> >& valuesets);
	
	virtual	bool	WriteOPCValue(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteModbusValue(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteCO3PValue(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteMemoryPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteBacnetPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool    WriteSiemensTCPPoint(wstring strIP, const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteFCbusValue(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteMysqlPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteSqlServerPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteSqlitePoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteWebPoint(const Beopdatalink::CRealTimeDataEntry& entry);

	virtual bool	WriteWirelessPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool    WriteModbusEquipmentPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool    WritePersagyControllerPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool    WriteKNXPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool    WriteDLT645Point( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool    WriteOPCUAPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool    WriteBacnetPyPoint( const Beopdatalink::CRealTimeDataEntry& entry );


	virtual bool  WriteThirdPartyPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool  WriteObixPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool  WriteLogixPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool  WriteABSLCPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool  WriteMoxaTCPServerPoint( const Beopdatalink::CRealTimeDataEntry& entry );


	bool	UpdateHistoryRedisTableRecentMinutes(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	bool	UpdateHistoryRedisTable(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	bool	UpdateHistoryTable(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nCount);
	bool	GenerateDTURealData(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle,int nSendAll);
	bool    CheckCreateHistoryTable(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle);
	bool    UpdatePointBuffer();
	bool	GenerateAndZipDataBack(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,wstring& strFileName,wstring& strFilePath);
	bool	GenerateDataBack(const SYSTEMTIME &st, wstring& strFileName, wstring& strFileFolder, vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	bool	InsertRedundancyInput();

	bool	GenerateDTURealDataS5(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle,int nSendAll);
	bool	GenerateAndZipDataBackS5(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,wstring& strFileName,wstring& strFilePath);
	bool	GenerateDataBackS5(const SYSTEMTIME &st, wstring& strFileName, wstring& strFileFolder, vector<Beopdatalink::CRealTimeDataEntry> &entrylist);

	void	GenerateEntryList(vector<Beopdatalink::CRealTimeDataEntry>& resultlist);

	bool    IsEquipmentEnabled(Equipment &equip);

	void	SetPointManager(CDataPointManager* pointmanager);
	void	SetDbCon(Beopdatalink::CRealTimeDataAccess* pdbcon);
	void	SetHistoryDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
	void	SetRedundancyDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
	void	SetLogDbCon(Beopdatalink::CLogDBAccess* plogdb);

	void	ChangeValue(wstring &strChangedValues);

	void	SetMsgWnd(CWnd* msgwnd);

	void	SendLogMsg(LPCTSTR logFrom, LPCTSTR loginfo);

	void	CleatOutputData();

	void	ExitTAllThread();

	bool	GetExitThread_Input() const;
	bool	GetExitThread_Output() const;
	bool	GetExitThread_History() const;

	void	AddLog(const wstring& loginfo);
	wstring AutoSupportSimense1200Address(wstring strAddress);

	bool   UpdateRedis(COleDateTime tUpdate, vector<CRealTimeDataEntry > &entryList);

	//��������opc������3���ӣ�ֱ�������ɹ�Ϊֹ��
	bool	InitOPC(COPCEngine* opcengine,int nOPCClientIndex = -1);
	vector<vector<DataPointEntry>> GetOPCServer(vector<DataPointEntry> opcpoint,int nMaxSize,wstring strDefaultIP);	
	int	IsExistInVec(wstring strSer, wstring strProgram,vector<OPCServer>& vecSer,int nMaxSize);

	Beopdatalink::CLogDBAccess*	GetLogAccess();
	COPCEngine*	GetOPCEngine();
	CBacnetEngine *GetBacnetEngine();
	CMemoryLink *GetMemoryEngine();
	CDiagnoseLink*	GetDiagnoseEngine();
	CBacnetServerEngine *GetBacnetServerEngine();

	int GetModbusEngineCount();
	int GetCO3PEngineCount();
	int GetProtocol104EngineCount();

	//ȡ��һ�����ֵ��
	//double	GetValue(const wstring& pointname);
	bool FindS7CPUInfo(const vector<_SiemensPLCProp> & ppList, const _SiemensPLCProp &pp);
	void	InitPointsValueFromDB();

	bool	IsMasterMode();
	void	SetEngineMode(int modevalue);
	BeopDataEngineMode::EngineMode GetEngineMode() ;
	virtual void	UpdateMainServerMode();		//��������.

	bool	InitModbusEngine(vector<DataPointEntry> modbuspoint, int nReadCmdMs = 50,int nMutilReadCount=99,int nIDInterval=500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false,bool bSaveErrLog = false);
	vector<vector<DataPointEntry>> GetModbusServer(vector<DataPointEntry> modbuspoint);	
	int	IsExistInVec(wstring strSer, vector<wstring> vecSer);

	bool	InitCO3PEngine(vector<DataPointEntry> co3ppoint, int nReadCmdMs = 50,int nRollInterval=2,int nTimeOut=5000,int nPrecision=6);
	vector<vector<DataPointEntry>> GetCO3PServer(vector<DataPointEntry> co3ppoint);	
	
	bool	InitFCbusEngine(vector<DataPointEntry> fcbuspoint, int nReadCmdMs = 50,int nPrecision=6);
	vector<vector<DataPointEntry>> GetFCbusServer(vector<DataPointEntry> fcbuspoint);	
	int	IsExistInVec(wstring strSer, wstring strPort,vector<FCServer> vecSer);

	int IsExistInVec(wstring strIP,wstring strSechema, vector<MysqlServer> vecSer);
	int IsExistInVec(wstring strIP,wstring strSechema, vector<SqlServer> vecSer);
	int IsExistInVec(wstring strPath,wstring strTName, vector<SqliteServer> vecSer);
	int IsExistInVec(wstring strIP,wstring strUser, wstring strPsw,vector<WebServer> vecSer);

	bool	InitProtocol104Engine(vector<DataPointEntry> protocol04point);
	vector<vector<DataPointEntry>> GetProtocol104Server(vector<DataPointEntry> protocol04point);	

	bool	InitMysqlEngine(vector<DataPointEntry> mysqlpoint);
	vector<vector<DataPointEntry>> GetMysqlServer(vector<DataPointEntry> mysqlpoint);

	bool	InitSqlServerEngine(vector<DataPointEntry> sqlpoint);
	vector<vector<DataPointEntry>> GetSqlServer(vector<DataPointEntry> sqlpoint);

	bool	InitSqliteEngine(vector<DataPointEntry> sqlitepoint);
	vector<vector<DataPointEntry>> GetSqliteServer(vector<DataPointEntry> sqlitepoint);	

	bool	InitWebEngine(vector<DataPointEntry> webpoint);
	vector<vector<DataPointEntry>> GetWebServer(vector<DataPointEntry> webpoint);

	void EnableLog(BOOL bEnable);
	BOOL IsLogEnabled();

	void GetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	void GetStoreRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	void SetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	void CompareModifiedRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, map<string, wstring> &entrylistMapBuffer, vector<Beopdatalink::CRealTimeDataEntry> &newEntrylist);
	void CombineRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, map<string, wstring> &entrylistMapBuffer, vector<Beopdatalink::CRealTimeDataEntry> &newEntrylist);
	void CompareAndUpdateRealTimeDataEntryList(SYSTEMTIME sNow);	//����仯���� ���±仯ʱ��
	void GetModifiedRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nMinutes);			//�õ�ָ��������ʱ��

	void GetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, const POINT_STORE_CYCLE & tCycle);


	void UpdateRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	void UpdateOutputParamsByList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);

	string BuildDTUSendData(int nPointStoreType, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nCount);
	string BuildDTUSendData_Change(int nPointStoreType, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nChange,int nCount);
	string GenerateDTUUpdateInfo_Change(int nPointStoreType,int nUpdateSize,int nCountSize,Beopdatalink::CRealTimeDataEntry& enrty);		// ͳ��DTU���͵�����
	string GenerateDTUUpdateInfo(int nPointStoreType,int nUpdateSize,int nCountSize);		// ͳ��DTU���͵�����
	string BuildDTUSendUnit01(vector<unitStateInfo> unitlist);

	HANDLE	GetSendDTUEvent();

	bool InsertRedundancyUnit01(string strRedundancyIP);
	string GetHostIP();

	
	bool	SetSendDTUEventAndType(int nDTUDataType, SYSTEMTIME tSendDTUTime);
	bool	SendDTUData();
	bool	ScanWarning();				//ɨ�豨��
	bool	ScanOperation();			//ɨ�账�����
	bool	ScanUserOperation();			//ɨ���û�����

	bool	SendCoreStart(SYSTEMTIME tSendDTUTime);									//Core����ʱ����
	bool	SendAllRealData(SYSTEMTIME tSendDTUTime);			//����ȫ��ʵʱ����
	bool	SendCoreStart();								//DTU ����������־
	bool    UpdateHistoryStatusTable();

	bool	HandleCmdFromDTUServer();							//��������DTUServer������
	bool	ActiveLostSendEvent();								//�д����Ͷ�ʧ�����򼤻�
	bool	UpdateWatch();										//������¿��Ź�
	bool	UpdateDebug();										//��ѯ������Ϣ���
	bool	ChangeValueByDTUServer(string strPointName, string strValue);
	bool	ChangeValuesByDTUServer(string strReceive);
	bool	ChangeUnit01ByDTUServer(string strParamName, string strValue);
	bool	SendHistoryDataFile(string strStart,string strEnd);
	bool	RestartCore();		//����Core
	bool	RestartWatch();		//����Watch
	bool	RestartLogic();		//����Logic
	bool	SynUnit01();		//ͬ��Unit01
	bool	SynRealData(POINT_STORE_CYCLE nType);	//ͬ��ĳ����������
	bool	SendRealData(POINT_STORE_CYCLE nType);	//ͬ��ĳ����������
	bool	ExecuteDTUCmd(string strReceive);			//ִ��Sql���
	bool	AckHeartBeat();						//��Ӧ������������
	bool	AckSynSystemTime(string strSystemTime);						//��Ӧ������ͬ��ϵͳʱ��
	bool	AckCoreVersion();									//��ӦCore�汾��
	bool	AckReSendData(string strIndex);					//��Ӧ��ʧ�ط�����
	bool	AckReSendLostData(string strMinutes,string strPacketIndex);			//��Ӧ��ʧ�����ڼ�����
	bool	AckReSendLostZipData(string strMinutes,string strFileNames);			//��Ӧ��ʧ�����ڼ�����
	bool	AckUpdateExeFile(string strExeName);								//��Ӧ�����ļ��ɹ���־
	bool	AckLostExeFile(string strLostFile);								//��Ӧ�����ļ��ɹ���־
	bool	AckReSendFileData(string strFileName);					//��Ӧ��ʧ�ط�����
	bool	AckReStartDTU();									//��Ӧ����DTU
	void	ReadCSVFileAndSendHistory(string strPath);
	bool	ReadDataAndGenerateHistoryFile(string strFileName,string strFolder,string strPath);		//�����ļ�������ʱ�򣬶�ȡ���ݿ��������ɵ�ַ
	bool	ScanSendHistoryQueue(int nSecond = 4);
	bool	SendOneLostHistoryData();							//����һ�������͵���ʷ��ʧ����

	bool	StoreFileInfo(wstring strFolder,wstring strFileName,int nPointCount);
	int		GetFileInfoPointCont(string strPath,string strFileName);

	wstring ConvertPointType(POINT_STORE_CYCLE nType);

	bool	SendExecuteOrderLog();    //����Log���洢����־�ļ�
	bool	OutPutLogString(SYSTEMTIME st,string strOut);
	//DTU��Ԫ����
	bool	SendDTUTestPoint(string strPointName, wstring strValue);
	CDTUSender*	GetDTUSender();

	bool	UpdateDTUSuccess();

	void	SplitString(const std::string& source, const char* sep, std::vector<string>& resultlist);

	bool	CheckDiskFreeSpace(string strPath);	

	bool	CloseWatchDog();		 //�رյ�ǰ����Watch
	bool	OutPutUpdateLog(wstring strLog);		//��¼����Log
	bool	UpdatePrivilege();						//����Ȩ��
	bool	ImproveProcPriv();
	bool	RunBatCmd_SetSystemTime(SYSTEMTIME st);
	bool	SaveBacnetInfo();

	//////////////////////////////////////////////////////////////////////////
	bool	RestartByMaster(E_SLAVE_MODE nRestartExe, E_SLAVE_REASON nReason,wstring strPath);
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
public:
	int		m_nDTUDataType;		//0 ʵʱ���ݣ� 1 ����   2 �������� 3: Unit01 4: ExecuteLog
	bool m_bRealSite;
	bool	m_bFirstStart;
	int m_nUpdateInputCount;
	int m_nUpdateOutputCount;

	map<wstring, Equipment> m_mapEquipment;

	CRedisManager m_Redis;
	CRedisManager m_Redis_To_DeepLogic;

private:
	//data engine will hold this two result.
	COPCEngine* m_opcengine;
	CMemoryLink*  m_memoryengine;
	CDiagnoseLink* m_pDiagnoseEngine;
	CBacnetEngine* m_bacnetengine;
	CBacnetServerEngine * m_bacnetServerEngine;
	vector<wstring>	m_vecModbusSer;
	vector<wstring>	m_vecCO3PSer;
	vector<wstring>	m_vecProtocol104Ser;
	vector<MysqlServer>	m_vecMysqlSer;
	vector<WebServer>	m_vecWebSer;
	vector<SqlServer>	m_vecSqlSer;
	vector<SqliteServer>	m_vecSqliteSer;
	vector<FCServer>	m_vecFCSer;
	vector<OPCServer>	m_vecOPCSer;
	map<wstring,CModbusEngine*> m_mapMdobusEngine;
	map<wstring,CCO3PEngine*> m_mapCO3PEngine;
	map<wstring,CFCBusEngine*> m_mapFCBusEngine;
	map<wstring,CProtocol104Engine*> m_mapProtocol104Engine;
	map<wstring,CMysqlEngine*> m_mapMysqlEngine;		//IP+���ݿ���ΪKey
	map<wstring,CSqlServerEngine*> m_mapSqlServerEngine;	//IP+���ݿ���ΪKey
	map<wstring,CSqliteEngine*>	m_mapSqliteEngine;		//��ַ+������Ϊkey
	map<int,COPCEngine*>	m_mapOPCEngine;				//index���OPC���棨OPC�����10000ʱ�����ã�
	map<wstring,CWebEngine*> m_mapWebEngine;			//IP+�û���+���� ��ΪKey
//	CSiemensLink * m_pSiemensEngine;

	bool m_bModbusEquipmentEngineEnable;
	bool m_bThirdPartyEngineEnable;
	bool m_bPersagyControllerEngineEnable;
	bool m_bOPCUAEngineEnable;
	bool m_bBacnetPyEngineEnable;
	bool m_bObixEngineEnable;
	bool m_bLogixEngineEnable;
	bool m_bABSLCEngineEnable;
	bool m_bMoxaTCPServerEngineEnable;
	bool m_bCoreStationEngineEnable;
	bool m_bKNXEngineEnable;
	bool m_bDLT645EngineEnable;


	HANDLE	m_hupdateinput;
	bool	m_bexitthread_input;
	HANDLE	m_hupdateoutput;
	bool	m_bexitthread_output;
	HANDLE	m_hhistorydata;
	HANDLE	m_hhistoryS5data;
	bool	m_bexitthread_history;
	
	// data engine will not not hold the object.
	Beopdatalink::CRealTimeDataAccess*	m_dbsession_realtime;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
	Beopdatalink::CCommonDBAccess*	m_dbsession_Redundancy;
	Beopdatalink::CLogDBAccess*			m_dbsession_log;
	CDataPointManager*	m_pointmanager;

	vector<CRealTimeDataEntry>	m_vecOutputEntry;

	Project::Tools::Mutex m_lock_vecMemoryPointChangeList;
	Project::Tools::Mutex m_toollock;			//������
	vector<CRealTimeDataEntry>	m_vecMemoryPointChangeList;

	CWnd*	m_msgwnd;

	BeopDataEngineMode::EngineMode	m_enginemode;
	Project::Tools::Mutex m_enginemode_lock;

	CRITICAL_SECTION m_RealTimeData_CriticalSection;

	vector<Beopdatalink::CRealTimeDataEntry> m_RealTimeDataEntryList;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_S5;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_M1;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_M5;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_H1;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_D1;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_Month1;
	POINT_STORE_CYCLE m_nMinTCycle;

	map<string,CRealTimeDataEntry>	m_RealTimeDataMap;
	map<string,int>					m_mapSendLostHistory;

	BOOL m_bLog;

	COleDateTime m_tLastHistorySaved;
	COleDateTime m_oleLastRemoteSetting;		//����յ�Զ�̸��µ�ʱ�� 2�����ڼӿ췢��ʱ��
	COleDateTime	m_oleCoreStart;				//Core����ʱ��
	bool			m_bUpdateBacnetInfo;		//����Bacnet��Ϣ������Core�������5������ִ��һ��

	bool m_bDTUEnabled;
	CDTUSender*	m_pdtusender;
	bool		m_bdtuSuccess;
	HANDLE		m_senddtuevent;
	HANDLE		m_hsenddtuhandle;
	HANDLE		m_hReceivedtuhandle;
	HANDLE		m_hscanwarninghandle;				//ɨ�豨��
	string		m_strSendBuf;
	string		m_strSendWarningBuf;				//��������
	string		m_strSendOperationBuf;				//��������
	string		m_strSendUserOperationBuf;			//�û�����
	SYSTEMTIME  m_tSendDTUTime;						//DTU����ʱ��
	Project::Tools::Mutex m_dtu_lock;
	Project::Tools::Mutex m_dtu_ack_lock;

	bool m_bFirstInitRedundancyBuffer;
	int	 m_nDTUPort;
	int	 m_nDTUSendMinType;
	bool m_bDTUChecked;
	bool m_bDTURecCmd;
	bool m_bModbusreadonebyone;
	int  m_nMobusInterval;
	int	 m_nCO3PInterval;								//ms
	int	 m_nCO3PRecevieTimeOut;				//ms
	int	 m_nCO3PRollInterval;				//s
	int	m_nMutilReadCount;		//������������ Ĭ��99
	int		m_nIDInterval;			//��ͬID֮��ļ�� Ĭ��500ms
	int		m_nModbusTimeOut;			//modbus��ʱ
	int		m_nModbusPollInterval;		//modbus��ѯ���
	bool m_bDTUDisableSendAllData;
	vector<DTUSendInfo>	m_vecSendBuffer;		//DTU���ͻ���
	bool		m_bVecCopying;					//����copy�������

	bool	m_bTCPEnable;						//TCP���緢��ģʽ
	wstring m_strTCPServerIP;					//TCP��������ַ
	wstring m_strTCPName;						//���ڱ�ʶTCP��Ŀ
	int		m_nTCPServerPort;					//TCP�������˿�

	string		m_strAck;							//�޸ĵ�ACK�ظ����ݺϲ�����
	string		m_strZipPath;
	string      m_strSendRealFilePath;				//��Ҫ�������͵�ʵʱ����ַ
	int			m_nSendRealFileCount;				//��Ҫ�������͵�ʵʱ������
	SYSTEMTIME m_ackTime;							//�ظ�ʱ��
	int			m_nStoreZip;					//0 ���洢 1:5s 2:1m  3:5m 4:30m 5:1h 6:1d
	int			m_nSendAll;						//0 ������ȫ��  1��ÿ�η���ȫ��
	int			m_nSendPointCount;				//���͸���
	int			m_nOPCClientMaxPoint;			//OPCClient����������
	int			m_nMutilCount;					//OPC������ȡ�����
	int			m_nOPCCmdSleep;					//OPC���������ӻ��棩
	int			m_nOPCCmdSleepFromDevice;		//OPC������(���豸)
	int			m_nOPCPollSleep;				//OPC��ѯ������ӻ��棩
	int			m_nOPCPollSleepFromDevice;		//OPC��ѯ���(���豸)
	int			m_nReconnectMinute;				//������N�����ڲ��洢����
	int			m_nSendRemoteSet;				//Զ�����ù���Ns�ڷ���5������
	bool		m_bMutilOPCClientMode;			//��OPCClientģʽ
	bool		m_bMutilOPCClientUseThread;		//��OPCClientģʽʹ���߳�ģʽ
	bool		m_bStoreHistory;				//�Ƿ�洢��ʷ����
	bool		m_bEnableSecurity;				//�������̰�ȫ������
	bool		m_bDisableCheckQuality;			//��������Ʒ�����ݼ��
	int			m_nOPCMainThreadOPCPollSleep;	//OPC��ѯ���s�������̴ӻ��棩
	bool		m_bAsync20Mode;					//�첽��дģʽ
	int			m_nRefresh20Interval;			//�첽ˢ�¼��
	int			m_nLanguageID;					//����ID
	int			m_nMultiAddItems;				//������ӵ�
	int			m_nUpdateRate;					//����������������ˢ����
	wstring		m_strLastSendOperation;			//����Ͳ�����¼ʱ��

	char			m_cSpiltBuffer[DATA_BUFFER_SIZE];			//���ݽ�������
	//map<string, wstring>		m_RealTimeDataEntryListBufferMap_Month1;

	CProcessView		m_PrsV;

	int			m_nRemoteSetOPC;
	int			m_nRemoteSetModbus;
	int			m_nRemoteSetBacnet;
	int			m_nRemoteSetSiemens;
	int			m_nRemoteSetMysql;
	int			m_nRemoteSetSqlite;
	int			m_nRemoteSetSqlServer;
	int			m_nOPCCheckReconnectInterval;	//OPC������������min��
	int			m_nOPCReconnectInertval;		//OPC N��������δ��������(min)
	int			m_nDebugOPC;
	int			m_nPrecision;				//���ݾ��� Ĭ��6
	int			m_nMaxInsertLimit;			//���ݿ�һ���Բ����������
	bool		m_bDisSingleRead;
	bool		m_bSaveErrLog;			//�Ƿ�洢������
	bool		m_bRunDTUEngine;			//�Ƿ�����DTUEngine����
};
