
// BEOPOPCClientDlg.h : header file
//

#pragma once
#include "./OPCCtrl/BEOPOPCCtrl.h"
#include "DataPointEntry.h"
#define WM_SHOWTASK (WM_USER+20)	//������Ϣ

class COPCEngine;

struct OPCServer
{
	wstring strIP;
	wstring strServerName;
	int		nCount;		
};

typedef struct _tagWriteNetCfg
{
	CString	strIp;
	int		nPort;
	CString	strUserName;
	CString	strPwd;
	CString	strDbName;

	_tagWriteNetCfg(void)
	{
		strIp = _T("127.0.0.1");
		nPort = 3306;
		strUserName = _T("domcore");
		strPwd = _T("RNB.beop-2013");
		strDbName = _T("test");
	}
}WriteNetCfg;


// CBEOPOPCClientDlg dialog
class CBEOPOPCClientDlg : public CDialog
{
// Construction
public:
	CBEOPOPCClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BEOPOPCCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA	m_nid;

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
	DECLARE_MESSAGE_MAP()

public:
	bool	InitConfigParams();							//��ʼ����Ϣ
	bool	OutPutLogString(CString strOut);			//���log
	bool	InitOPC();									//��ʼ��OPC
	bool	ExitOPC();									//�˳��ͷ�
	void	GetOPCValueSets(vector<pair<wstring, wstring> >& opcvaluesets);
protected:
	bool	InitOPC(COPCEngine* opcengine,int nOPCClientIndex = -1);
	vector<vector<OPCDataPointEntry>> GetOPCServer(vector<OPCDataPointEntry> opcpoint,int nMaxSize,wstring strDefaultIP);	
	int	IsExistInVec(wstring strSer, wstring strProgram,vector<OPCServer>& vecSer,int nMaxSize);
private:
	void	InitConfig(void);
	void	InitConfigFile(const CString strPath);
	void	ReadConfigInfo(const CString strPath);
	int		CreateDir(CString strFolderPath);
	void	WriteLog(CString strLog);

	static unsigned __stdcall GetOpcValueThread(void* pArguments);
	void	WriteIntoRealTimeData(const vector<pair<wstring, wstring> >& vecOpc);
	MYSQL	m_mysql;
	WriteNetCfg	m_stWriteInfo;
	CString	m_strExeDir;
	CString	m_strCfgPathName;
	CString	m_strLogDir;
	int		m_nSleep;


public:
	bool						m_bFirst;
	vector<OPCDataPointEntry>	m_opcpointlist;
	vector<OPCServer>			m_vecOPCSer;
	map<int,COPCEngine*>	m_mapOPCEngine;				//index���OPC���棨OPC�����10000ʱ�����ã�
	COPCEngine* m_opcengine;

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
	int			m_nOPCCheckReconnectInterval;	//OPC������������min��
	int			m_nOPCReconnectInertval;		//OPC N��������δ��������(min)
	int			m_nDebugOPC;
	wstring		m_strEnableSecurity;			//���ý��̰�ȫ

	CString		m_strCsvPathName;
};
