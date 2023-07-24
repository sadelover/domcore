#pragma once

/*
	This dtu sender will send the data to the dtu.
	the dtu is connected to the com port. So we just need to 
	write to the com port.
*/
#include <memory>
#include <string>
#include <vector>
#include <queue>

#include "../COMdll/COM/SerialPort.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "../COMdll/ModbusTcp/TCPDataSender.h"
using std::string;
using std::vector;

class CSerialPort;
class CTCPDataSender;
//class CBEOPDataLinkManager;
using namespace Beopdatalink;

struct _declspec(dllexport) DTUSendInfo
{
	DTUSendInfo()
	{
		nType = 0;
		nSubType = 0;
		nPointCount = 0;
		strPacketIndex = "-1";
		GetLocalTime(&tSendTime);
	}
	SYSTEMTIME tSendTime;
	string	   strSendBuf;
	int		   nType;		//0:实时数据  1：报警数据  2：报警操作 3:unit01 4 ExecuteLog 5 resend 6History 7realdata file 8exe更新回执 9用户操作
	int		   nSubType;	//0:send 1:syn  2:edit 3:restart 4批量修改 5：sql命令 6心跳包 7 start 8 history
	int		   nPointCount;		//发送的点个数
	string	   strPacketIndex;	//包编号   0-9 类型数据同步命令 10同步unit01命令 11 修改unit01命令 12 重启Core 13 心跳包在线  14单个修改实时值
};

struct _declspec(dllexport) DTUServerCmdInfo
{
	DTUServerCmdInfo()
	{
		nLength = 0;
		memset(cData,0,g_nPackageSize);
	}
	char   cData[g_nPackageSize];				//内容
	int	   nLength;					//长度
};

//CmdType:   1:data 2:同步Unit01表 3重启Core 4 设置点 5 设置Unit01 6 批量修改点(以,分割)  7 sql语句 8 心跳包 9重发丢失数据包
//StateType: 0:send 1:syn  2:edit 3 resend 

class _declspec(dllexport) CDTUSender
{
public:
	CDTUSender(CCommonDBAccess* dbsession_history,bool bDTUChecked,bool bDTURecCmd,string strTCPName,string strHost="",u_short portnumer=9500,bool bDTUMode = true,
			  int portnumber = 1, int baudrate = 9600);
	~CDTUSender(void);

	bool SendData(char* buffer, int buffersize);

	bool SendDataByWrap(char* buffer, int buffersize);//自动添加换行符

	//初始化接口
	bool InitDTU();
	bool Exit();

	static UINT WINAPI DTUThreadFunc(LPVOID lparam);
	static UINT WINAPI DTUThreadHandRecCmd(LPVOID lparam);
	bool	ScanAndSendLostPackage(string& strUpdateExeAck,string& strLostFile);

	bool	ScanAndSendLostPackage();

	bool SendData();

	bool SendData(const SYSTEMTIME tSendTime, const string& strSendBuff);						//DTU发送实时数据

	bool SendWarningData(const SYSTEMTIME tSendTime, const string& strSendBuff);				//DTU发送报警数据

	bool SendWarnigOperationData(const SYSTEMTIME tSendTime, const string& strSendBuff);		//DTU 发送报警操作数据

	bool SendUserOperationData(const SYSTEMTIME tSendTime, const string& strSendBuff);		//DTU 发送用户操作数据

	bool SendExecuteLog(const SYSTEMTIME tSendTime, const string& strSendBuff);				//DTU 发送执行顺序

	bool SendUnit01(const SYSTEMTIME tSendTime, const string& strSendBuff);		//DTU 发送Unit01数据

	bool SendHeartBeat(const SYSTEMTIME tSendTime);								//DTU 发送心跳包回应

	bool SendCoreStart(const string strProjectName);								//DTU 发送启动标志

	bool SendFile(string strFilePath);					//DTU发送文件

	bool SendRealDataFile(string strFilePath,int nPointCount);					//DTU发送实时数据文件

	bool SendHistoryDataFile(string strFilePath,int nPointCount);					//DTU发送历史丢失文件

	bool SendUpdateFileAck(int nState, string strContetn);		//DTU发送更新文件回执  1:更新exe成功  2:更新exe失败  3发送丢失文件成功 4发送丢失文件失败

	bool SendCmdState(const SYSTEMTIME tSendTime,int nCmdType, int nCmdState, string strPacketIndex);

	bool SendRDP_AT();			//RDP  进入AT指令

	bool SendRDP_Login();		//AT 登录指令

	bool SendRDP_QueryDTUID();	//查询DTU标示符

	bool SendRDP_QueryDSCIP();	//查询DSCIP

	bool SendRDP_QueryCSQ();	//查询信号

	bool SendRDP_Reset();		//退出AT 重启DTU

	bool SendQueryState();

	bool RestartDTU();			//重启DTU

	bool SendLogin();

	bool SendLogin1();

	//DTU每次最长只能传输1024个字节。安全起见，每次最多只传输512个字节的数据。
	//需要打包。
	void BuildPackage(const string& buffer, vector<string>& resultlist);

	void BuildAndSendOperationRecord();

	void	ExitThreads();

	bool	GetReadThreadExit() const;
	bool	GetSendThreadExit() const;
	bool	GetRecThreadExit() const;
	//每偶数分钟定时读数据。
	void	BuildData(const SYSTEMTIME& st);

	void	AddLog(const wchar_t* loginfo);
	std::string	GetSuccessBuffer();

	//接收数据
	static LRESULT OnRecData(const unsigned char* pRevData, DWORD length,DWORD userdata);
	void   GetOutputByRecData(const char* buffer,vector<Beopdatalink::CRealTimeDataEntry>& entrylist);

	void   GetDTU_TO_RTU(const unsigned char* pRevData, DWORD length);

	bool	AnalyzeReceive(const unsigned char* pRevData, DWORD length);
	void	AnalyzeUpdateFile(const unsigned char* buffer);
	void	AnalyzeRecLogFile(const unsigned char* buffer);

	bool	GetReceiveDTUServerEvent(vector<vector<string>>& vecCmd);		//DTUServer有命令传递
	bool	SetReceiveDTUServerEvent(bool bReceiveDTUServerEvent);
	void	UpdateReceiveBuffer(char* pBuffer);
	void	UpdateDTUWriteTime(char* pBuffer);

	void   SplitString(const std::string& source, const char* sep, std::vector<string>& resultlist);

	bool	StoreSendInfo(string strSendContent);
	string	GetStoreInfo(int nIndex,int& nBase);
	bool	ReSendRealData(string strIndex);

	bool	OutPutLogString(char* pOut);
	bool	OutPutLogErrString(char* pOut);

	bool	HandRecCmd();
	bool	GetSendIdle(int nSeconds=4);
	void	SetDTUToRTU(bool bDTU_To_RTU);

	string	GetDTUCSQInfo();

	bool	StoreDTUCmd(DTUServerCmdInfo data);
	//void	SplitStringSpecial(const char* buffer,int nSize, std::vector<char*>& resultlist,std::vector<int>& sizelist);		//争对;\n操作
	unsigned int BufferToInt(const unsigned char* buffer);

	bool	GetDTUSuccess();
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	std::auto_ptr<CSerialPort> m_porthandler;
	std::auto_ptr<CSerialPort> m_porthandler2;

	std::auto_ptr<CTCPDataSender> m_tcphandler;

	int m_portnumber;		//端口号
	int m_baudrate;			//波特率

	HANDLE m_sendthread;
	bool	m_bsendthreadexit;

	HANDLE	m_readthread;
	HANDLE m_recthread;
	bool    m_brecthreadexit;
	std::string m_sendbuffer;
	SYSTEMTIME  m_sendtime;
	bool    m_breadthreadexit;
	bool	m_bDTU_To_RTU;
	bool	m_bDTUConnectOK;		//DTU连接成功
	bool	m_bReceiveDTU;			//收到DTU发送
	bool	m_bReceiveDTUServerEvent;			//收到DTUServer发送命令
	bool	m_bDTUChecked;
	bool	m_bDTURecCmd;
	int		m_nReceiveDTUCmdType;				//收到DTUServer命令种类
	char*   m_pReceiveDTUServer;
	DWORD	m_dReceiveDTUServer;
	vector<string>	m_vecCmd;

	Project::Tools::Mutex	m_lock;
	Project::Tools::Mutex	m_sendlock;
	Project::Tools::Mutex	m_reveivelock;
	Project::Tools::Mutex	m_reclock;
	//CBEOPDataLinkManager* m_pBeopdatalinker;
	COleDateTime	m_oleLastWriteTime;			//上一次写DTU时间

	int		m_nDTUPackageNumber;				//发送实时数据包编号
	int		m_nDTUPackageBase;					//实时数据发包基数
	int		m_nDTUFilePackageNumber;			//文件包编号
	map<int, string> m_mapStoreSendPackage;		//存储最近发送的包
	map<int,int>	 m_mapStoreSendPackageBase;	//存储最近发送包的基础号（丢包确认时候使用）
	map<int, string> m_mapStoreFilePackage;		//存储最近一次发送文件的内容

	vector<vector<string>>	m_vecDTUCmd;			//收到的服务器命令
	queue<string>	m_qRecDTUCmd;					//收到的服务器队列

	char			m_cRXBuff[MAX_REC_COUNT];			//DTU接收字符串
	char			m_cSpiltBuffer[MAX_REC_COUNT];			//数据解析缓存
	char			m_cRTUBuffer[MAX_REC_COUNT];			//RTU交换
	char			m_cSpiltBufferSpilt[MAX_REC_COUNT];			//数据解析缓存
	char			m_cFileBuffer[1024][g_nPackageSize];					//文件存储
	char			m_cFileUpdateBuffer[1024][g_nPackageSize];			//Core更新
	map<int,int>	m_mapFileUpdateState;						//文件收包情况
	

	int				m_nReceCount;
	bool			m_bRunReserverPort;
	int				m_nRunReserverPort;

	string			m_strDSCIP;
	string			m_strDTUID;				//DTU检测出来的ID
	string			m_strCSQ;
	int				m_nRTUAnswerMode;		//RTU回应

	CRITICAL_SECTION m_csDTUDataSync;			//临界区对象 保护共享资源
	queue<DTUServerCmdInfo>	m_queCmdPaket;		//存储服务器命令包

	string			m_strRecFileName;				//文件名称
	int				m_nFileCount;					//文件包总数
	int				m_nRecFileCount;				//收到的文件包总数


	string			m_strCoreFileName;				//Core文件名称
	int				m_nCoreFileCount;				//Core文件包总数
	int				m_nRecCoreFileCount;			//收到的Core文件包总数
	COleDateTime	m_oleUpdateExeTime;				//收到更新exe时间
	bool			m_bHasLostFileFlag;				//需要重发标志
	string			m_strUpdateExeAck;				//成功更新EXE回执
	int				m_nLastUpdateExeSize;				//最后一个包的大小

	bool			m_bDTUMode;						//true为DTU模式  false 为TCP模式
	string			m_host;
	u_short			m_port;
	string			m_strTcpName;					//名称
	bool			m_bdtuSuccess;					//DTU引擎初始化成功

	int				m_nTCPSendFilePackageSize;				//TCP每次发包的长度 默认为1000（2gDTU只能选择该配置） TCP为4000
	int				m_nTCPSendFilePackageInterval;			//TCP每次发包的长度 默认为2000ms（2gDTU只能选择该配置）	TCP最快为200ms

	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
};

