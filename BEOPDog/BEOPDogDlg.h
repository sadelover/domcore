
// BEOPDogDlg.h : header file
//

#pragma once
#include "ProcessView.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <hash_map>
#include "afxwin.h"
#include "TCPCustom.h"
#include "TCPServer.h"
#include "RedundencyCtrl.h"

#define WM_SHOWTASK (WM_USER+20)	//������Ϣ

// CBEOPDogDlg dialog
class CBEOPDogDlg : public CDialog
{
// Construction
public:
	CBEOPDogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BEOPDog_DIALOG };

	static  unsigned int WINAPI MasterThread(LPVOID lpVoid);						//Master�������߳�
	static  unsigned int WINAPI MasterTaskHandleThread(LPVOID lpVoid);				//Master�������߳�

	bool	MasterMain();															//Master��������
	bool	MasterTaskHandle();
	bool	MasterTask(E_RESTART_TYPE eType,E_RESTART_REASON eReason);
	bool	MasterTaskResult(E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult);
	bool	MasterTaskLog(E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult);
	bool	MasterSlaveCloseAll();													//Master�ӻ��ر����г���

	bool	MasterHandle();									//��������������
	bool	CheckExeState();								//���ι���Ƿ�ʱ
	bool	CheckOherExe();
	bool	FindExeByName(CString strExeName);
	bool	OpenExeByName(CString strExeName);
	bool	OpenExeByPath(CString strExePath);
	bool	CloseExe();
	bool	CloseOherExe();
	bool	CloseExeByName(CString strExeName);

	bool	ChangeIcon();

	//////////////////////////////////////////////////////////////////////////
	bool	RestartExe(E_RESTART_TYPE eTyp,E_RESTART_REASON eReason);				//����Exe
	bool	CloseExe(E_RESTART_TYPE eTyp);					//�ر�Exe
	//////////////////////////////////////////////////////////////////////////

	void	StoreTask(int nRestartType,int nReason);
	void	RegTask(E_RESTART_REASON eRegType, char* strProgName);

	bool	ReadConfig();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private: 
	//�ͻ������ӽ����¼������� 
	static void CALLBACK	OnClientConnect(void* pOwner,CTCPCustom *pTcpCustom); 
	//�ͻ���SOCKET�ر��¼������� 
	static void  CALLBACK OnClientClose(void* pOwner,CTCPCustom*pTcpCustom); 
	//���������յ����Կͻ��˵����� 
	static  void CALLBACK OnClientRead(void* pOwner,CTCPCustom* pTcpCustom,const char * buf,DWORD dwBufLen ); 
	//�ͻ���Socket�����¼������� 
	static  void CALLBACK OnClientError(void* pOwner,CTCPCustom* pTcpCustom,int nErrorCode); 
	//��������Socket�����¼������� 
	static void CALLBACK OnServerError(void* pOwner,CTCPServer* pTcpServer_CE,int nErrorCode); 

	static  void CALLBACK OnClientRestartTask(void* pOwner,CTCPCustom* pTcpCustom,int nRestartType,E_RESTART_REASON eReason); 

	static void CALLBACK OnClientRegedit(void* pOwner,CTCPCustom* pTcpCustom,E_RESTART_REASON eRegType, char* strProgName); 
// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA		m_nid;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnShowTask(WPARAM wParam,LPARAM lParam); 
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCloseDog();
	DECLARE_MESSAGE_MAP()

public:
	bool	OutPutLogString(CString strOut);
	bool	InitTCPDataCernter();
	bool	CloseDog();
	bool	Exit();
private:
	CProcessView		m_PrsV;
	bool                m_bFirst;

public:
	CString m_strLog;
	CString	m_strPath;
	CString m_strIniFilePath;
	CString m_strLogFilePath;

	CString	m_strCorePath;
	CString m_strLogicPath;
	CString m_strWatchPath;

	CTCPServer		m_tcpServer; 						//TCPServer
	hash_map<E_RESTART_TYPE,E_RESTART_REASON>	m_mapTaskList;
	CRITICAL_SECTION        m_criTask; 

	bool		m_bExitThread;
	HANDLE		m_hMasterTaskHandle;

	COleDateTime	m_oleRegByCore;		//Coreע��ʱ��
	COleDateTime	m_oleRegByLogic;	//Logicע��ʱ��
	COleDateTime	m_oleRegByUpdate;	//Updateע��ʱ��
	COleDateTime	m_oleRegByDTU;		//DTUע��ʱ��
	COleDateTime	m_oleCheckTime;		//�ϴμ��ʱ��

	int			m_nCheckInterval;		//��ü��һ��
	int			m_nTimeOut;				//���û�յ�ι���жϳ�ʱ
	bool		m_bIconOnline;
	bool		m_bCloseDlg;

	bool		m_bEnableCore;			//����Coreι��
	bool		m_bEnableUpdate;		//����Updateι��
	bool		m_bEnableLogic;			//����Logicι��
	bool		m_bEnableDTUEngine;		//����DTUEngineι��
	wstring		m_wstrRedundencyIP;		//����IP
	wstring		m_strMasterVersion;		//�汾��
	int			m_nMasterTCPPort;		//Master TCP�˿�
	int			m_nRedundencyPort;		//����˿�
	int			m_nRedundencyTimeOut;	//����ι����ʱ
	int			m_nMasterFeedLimitPerHour;//ι��ÿСʱ��������
	wstring		m_wstrOtherCheckExeName;//������ά��Exe
	wstring		m_wstrOtherCheckExePath;//������ά��Exe

	hash_map<COleDateTime,E_RESTART_REASON>	m_mapCoreUnFeedRestart;		//coreι����ʱ������¼
	hash_map<COleDateTime,E_RESTART_REASON>	m_mapLogicUnFeedRestart;	//Logicι����ʱ������¼
	hash_map<COleDateTime,E_RESTART_REASON>	m_mapUpdateUnFeedRestart;	//Updateι����ʱ������¼
	hash_map<COleDateTime,E_RESTART_REASON>	m_mapDTUEngineUnFeedRestart;//DTUEngineι����ʱ������¼
	hash_map<string,COleDateTime>			m_mapThirdRegTime;			//������ע��ʱ��

	Project::Tools::Mutex m_lock;
	Project::Tools::Mutex m_dog_lock;
};
