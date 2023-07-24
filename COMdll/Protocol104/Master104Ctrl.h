#pragma once

#include "TCPSocket.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"

#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

#define WM_STATUS_CHANGED WM_USER+5
#define WM_DATA_ARRIVED WM_USER+6

#include "DB_BasicIO/RealTimeDataAccess.h"

//Frame֡����
enum TCP_FRAME_TYPE
{
	PROTOCOL_TYPEID64H =0,       //���ٻ�
	PROTOCOL_TYPEID67H,			//��վ��ʱ  
	PROTOCOL_TYPEID2DH,    //����ң�� 
	PROTOCOL_TYPEID2EH,    //˫��ң��   
	PROTOCOL_TYPEID65H,	//�����Ƽ���������(BCR) -- ����ۼ���   
	PROTOCOL_TYPEID01H,   //������YX���ģ�
	PROTOCOL_TYPEID0DH      //�θ���ң��
};

class __declspec(dllexport) CMaster104Tcp : public CTCPSocket
{
public:
	CMaster104Tcp(void);
	virtual ~CMaster104Tcp(void);

protected:
	
	//����ͨѶ�����İ�.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:

	//״̬�ص�
	static LRESULT OnStatusChange(TCP_CONNECT_STATE type,char *data,int length,DWORD userdata);

	void	SetHost(const string& strhost);
	void	SetPort(u_short portnumer);

	//Initialize Modbus Tcp Info
	bool	Init();
	bool    Exit();

	bool    Reconnect();

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	DataPointEntry*	FindEntry(int nFrameType, int nPointID);

	//�Ӱ����֡����
	void	GetFrameFromPackage(BYTE *msgbuf, int len);

	double	GetMultiple(wstring strMultiple);


	//֡����
	void UnPack_Start68H(BYTE *msgbuf, int len);   //
	void UnPack_TypeID64H(BYTE *msgbuf,int len );		//���ٻ� 
	void UnPack_TypeID67H(BYTE *msgbuf,int len );     //��վ��ʱ   
	void UnPack_TypeID2DH(BYTE *msgbuf,int len );     //����ң��   
	void UnPack_TypeID2EH(BYTE *msgbuf,int len );     //˫��ң��   
	void UnPack_TypeID65H(BYTE *msgbuf,int len );    //�����Ƽ���������(BCR) -- ����ۼ���   
	void UnPack_TypeID01H(BYTE *msgbuf,int len );    //������YX���ģ�
	void UnPack_TypeID0DH(BYTE *msgbuf,int len );    //�θ���ң��
	void UnPack_Start68H_Unknow(BYTE *msgbuf,int len ); 
	
	void	SendPack_TypeID64H();					//�������ٻ�

	static UINT WINAPI DTUThreadSendFunc(LPVOID lparam);

	int		GetSendInterval();
	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	EnableLog(BOOL bEnable);
	void	SetIndex(int nIndex);
private:
	Mutex						m_mutex;
	string	m_host;
	u_short	m_port;
	int		m_nSendInterval;
	HANDLE	m_hsendthread;
	vector<DataPointEntry> m_pointlist;	//���.
	Project::Tools::Mutex	m_lock;
	bool m_bExitThread;
	
	BOOL m_bLog;
	Beopdatalink::CLogDBAccess* m_logsession;

	int m_IRPacketNum;//I֡������Ŀ
	int m_ISPacketNum;//I֡������Ŀ
	BYTE m_IPacket[16];//I֡����

	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
};