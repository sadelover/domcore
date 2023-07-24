
// BEOPWatchDlg.h : header file
//

#pragma once
#include "ProcessView.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include "afxwin.h"
#include "./db/DataHandler.h"
#include "./tcp/DTUSender.h"
#include "./tcp/ExcelOperator.h"
#include "./tcp/DogTcpCtrl.h"
#include "Tools/Zip/zip.h"
#include "Tools/Zip/unzip.h"
#include "LogicBase.h"
#define WM_SHOWTASK (WM_USER+20)	//������Ϣ

typedef CLogicBase* (*pfnLogicBase)();
/*			Զ��DTU����

2|0			//ͬ��Unit01
4|			//�ط���ʧʵʱ�ļ���
5|			//����Unit01
6|			//�����޸ĵ�
8|0			//����������
9|			//��Ӧ������״̬

//////////////////////////////////////////////////////////////////////////
1|��ʼ|����			//��ʷ����
3|0|0				//����Core
3|1|0				//Core�汾��
3|1|1				//ͬ���ֳ����
3|1|2				//��Ӧ�����ļ��б�
3|2|ʱ��			//Core�������
3|3|�ļ���			//��Ӧ�����ļ�
3|4|����			//��Ӧ���ݿ��ļ�

//////////////////////////////////////////////////////////////////////////
5|����|ֵ					//����Unit01
7|����|�ļ���				//��Ӧ��ʧ�����ڼ�����zip
10|0|				//��/�޵��
10|1|����			//ɾ����


//////////////////////////////////////////////////////////////////////////
10|0|����|����		//��/�޵��

*/

/*		DTU����

3|		//Unit01
3|1		//ͬ��
3|2		//�޸�
3|3		//restart
3|5		//sql����
3|6		//��������Ӧ
3|7		//start core
3|8		//������״̬��Ϣ
3|10	//ͬ���ֳ�ʱ��
3|11	//�������
3|12	//������־�б�

6|1		//ɾ���ļ�

*/
// CBEOPWatchDlg dialog
class CBEOPWatchDlg : public CDialog
{
// Construction
public:
	CBEOPWatchDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BEOPWATCH_DIALOG };

	static  unsigned int WINAPI CheckExeThread(LPVOID lpVoid);						//���Core�Ƿ�������
	static  unsigned int WINAPI SendDTUThread(LPVOID lpVoid);						//��������
	static  unsigned int WINAPI SendDTUReceiveHandleThread(LPVOID lpVoid);			//�������Է�����������
	static  unsigned int WINAPI ScanUpdateStateHandleThread(LPVOID lpVoid);			//ɨ�����ݿ��и���״̬��
	static  unsigned int WINAPI CreateTCPConnection(LPVOID lpVoid);					//�̴߳���TCP����
	static  unsigned int WINAPI AckMysqlFileByTableNameThread(LPVOID lpVoid);		//�ظ����ݿ��ļ��߳�

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA		m_nid;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonRestart();
	afx_msg LRESULT OnShowTask(WPARAM wParam,LPARAM lParam); 
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCloseWatch();
	DECLARE_MESSAGE_MAP()

public:
	bool	OutPutLogString(CString strOut);
	
	bool	CheckStateIsNeedRestart();						//���һ���Ƿ���Ҫ����
	bool	HandleRestart();								//������������
	bool	HandleRestartLogic();							//��������Logic����
	bool	RestartCoreByDog(int nType,int& nErrorCode,bool bRestart=false);//����dog��������������Ƿ�ͨ��dog
	bool	RestartLogicByDog(int nType,int& nErrorCode,bool bRestart=false);//����dog��������������Ƿ�ͨ��dog
	bool	WriteActiveTime();								//���߻ʱ��
	bool	SendWatchStart();								//��ʱ������Ŀ��Ϣ
	bool	CloseWatch();

	bool	ScanFile(unitLogFileInfo logFile);				//����.log�ļ� ������backup/filelog�ļ���

	//////////////////////////////////////////////////////////////////////////
	bool	ScanUpdateState();								//ɨ�����ݿ��и���״̬��
	bool	UpdateDTUInfo();								//ɨ��DTU״̬
	bool	OutputMemorySize();								//����ڴ��С
	bool	DeleteBackupFolderByDate();						//��������ɾ�������ļ�
	bool	ScanAndGernerateLogFile();						//ɨ������ļ�log
	bool	DeleteTableByDiskWarning();						//ɾ�����ݿ����ݴ��̱���

	//����
	double	CheckDiskFreeSpace(string strDisk);				//���ش���ʹ����
	bool	DeleteTableByDate(string strTablePrefix, int nDate);//ɾ����
	bool	DeleteTableByDeadDate(string strTablePrefix, int nDate);//ɾ����ֹ������ǰ��
	//////////////////////////////////////////////////////////////////////////

	bool	CloseCore();
	bool	CloseLogic();
	bool	SumRestartTime(int nMinute = 40);					//ͳ��ʱ�����Զ�����3������ ��������Ҫ�ӳ�1����

	bool	InitTCPSender();
	bool	ExitCPSender();
	HANDLE	GetSendDTUEvent();
	bool	SendDTUData();
	bool	ActiveLostSendEvent();								//�д����Ͷ�ʧ�����򼤻�
	bool	HandleCmdFromDTUServer();							//��������DTUServer������
	bool	UpdateDTUSuccess();

	bool	AckReceiveExeFile(string strExeName);								//��Ӧ�����ļ��ɹ���־
	bool	AckUpdateExeFile(string strExeName);								//��Ӧ�����ļ��ɹ���־
	bool	AckLostExeFile(string strLostFile);								//��Ӧ�����ļ��ɹ���־
	bool	AckUpdatePoint(string strPointNames, int nOperation);							//��Ӧ���µ��־
	bool	AckRestartCore();
	bool	AckCoreVersion();								//Core�汾��
	bool	AckCoreErrCode(string nMintue);					//Core�������
	bool	SetSendDTUEventAndType();

	bool	DelteFile(CString strDir,CString strBacDir);						//ɾ���ļ����ⱸ��
	bool	DelteFile(CString strFilePath);						//ɾ���ļ����ⱸ��

	bool	OutPutUpdateLog(CString strLog);		//��¼����Log
	bool	FindFile(CString strFolder,CString strExten,vector<CString>& vecFileName);		//��ָ��Ŀ¼���Һ�׺��ΪXX���ļ�
	bool	FindFile(CString strFolder,CString strName);
	
	//////////////////////////////////////////////////////////////////////////
	bool	SynUnit01();		//ͬ��Unit01
	bool	ChangeValuesByDTUServer(string strReceive);
	bool	ChangeUnitsByDTUServer(string strReceive);
	bool	AckHeartBeat();						//��Ӧ������������
	bool	AckServerState();						//��Ӧ������״̬
	bool	AckErrFileList();						//��Ӧ�����ļ��б�
	bool	AckErrFileByName(string strErrName);	//��Ӧ�����ļ��б�
	void	AckMysqlFileByTableName(string strTableName);						//��Ӧ���ݿ��ļ�
	bool	AckMysqlFileByTableName_(string strTableName);	//��Ӧ���ݿ��ļ�
	bool	AckReSendFileData(string strFileName);					//��Ӧ��ʧ�ط�����
	bool	SendHistoryDataFile(string strStart,string strEnd);
	bool	AckReSendLostZipData(string strMinutes,string strFileNames);			//��Ӧ��ʧ�����ڼ�����
	bool	ChangeUnit01ByDTUServer(string strParamName, string strValue);
	string BuildDTUSendUnit01(vector<unitStateInfo> unitlist);
	int		GetFileInfoPointCont(string strPath,string strFileName);
	bool	SendOneLostHistoryData();							//����һ�������͵���ʷ��ʧ����
	int		GetColumnIndex(std::map<std::string, int> map, string strColumnName);
	bool	CheckDiskFreeSpace(string strDisk,string& strDiskInfo);
	double	CpuUseage();
	int	GetCPUUseage();
	__int64 CompareFileTime ( FILETIME time1, FILETIME time2 );
	double FileTimeToDouble(FILETIME &filetime);
	int		GetMemoryPervent();
	//////////////////////////////////////////////////////////////////////////20151213
	//ͬ���ֳ����
	bool	AckLocalPoint();								//ͬ���ֳ����
	bool	GenerateLocalPointCSV(wstring strFolder,wstring& strLocalZip);	
	bool	ReadDataPoint(string strS3dbPath,vector<DataPointEntry> &vecPoint);
	bool	GeneratePointCSV(vector<DataPointEntry> &vecPoint,wstring strFolder,CString& strCSVName,wstring & strCSVPath);
	bool	ChangeIconByDTUState();
	bool	ChangeIcon(bool bOK);
	bool	ShowDTURouterStateInfo();			//DTU 30����δ������ʱ�����ź�
	bool	ShowDTURouterState(string strRouterIP="192.168.1.241");
	//////////////////////////////////////////////////////////////////////////
	//
	bool	UpdatePoint();		//���µ��
	bool	UpdatePoint_CSV();		//���µ��
	bool	DeleteMutilPoint(string strPoints);
	bool	UpdateMutilPoint(string strPoints);
	bool	UpdateMutilPoint(vector<DataPointEntry>& vecPoint);

	bool	UpdateMutilPoint_(string strPropertyCount,string strPoints);
	bool	GeneratePointFromInfo(vector<vector<wstring>> vecPoints, vector<DataPointEntry>& vecPoint);
	bool	UpdateDll();			//���²���
	bool	UpdateS3db();			//����S3db
	bool	UpdateCore();			//����Core
	bool	UpdateWatch();		//����Watch
	bool	UpdateLogic();			//����Logic
	bool	UpdateRestart();
	bool	UpdateBat();			//ִ���������ļ�

	bool	UpdatePointXls(CString strExcelPath,CString strS3dbPath,int& nErrCode);
	bool	UpdatePointCSV(CString strCSVPath,CString strS3dbPath,int& nErrCode);

	bool	Exit();
	bool	GetExitThread_History() const;

	bool	FindAndDeleteBackupDirectory(CString strDir,CString strDeadFolder);
	bool	FindAndDeleteErrFile(CString strDir,CString strDeadFileName);
	bool	FindAndDeleteErrFile(CString strDir,CString strFilterName,CString strFilterPri,CString strDeadFileName);
	bool	DelteFoder(CString strDir);

	bool	ResetTCPInfo();

	bool	OpenDog();
	bool	ImproveProcPriv();
	bool	TestNetWork();

	bool	FeedDog(bool bFirst = true);				//ι��

	//////////////////////////////////////////////////////////////////////////
	bool	UpdateDll(int nUpdateSate,CString strDllName,CString strThreadName,CString strS3dbPath);
	bool	StoreDllToDB(CString strDllPath,CString strS3dbPath,CString strDllName,CString strThreadName,CString strVersion,CString strDescription,CString strAuthor);
	bool	DeleteDll(CString strS3dbPath,CString strDllName,CString strThreadName);
	bool	UpdateDllThread(CString strS3dbPath,CString strDllName,CString strThreadName);
	bool	UpdateDllParamter(CString strS3dbPath,wstring strParamSql);
	bool	DeleteDllParamter(CString strS3dbPath,wstring strParamSql);
	bool	UpdateThreadState(wstring strS3dbPath,wstring strThreadName,wstring strState,int nPeriod=-1);
	bool	UpdateThreadState(wstring strS3dbPath,wstring strState,wstring strThreadName,wstring strValue);

	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<string>& resultlist);		//����),����

private:
	CProcessView		m_PrsV;
	bool                m_bFirst;
	bool				m_bChangeIcon;
public:
	CEdit	m_eLog;
	CString m_strLog;
	CString	m_strPath;
	CString	m_strLogicPath;
	CString m_strIniFilePath;
	CString m_strLogFilePath;
	CString m_strMemoryLogFilePath;
	int		m_nErrorCode;

	//
	CString	m_strUpdateS3dbPath;
	CString	m_strUpdateXlsFile;
	CString	m_strUpdateCorePath;
	CString	m_strUpdateBatFile;

	int		m_nRestartType;					//������ʽ��-1: �������� 0:�ֶ����� 1 Core������; 2:servertime10����δ���� 3��������5����δ���� 4:dtu60����δ���� 5:���µ�� 6������core 7������dll 8������dog 9��Զ������ 10:��ʱ���� 11:��������dtu���� 12����s3db
	int		m_nRestartLogicType;			//������ʽ��-1: �������� 0:�ֶ����� 1 Logic������; 2:servertime10����δ���� 6������logic 7������dll  9��Զ������ 12����s3db(ʵ��ֻд���ݿ⣬������core��������)
	HANDLE		m_hcheckhandle;
	int		m_nErrCode;

	vector<COleDateTime> m_vecReStartTime;			//�Զ�����ʱ��
	CDataHandler* m_pDataHandle;	//���ݿ�����

	CDTUSender*	m_pdtusender;
	bool		m_bdtuSuccess;
	bool		m_bexitthread_history;
	HANDLE		m_senddtuevent;
	HANDLE		m_hsenddtuhandle;
	HANDLE		m_hReceivedtuhandle;
	HANDLE		m_hScanUpdateStatehandle;
	wstring		m_strTCPName;						//���ڱ�ʶTCP��Ŀ
	wstring		m_strTCPPort;						//���ڱ�ʶTCP��Ŀ

	vector<DTUSendInfo>	m_vecSendBuffer;		//DTU���ͻ���
	bool		m_bVecCopying;					//����copy�������
	int			m_nDTUState;					//Core DTU����״̬  0ʧ��  1�ɹ�
	int			m_nOutPutMemoryInterval;		//����ڴ���
	COleDateTime	m_oleLastDleteTime;
	COleDateTime	m_oleLastRestartTime;			//��һ���Զ�����ʱ��
	COleDateTime	m_oleLastSendReg;				//��һ�η���ע����Ϣ
	COleDateTime	m_oleStartTime;					//����ʱ��
	COleDateTime	m_oleShowRouterState;			//��ѯDTU״̬
	COleDateTime	m_oleGetMemoryTime;				//��ѯ�ڴ��С 30���Ӽ�¼һ��
	map<string,int>					m_mapSendLostHistory;
	string      m_strSendRealFilePath;				//��Ҫ�������͵�ʵʱ����ַ
	int			m_nSendRealFileCount;				//��Ҫ�������͵�ʵʱ������
	string		m_strAck;							//�޸ĵ�ACK�ظ����ݺϲ�����
	SYSTEMTIME m_ackTime;							//�ظ�ʱ��
	string		m_strZipPath;
	string		m_strAckMysqlTableName;				//
	Project::Tools::Mutex m_dtu_lock;
	Project::Tools::Mutex m_dtu_ack_lock;
	Project::Tools::Mutex m_watch_lock;

	FILETIME m_preidleTime;
	FILETIME m_prekernelTime;
	FILETIME m_preuserTime;
	bool			m_bCloseDlg;
	afx_msg void OnBnClickedCheckTcpConnection();
};
