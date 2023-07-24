
// BEOPClinetSampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BEOPClinetSample.h"
#include "BEOPClinetSampleDlg.h"
#include "afxdialogex.h"
#include "CSVParser.h"
#include "RestartAutoRun.h"
using namespace CSVParser;

/*
	���޸ģ�
		1���������Ƽ������ļ�,log����
		2����������������Ҫע��
		3: Init ��������Ҫ��ʼ������������
		4: GetSampleValueSets ��ȡ��������
		5: Exit ������Ҫ���и����˳��ͷ�
		6��BEOPClinetSample.cpp����ĳ�����
*/

#define SAMPLE_CLIENT_NAME	_T("BEOPClientSample V1.0.1")
#define SAMPLE_INI_NAME		_T("beopclientsample.ini")
#define SAMPLE_LOG_NAME		_T("beopsapmpleclient.log")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBEOPClinetSampleDlg dialog




CBEOPClinetSampleDlg::CBEOPClinetSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBEOPClinetSampleDlg::IDD, pParent)
	, m_bFirst(TRUE),
	m_bExitThread(false),
	m_hupdatehandle(NULL),
	m_bCloseDlg(false),
	m_nUpdateInterval(2),
	m_pDataHandle(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBEOPClinetSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBEOPClinetSampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CBEOPClinetSampleDlg message handlers

BOOL CBEOPClinetSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//��ʼ��ϵͳ����ͼ��
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;//�Զ������Ϣ����
	m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));//����ͼ��
	wcscpy(m_nid.szTip, SAMPLE_CLIENT_NAME);			//��Ϣ��ʾ  edit
	Shell_NotifyIcon(NIM_ADD, &m_nid);//�����������ͼ��
	ShowWindow(SW_HIDE);//���ش���  

	//дע��� ��������
	TCHAR exepath[MAX_PATH];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	CRestartAutoRun::SetAutoRun(exepath);

	//��ʼ��
	Init();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBEOPClinetSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if(nID==SC_MINIMIZE)
	{
		ShowWindow(SW_HIDE);//���ش���
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBEOPClinetSampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		if(m_bFirst)
		{
			ShowWindow(SW_HIDE);
			m_bFirst = false;
		}
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBEOPClinetSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CBEOPClinetSampleDlg::OnShowTask( WPARAM wParam,LPARAM lParam )
{
	if(wParam!=IDR_MAINFRAME) 
		return 1; 

	switch(lParam) 
	{ 
	case WM_RBUTTONUP://�Ҽ�����ʱ������ݲ˵�������ֻ��һ�����رա� 
		{ 
			LPPOINT lpoint=new tagPOINT; 
			::GetCursorPos(lpoint);//�õ����λ�� 
			CMenu menu;
			menu.CreatePopupMenu();//����һ������ʽ�˵� 
			//���Ӳ˵���رա������������ϢWM_DESTROY�������ڣ���//���أ�������������� 
			menu.AppendMenu(MF_STRING,WM_DESTROY,_T("Close(&C)"));
			//ȷ������ʽ�˵���λ�� 
			menu.TrackPopupMenu(TPM_LEFTALIGN,lpoint->x,lpoint->y,this);
			//��Դ���� 
			HMENU hmenu=menu.Detach();
			menu.DestroyMenu();
			delete lpoint;
		} 
		break;
	case WM_LBUTTONDBLCLK://˫������Ĵ��� 
		{
			//this->ShowWindow(SW_SHOW);//�򵥵���ʾ���������¶�
		}
		break; 
	default:
		break;
	} 
	return 0;
}

BOOL CBEOPClinetSampleDlg::OnEraseBkgnd( CDC* pDC )
{
	static bool bFirst=true;
	if(bFirst)
	{
		bFirst=false;
		ShowWindow(SW_HIDE);
		return TRUE;
	}

	return CDialog::OnEraseBkgnd(pDC);
}

void CBEOPClinetSampleDlg::OnClose()
{
	if(m_bCloseDlg)
	{
		m_bCloseDlg = false;
		CDialog::OnClose();
	}
	else
	{
		ShowWindow(SW_HIDE);//���ش���
	}
}

void CBEOPClinetSampleDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	//ɾ������ͼ��
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void CBEOPClinetSampleDlg::OnTimer( UINT_PTR nIDEvent )
{
	CDialog::OnTimer(nIDEvent);
}

void CBEOPClinetSampleDlg::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if(nType == SIZE_MINIMIZED)
	{
		ShowWindow(SW_HIDE);
	}
}

BOOL CBEOPClinetSampleDlg::PreTranslateMessage( MSG* pMsg )
{
	DWORD msg = pMsg->message;
	if (msg == WM_KEYDOWN)
	{
		DWORD keytype = pMsg->wParam;
		if (keytype == VK_ESCAPE || keytype == VK_RETURN){
			return true;
		}
	}
	if (msg == WM_CLOSE)
	{
		OnClose();
	}
	return CDialog::PreTranslateMessage(pMsg);
}

bool CBEOPClinetSampleDlg::Init()
{
	wstring strPath;
	GetSysPath(strPath);
	CString strIniFilePath;
	strIniFilePath.Format(_T("%s\\%s"),strPath.c_str(),SAMPLE_INI_NAME);				//edit

	//��ʼ�����ݿ����
	/*
	wchar_t charContent[256];
	GetPrivateProfileString(L"db", L"defaulathost", L"", charContent, 256, strIniFilePath);
	wstring wstrDefaultHost = charContent;
	if(wstrDefaultHost == L"")
		wstrDefaultHost = L"localhost";
	WritePrivateProfileString(L"db",L"defaulathost",wstrDefaultHost.c_str(),strIniFilePath);
	*/

	//��ʼ�����
	wstring strCSVPath = PreInitFileDirectory();
	if(strCSVPath.length() <= 0)
	{
		OutPutLogString(L"ERROR:Can't find Point CSV!");
		MessageBox(_T("ERROR: Can't find Point CSV!"));
		EndDialog(0);
		return 0;
	}

	int nErrCode = 0;
	if(!ReadFromCSV_Comma(strCSVPath.c_str(),m_pointlist,nErrCode))
	{
		OutPutLogString(L"ERROR:Read Point CSV Fail!");
		MessageBox(_T("ERROR: Read Point CSV Fail!"));
		EndDialog(0);
		return 0;
	}

	//��ʼ�����ݿ�
	wstring wstrDefaultHost,wstrDefaultUser,wstrDefaultPsw,wstrDefaultDB;
	m_pDataHandle = new CDataHandler();
	if(m_pDataHandle)
	{
		if(!m_pDataHandle->Connect(wstrDefaultHost,wstrDefaultUser,wstrDefaultPsw,wstrDefaultDB,3306))
		{
			MessageBox(_T("ERROR: Connect DB Fail!"));
			EndDialog(0);
			return 0;
		}
	}

	//��ʼ������

	//�����߳�
	m_hupdatehandle = (HANDLE)_beginthreadex(NULL, 0, UpdateMysqlThread, this, 0, NULL);
	return true;
}

bool CBEOPClinetSampleDlg::ReadFromCSV_Comma( CString sExcelFile, vector<ClientDataPointEntry>& vecPoint,int& nErrCode )
{
	try
	{
		Csv csv = Csv(Project::Tools::WideCharToAnsi(sExcelFile).c_str());
		//
		if(csv.getRowCount() >0)
		{
			std::map<CString,int> mapPointName;
			for(int i=0; i<csv.getRowCount(); ++i)
			{
				//ÿһ��
				vector<wstring> vecPointInfo;
				ClientDataPointEntry entry;
				CString strPhysicalID = Project::Tools::AnsiToWideChar(csv[i][0].c_str()).c_str();
				CString strparam1 = Project::Tools::AnsiToWideChar(csv[i][1].c_str()).c_str();
				CString strparam2 = Project::Tools::AnsiToWideChar(csv[i][2].c_str()).c_str();
				CString strparam3 = Project::Tools::AnsiToWideChar(csv[i][3].c_str()).c_str();
				CString strparam4 = Project::Tools::AnsiToWideChar(csv[i][4].c_str()).c_str();
				entry.SetShortName(strPhysicalID.GetString());			//edit
				entry.SetSourceType(L"OPC");

				entry.SetParam(1, strparam1.GetString());
				entry.SetParam(2, strparam2.GetString());
				entry.SetParam(3, strparam3.GetString());
				entry.SetParam(4, strparam4.GetString());
				if(entry.GetShortName().length() >0)
					vecPoint.push_back(entry);
			}
			return true;
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
	return false;
}

wstring CBEOPClinetSampleDlg::PreInitFileDirectory()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);

	wstring strDBFileName;


	CFileFind s3dbfilefinder;
	wstring strSearchPath = cstrFile + L"\\*.csv";
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
			if(!DeleteFile(strFileNameToDelete.c_str()))
			{
				_tprintf(L"ERROR: Delete UnNeccesary DB Files Failed!");
			}
		}
	}
	return strDBFileName;
}

unsigned int WINAPI CBEOPClinetSampleDlg::UpdateMysqlThread( LPVOID lpVoid )
{
	CBEOPClinetSampleDlg* pUpdateValueHandle = reinterpret_cast<CBEOPClinetSampleDlg*>(lpVoid);
	if (NULL == pUpdateValueHandle)
	{
		return 0;
	}

	while(!pUpdateValueHandle->m_bExitThread)
	{		
		pUpdateValueHandle->UpdateMysqlValue();
		int nSleep = pUpdateValueHandle->m_nUpdateInterval;
		while(!pUpdateValueHandle->m_bExitThread)
		{
			nSleep--;
			if(nSleep < 0)
				break;
			Sleep(1000);
		}
	}
	return 1;
}

bool CBEOPClinetSampleDlg::UpdateMysqlValue()
{
	vector<pair<wstring, wstring> >	valuesets;
	GetSampleValueSets(valuesets);
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		return m_pDataHandle->UpdateOutPutValue(valuesets);
	}
	return false;
}

void CBEOPClinetSampleDlg::GetSampleValueSets( vector<pair<wstring, wstring> >& opcvaluesets )
{
	
}

bool CBEOPClinetSampleDlg::OutPutLogString( wstring strLog )
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\log";
	if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{
		char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
		setlocale( LC_ALL, "chs" );

		CString strOut;
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		strOut.Format(_T("%s ::%s\n"),strTime.c_str(),strLog.c_str());

		CString strLogPath;
		strLogPath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),SAMPLE_LOG_NAME);			//edit

		//��¼Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strOut);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
		setlocale( LC_CTYPE, old_locale ); 
		free( old_locale ); 
	}
	return false;
}

void CBEOPClinetSampleDlg::OnCloseClient()
{
	if(CloseSampleClient())
	{
		m_bCloseDlg = true;
		SendMessage(WM_CLOSE);
	}
}

bool CBEOPClinetSampleDlg::CloseSampleClient()
{
	return Exit();
}

bool CBEOPClinetSampleDlg::Exit()
{
	m_bExitThread = true;
	if(m_hupdatehandle)
		WaitForSingleObject(m_hupdatehandle, 2000);
	if(m_hupdatehandle)
	{
		CloseHandle(m_hupdatehandle);
		m_hupdatehandle = NULL;
	}
	if(m_pDataHandle)
	{
		delete m_pDataHandle;
		m_pDataHandle = NULL;
	}
	return true;
}