//_ **********************************************************
//_ 
//_ Name: SerialPort.h : header file
//_ brief	: Serial Port communication
//			
//_ Purpose: 
//_ Author: Zhu Huawei  z_hw66@sohu.com
//-
//_ Copyright (c) 2009 
//_ 
//_ **********************************************************
#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__
#pragma  once

#include "../ComInterface.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#define MAX_REC_COUNT	8192

#define WM_COMM_BREAK_DETECTED		WM_USER+1	// A break was detected on input.
#define WM_COMM_CTS_DETECTED		WM_USER+2	// The CTS (clear-to-send) signal changed state. 
#define WM_COMM_DSR_DETECTED		WM_USER+3	// The DSR (data-set-ready) signal changed state. 
#define WM_COMM_ERR_DETECTED		WM_USER+4	// A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define WM_COMM_RING_DETECTED		WM_USER+5	// A ring indicator was detected. 
#define WM_COMM_RLSD_DETECTED		WM_USER+6	// The RLSD (receive-line-signal-detect) signal changed state. 
#define WM_COMM_RXCHAR				WM_USER+7	// A character was received and placed in the input buffer. 
#define WM_COMM_RXFLAG_DETECTED		WM_USER+8	// The event character was received and placed in the input buffer.  
#define WM_COMM_TXEMPTY_DETECTED	WM_USER+9	// The last character in the output buffer was sent.  

typedef LRESULT (*LPRecDataProc)(const unsigned char* pRevData, DWORD length,DWORD userdata); //�������ݵĻص�����  add by robert 12.11

class CSerialPort:public CComBase
{
public:
	// contruction and destruction
	CSerialPort();
	virtual	~CSerialPort();

protected:
	// protected memberfunctions
	void ProcessErrorMessage(char* ErrorText);
	//void ProcessErrorMessage(CString strErrorText);
	static UINT CommThread(LPVOID pParam);
	static void	ReceiveChar(CSerialPort *port, COMSTAT comsta);
	static bool	WriteChar(CSerialPort *port);
public:
	/*BOOL InitPort(CWnd *pPortOwner, UINT portnr = 1, UINT baud = 19200, char parity = 'E', UINT databits = 8
		, UINT stopbits = 1, DWORD dwCommEvents = EV_RXCHAR, UINT writebuffersize = 1024);*/
	BOOL InitPort(UINT portnr = 1, UINT baud = 19200, char parity = 'N', UINT databits = 8
		, UINT stopbits = 1, DWORD dwCommEvents = EV_RXCHAR, UINT writebuffersize = 1024);

	void ClosePort();

	bool	SetReceiveEvent(bool bReceived);			//�����Ƿ����������
	bool	GetReceiveEvent();

	// start/stop comm watching
	BOOL	StartMonitoring(LPRecDataProc proc1=NULL,DWORD userdata=NULL);
	BOOL	RestartMonitoring();
	BOOL	StopMonitoring();

	DWORD	GetWriteBufferSize();
	DWORD	GetCommEvents();
	DCB		GetDCB();

	bool	WriteToPort(char *pData);
	bool	WriteToPort(char *pData,int nLen);
	bool	WriteToPortByWrap(char *pData,int nLen);			//�Զ���ӻ��з�
	bool	WriteToPort(LPCTSTR string);
	bool	WriteToPort(LPCTSTR string, int nLen);

	BOOL	RecvData(LPTSTR lpszData, const int nSize);      //���ڽ��պ���
	bool	StoreRecvData(char *pData,int nLen);

	std::string	GetSuccessBuffer();

	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);
	bool	UpdateDTUWriteTime();
	bool	GetDTUIdleState(int nSeconds = 4);				//������ʱ����4s,����ʾ����,�����ڲ�����ʷ����

	void	SetHistoryDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
private:

	LPRecDataProc	m_lpRecDataProc;	//�յ����ݻظ��ص�
	DWORD m_dwUserData; //�û�����
	// thread
	HANDLE   m_hThread;

	// Event array. 
	// One element is used for each event. There are two event handles for each port.
	// A Write event and a receive character event which is located in the overlapped structure (m_ov.hEvent).
	// There is a general shutdown when the port is closed. 
	HANDLE		m_hEventArray[3];
	// handles
	HANDLE		m_hWriteEvent;
	HANDLE		m_hShutdownEvent;

	HANDLE		m_hRecEvent;		//�յ������¼�

	// synchronization objects
	CRITICAL_SECTION m_csCommunicationSync;			//�ٽ������� ����������Դ
	BOOL			 m_bThreadAlive;
	bool			m_bReceived;					//�����ַ���

	// structures
	OVERLAPPED		 m_ov;					//�첽�����������Ϣ�Ľṹ��
	COMMTIMEOUTS	 m_CommTimeouts;		//��ʱ����
	DCB				 m_dcb;					//�豸���ƿ�(Device Control Block)�ṹ��ַ

	// owner window
	CWnd*				m_pOwner;

	// misc
	UINT			 m_nPortNr;
	DWORD			 m_dwCommEvents;
	DWORD			 m_nWriteBufferSize;
	char			 *m_szWriteBuffer;
	int				 m_nWriteSize; 
	
	std::string			m_strSendSuccessBuffer;			//���һ�γɹ����͵��ַ���
	COleDateTime		m_oleLastWriteTime;			//��һ��дDTUʱ��
	bool				m_bRecLog;					//��ʾlog
	int					m_nReadMode;				//0 ��1024���ȶ�  1�������
	bool				m_bCmdTerminator;			//0 ����'\0'��β  1::��'\0'��β
public:
	// port initialization											
	HANDLE			 m_hComm;
	unsigned char	m_cRXBuff[MAX_REC_COUNT];			//DTU�����ַ���
	int				m_nReceCount;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
};
#endif //__SERIALPORT_H__


