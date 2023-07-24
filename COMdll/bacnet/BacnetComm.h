//BacnetComm.h
#pragma once

#include "tools/CustomTools/CustomTools.h"
#include "../bacnet/include/bacdef.h"
#include "../bacnet/include/apdu.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include <hash_map>
#include "DB_BasicIO/RealTimeDataAccess.h"
//#include "vld.h"
//struct BACNET_ADDRESS;
//struct BACNET_CONFIRMED_SERVICE_ACK_DATA;
struct BACnet_Read_Property_Data;
typedef struct BACnet_Read_Property_Data BACNET_READ_PROPERTY_DATA;

struct BACnet_Read_Access_Data;
typedef struct BACnet_Read_Access_Data BACNET_READ_ACCESS_DATA;

struct BACnet_Application_Data_Value;
typedef struct BACnet_Application_Data_Value BACNET_APPLICATION_DATA_VALUE;

const UINT C_BANECT_PROPERTY = 85;

#define LOG_ERROR 0
#define LOG_WARNING 1
#define LOG_INFO 2

void PrintAddressCache();

class CBacnetCtrl
{
public: 
	CBacnetCtrl(int nReadInterval, int nReadTypeInterval, int nReadCmdInterval,int nReadMode,int nReadTimeOut,int nPrecision=6, int nReadLimit = 20, int nWritePriority=0);             	 						          // default consructor
	virtual ~CBacnetCtrl();    	 						          // default destructor

public:

	void	SetPointList(const vector<DataPointEntry>& entrylist);
	void	SortPointList();
	static void	SetDebug(int nDebug);
	bool	IsMacInitOK();		//�����Ƿ�׼����
    virtual bool	Connect( );
    void	Disconnect(); 
	void	GetIPInfo(hash_map<wstring,wstring>& mapIP);
	bool	GetProcNameByPort(int nPort, string &strResult);
	
	// write
    bool SendWriteCmdBacnet(DWORD deviceObjInstance,
							WORD objType,
							DWORD objInstance,
							WORD objProperty,
							UINT8 tagType,
							double dValue,
							DWORD pointIndex,
							wstring pointName);

	vector<vector<DataPointEntry>> SortPointByDeviceID(vector<DWORD> vecDeviceID,vector<DataPointEntry> pointlist);

public:
    static void    InitServiceHandlers();
    void            InitServiceHandlersForWhois();
	
    bool            CreateThreadRead();
	
	static  DataPointEntry*    FindDataFromDeviceIDTypeAddress(DWORD deviceid,UINT type, DWORD address);
	static  DataPointEntry*    FindDataFromDeviceTypeName(DWORD index,wstring name,UINT type);
	static  DataPointEntry*    FindDataFromDeviceIDTypeInvokedID(UINT invokeId,UINT type, DWORD address);
	
    static UINT WINAPI      ThreadRead(LPVOID pParam);
	
    void            OnTimerRead();
    void            OnTimerWhois();
	void            OnTimerWhois_();

    static void            OnReceiveRead(BACNET_READ_PROPERTY_DATA* pData, BACNET_ADDRESS * src,UINT invokeId);
    static bool            OnReceiveReadMultiple(BACNET_READ_ACCESS_DATA* pRpmData, BACNET_ADDRESS * src,UINT invokeId);

    static void            CopyValue(BACNET_APPLICATION_DATA_VALUE* pDataValue, DataPointEntry* bacData );
    static void            FillWriteBuffer(BACNET_APPLICATION_DATA_VALUE* pDataValue, double dValue, UINT tag);

	
    static void		MyAbortHandler(BACNET_ADDRESS * src, uint8_t invoke_id,
										  uint8_t abort_reason, bool server);
	static void		MyIAMHandler(uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src);
	static void     MyIAMAddHandler(uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src);

    static void     MyErrorHandler(BACNET_ADDRESS * src, uint8_t invoke_id,
										  BACNET_ERROR_CLASS error_class, BACNET_ERROR_CODE error_code);
    static void     MyRejectHandler(BACNET_ADDRESS * src, uint8_t invoke_id, uint8_t reject_reason);
    static void     HandleReadPropertyAck(UINT8 * service_request,
										  UINT16 service_len,
										  BACNET_ADDRESS * src,
										  BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);
    static void            HandleReadPropertyMultiAck(UINT8 * service_request,
													  UINT16 service_len,
													  BACNET_ADDRESS * src,
													  BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);
    static void            HandleWritePropertyAck(BACNET_ADDRESS * src, uint8_t invoke_id);

		
	static void        ClearReadAccessData(BACNET_READ_ACCESS_DATA** pRpmData);

	//send thread
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);

	static DWORD GetBacnetDeviceID(const wstring strBacnetParam1);


	bool	GetSendCommandThreadExit() const;
	bool	GetReadDataThreadExit() const;

	void	SendReadCommands();

	static void	GetReceiveBacnetID(hash_map<UINT,wstring>& mapBacnetDevice);

	bool	SendReadCommand(DataPointEntry& entry);

	bool	SendCommandByGroup(int groupnum , vector<DataPointEntry> &entryList, int& nInvolkStartFromNum);

	void	GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist );

	void	SetValue(const wstring& pointname, double value);
	void	InitDataEntryValue(const wstring& pointname, double fvalue); //������ͨ��ָ��, add by golding

	bool	GetValue(const wstring& pointname, double& result);

	void	Exit();

	static void	AddLog(const wchar_t* loginfo);
	static	bool	FreeAllInvokeId();

	static	void	OutDebugInfo(const wstring strOut, int nLogLevel);

	//////////////////////////////////////////////////////////////////////////
	static UINT WINAPI ThreadActiveCommandsFunc(LPVOID lparam);
	HANDLE	GetSendCmdEvent();			//��ȡ�����¼�
	static void	SetSendCmdEvent();			//���ü����¼�
	void	ActiveSendCmdEvent();			//���ü����¼�
	void	SendReadCommandsByActive();
	bool	SendCommandByGroupByActive(vector<DataPointEntry> &entryList, bool& bDeviceNotBind);		//�����
	vector<vector<DataPointEntry>> SortPointByDeviceID(vector<DWORD> vecDeviceID,vector<DataPointEntry> pointlist,int nGroupMax);

	static  DataPointEntry*    FindDataByDeviceIDTypeInvokedID(UINT invokeId,UINT type, DWORD address);
	void	SumReadAndResponse();						//ÿ�η���ǰͳ����һ�ε��շ����
	//////////////////////////////////////////////////////////////////////////
	static void 	PrintLog(const wstring &strLog, int nLevel);
protected:
    
	HANDLE              m_readdata_thread;		//�����߳�
	bool                m_exit_readdata_thread;

	HANDLE              m_active_thread;		//�����߳�
	
	HANDLE				m_hsendcommand_thread;			//�����߳�
	bool                m_exit_sendcommand_thead;
	   
	//����б�
	static  std::vector<DataPointEntry>	m_vecpointlist;  

	static  std::vector<DataPointEntry>  m_pointlistAI;
	static  std::vector<DataPointEntry>  m_pointlistAO;
	static  std::vector<DataPointEntry>  m_pointlistBI;
	static  std::vector<DataPointEntry>  m_pointlistBO;
	static  std::vector<DataPointEntry>  m_pointlistAV;
	static  std::vector<DataPointEntry>  m_pointlistBV;
	static  std::vector<DataPointEntry>  m_pointlistMI;
	static  std::vector<DataPointEntry>  m_pointlistMO;
	static  std::vector<DataPointEntry>  m_pointlistMV;

	//���豸������
	static  std::vector<vector<DataPointEntry>>  m_vecAI;
	static  std::vector<vector<DataPointEntry>>  m_vecAO;
	static  std::vector<vector<DataPointEntry>>  m_vecBI;
	static  std::vector<vector<DataPointEntry>>  m_vecBO;
	static  std::vector<vector<DataPointEntry>>  m_vecAV;
	static  std::vector<vector<DataPointEntry>>  m_vecBV;
	static  std::vector<vector<DataPointEntry>>  m_vecMI;
	static  std::vector<vector<DataPointEntry>>  m_vecMO;
	static  std::vector<vector<DataPointEntry>>  m_vecMV;

	static	int			m_nReqInvokeIdErrCount;
	static	int			m_nReadMode;		//Ĭ��0Ϊ��index  1Ϊ��ip  2ΪIP/ID
	static	int			m_nCmdCount;		//�������
	static	int			m_nResponseCount;	//����ظ�����
	static	int			m_nUpdatePointCount;//���µ�
	static	int			m_nLastInvolk;
	static	int			m_nPointCount;		//�������
	static	int			m_nOutPut;			//1�������Ϣ
	static	int			m_nBacnetBindOK;	//0:bacnet bind fail
	static	int			m_nBacnetSendOK;	//0:send fail
	static	int			m_nBacnetResponseOK;//0:rec fail
	static	int			m_nBacnetRecOK;
	Project::Tools::Mutex	m_lock;

	static hash_map<wstring,DWORD>	m_mapDeviceID;		//IP/ID��Ϊkey
	static hash_map<wstring,DataPointEntry*> m_mapPointInvolk;		//�Ѹ�ֵInvol�ĵ�λָ��
	static hash_map<wstring,int>			m_mapDeviceResponse;		//�豸��Ӧ���
	//static hash_map<wstring,DataPointEntry*> m_mapQueryPointInvolk;		//�Ѹ�ֵInvol�Ĳ��ҵ�λָ��
	static hash_map<UINT8,hash_map<wstring,DataPointEntry*>> m_mapQueryPointInvolk;		//�Ѹ�ֵInvol�Ĳ��ҵ�λָ��  ���洢10��Involk�Ĳ�ѯ����
	static HANDLE		m_sendCmdEvent;

private:
	static Beopdatalink::CLogDBAccess* m_logsession;
	static CRITICAL_SECTION m_criticalSection;
public:
	static void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	static SYSTEMTIME	m_sExecuteTime;
	static string		m_strExecuteLog;
	//����log
	static void	SaveExecuteOrder(string nOrder);
	string	GetExecuteOrderLog();
	int		m_nReadInterval;
	int		m_nReadTypeInterval;
	int		m_nReadCmdInterval;
	static COleDateTime	m_oleUpdateTime;
	static COleDateTime	m_oleStartTime;
	static COleDateTime m_oleSendWhoISTime;			//���豸��δ�ҵ�ʱ��ÿСʱ���㲥1��
	static	int			m_nSendWhiIsCount;			//�㲥����
	string			m_strErrInfo;
	static	string m_strUpdateLog;
	int						m_nPrecision;				//���ݾ��� Ĭ��6
	static  int				m_nReadLimit;				//������ȡ����
	vector<DWORD>		m_vecDeviceID;
	//////////////////////////////////////////////////////////////////////////
	int					m_nReadTimeOut;							//��ʱʱ��
	bool				m_bNeedActive;							//��Ҫ�����¼�
	static COleDateTime	m_oleSendCmdTime;						//��������ʱ��
	static	wstring		m_wstrDeviceID;							//�豸����Ϣ
	//////////////////////////////////////////////////////////////////////////

	static int m_nLogLevel; //0:ERROR, 1:WARNING,2:INFO

	int m_nWritePriority;
};
