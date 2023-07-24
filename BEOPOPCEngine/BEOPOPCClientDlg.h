
// BEOPOPCClientDlg.h : header file
//

#pragma once
#include "./OPCCtrl//BEOPOPCCtrl.h"
#include "DataPointEntry.h"
#include "./db/DataHandler.h"
#define WM_SHOWTASK (WM_USER+20)	//������Ϣ

class COPCEngine;

struct OPCServer
{
	wstring strIP;
	wstring strServerName;
	int		nCount;		
};

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
	afx_msg void OnCloseWatch();
	DECLARE_MESSAGE_MAP()

public:
	bool	InitConfigParams();							//��ʼ����Ϣ
	bool	OutPutLogString(wstring strOut,bool bDaily=false);			//���log
	bool	InitOPC();									//��ʼ��OPC
	bool	ExitOPC();									//�˳��ͷ�
	bool	Exit();
	bool	CloseOPCClient();
	bool	ChangeIcon();
	bool	UpdateDebug();
protected:
	bool	InitOPC(COPCEngine* opcengine,int nOPCClientIndex = -1);
	vector<vector<OPCDataPointEntry>> GetOPCServer(vector<OPCDataPointEntry> opcpoint,int nMaxSize,wstring strDefaultIP);	
	int	IsExistInVec(wstring strSer, wstring strProgram,vector<OPCServer>& vecSer,int nMaxSize);

	wstring PreInitFileDirectory();
	bool ReadFromCSV_Comma(CString sExcelFile, vector<OPCDataPointEntry>& vecPoint,int& nErrCode);		//����ֵ������ŵ�CSV

	static  unsigned int WINAPI UpdateMysqlThread(LPVOID lpVoid);						//�������ݿ�output��
	bool	UpdateMysqlValue();
	void	GetOPCValueSets(vector<pair<wstring, wstring> >& opcvaluesets);

public:
	bool						m_bFirst;
	vector<OPCDataPointEntry>		m_opcpointlist;
	vector<OPCServer>			m_vecOPCSer;
	map<int,COPCEngine*>	m_mapOPCEngine;				//index���OPC���棨OPC�����10000ʱ�����ã�
	COPCEngine* m_opcengine;
	CDataHandler* m_pDataHandle;	//���ݿ�����
	bool							m_bExitThread;
	HANDLE							m_hupdatehandle;
	bool			m_bCloseDlg;

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
	wstring		m_strOPCServerIP;				//OPC��������ַ
	int			m_nUpdateInterval;				//�������ݿ���
	bool		m_bAsync20Mode;					//�첽��дģʽ
	int			m_nRefresh20Interval;			//�첽ˢ�¼��
	int			m_nPrecision;
	Project::Tools::Mutex m_mtuex;			//������
	bool		m_bIconOnline;			//ͼ������
};
