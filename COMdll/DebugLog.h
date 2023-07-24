#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__
#include "../Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "../Tools/ErrorCodeDefine.h"
#include "../BEOPDataPoint/DataPointEntry.h"

enum E_DEBUG_LOG_TYPE
{
	E_DEBUG_LOG_NULL = 0,		//w��
	E_DEBUG_LOG_ALL,			//ȫ������
	E_DEBUG_LOG_TCP,			//tcp
	E_DEBUG_LOG_MODBUS,			//modbus
	E_DEBUG_LOG_FCBUS,			//fcbus
	E_DEBUG_LOG_MYSQL_ENGINE,	//mysql���ݿ�����
	E_DEBUG_LOG_MASTER104,		//104���
	E_DEBUG_LOG_S7UDP,			//s7udpֱ��
	E_DEBUG_LOG_BACNET,			//bacnet
	E_DEBUG_LOG_OPC,			//opc
	E_DEBUG_LOG_DTU,			//dtu
	E_DEBUG_LOG_SERIES,			//series
	E_DEBUG_LOG_TCPSENDER,		//tcpdatasender
	E_DEBUG_LOG_MYSQL,
	E_DEBUG_LOG_DATALINK,
	E_DEBUG_LOG_LOGICLINK,
	E_DEBUG_LOG_OTHER,
};

enum E_DEBUG_LOG_LEVEL
{
	E_DEBUG_LOG_LEVEL_NULL = 0,
	E_DEBUG_LOG_LEVEL_All,
	E_DEBUG_LOG_LEVEL_BASE,		//����LOG(������������Բ�����)
	E_DEBUG_LOG_LEVEL_ERROR,	//����LOG������ϸ�Ĵ���
	E_DEBUG_LOG_LEVEL_PROCESS,	//����LOG(���е�����ж��Ƿ�ס)
	E_DEBUG_LOG_LEVEL_THREAD,	//�߳�LOG
	E_DEBUG_LOG_LEVEL_4,
	E_DEBUG_LOG_LEVEL_5,
};

enum POINTTYPE
{
	POINTTYPE_OPC_,
	POINTTYPE_MODBUS_,
	POINTTYPE_BACNET_,
	POINTTYPE_VPOINT_,
	POINTTYPE_PROTOCOL104_,
	POINTTYPE_MYSQL_,
	POINTTYPE_SIEMENSE1200TCP_,
	POINTTYPE_SIEMENSE300TCP_,
	POINTTYPE_FCBUS_,
	POINTTYPE_AB500_,
	POINTTYPE_INSIGHT_,
	POINTTYPE_CO3P_,
};	

struct ErrPoint
{
	int				nErrCode;		//�������
	COleDateTime	oleTime;		//����ʱ��
	wstring			strPointName;	//�������
	int				nPointType;		//��������
};

struct DebugLogInfo
{
	COleDateTime	oleTime;		//Logʱ��
	wstring			strDebugLog;	//Log
};

class CDebugLog
{
	public:
		virtual ~CDebugLog();                         // default destructor

 		static CDebugLog* GetInstance();              // get the singleton instance of assign object
		static void DestroyInstance();                   // destroy the singleton instance of assign objec

		static UINT WINAPI ThreadOutPutErrFunc(LPVOID lparam);
		bool	OutPutErr();
		bool	OutPutDebug();
		bool	Exit();

		bool	UpdateDebugSetting();
		bool	DebugLog( E_DEBUG_LOG_TYPE type,E_DEBUG_LOG_LEVEL level,wstring strLog );
		void	SetHistoryDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);

		bool	SetErrPoints(vector<DataPointEntry> vecPoints,int nErrCode);
		bool	SetErrPoint(DataPointEntry entry,int nErrCode);
		bool	SetErrPoint(wstring strPoint,int nPointType, int nErrCode);
		bool	SetErrCode(int nErrCode,bool bOutput=false);
		bool	SetErrPoint(vector<wstring> vecNames,int nPointType,int nErrCode);

		//////////////////////////////////////////////////////////////////////////
		bool	SetDebugLog(wstring strLog);			//core�ĵ���log
		bool	GetOutDebugFlag();

		bool	SetBacnetInfo(wstring strParamName,char* strParamValue);
		bool	SaveBacnetInfo();
		bool	UpdateErrInfo();
		bool	UpdateErrCode(int nErrMinute = 10);
		bool	UpdateErrPoints(int nErrMinute = 10);
	protected:
		void InitParams();                    // initialize parameters and object
		void	OutErrCodeLog(COleDateTime oleNow,int nErrCode);
		void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
	private:
		CDebugLog();                              // default constructor

	private:
		static CDebugLog*    m_csDebugCommObj;     // singleton object
		static Mutex            m_mutexLock;                    // mutex object
		static	bool		m_bExitThread;
		int					m_nDebugType;
		int					m_nDebugLevel;
		int					m_nDebugOut;
		static Beopdatalink::CCommonDBAccess*	m_dbsession_history;
		static HANDLE				m_houtthread;
		//
		static hash_map<int,COleDateTime>	m_mapErrCode;		//�����б�
		static hash_map<wstring,ErrPoint>	m_mapErrPoint;		//������
		static hash_map<wstring,wstring>    m_mapParamValue;	//������

		static	vector<DebugLogInfo>			m_vecDebugLog;		//����Log
		static bool			m_bOutDebugLog;
};

#endif  // __NETWORK_COMM_H__
