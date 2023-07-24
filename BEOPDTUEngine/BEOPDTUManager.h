#pragma once
#include "Tools/dtu/BEOPDTUCtrl.h"
class CBEOPDTUManager
{
public:
	CBEOPDTUManager(CDataHandler* pDataHandler);
	~CBEOPDTUManager();

	bool	Init(int nTimeOut=0);					//��ʼ������
	bool	Exit();									//�˳�����
	bool	GetExitThread();						//��ȡ�˳����
	bool	CheckTimeOut(int& nTimeOut);			//����Ƿ�ʱ 0:����ʱ  1:SendTimeOut  10:ReceiveTimeOut 100:ConnectTimeOut

	static  unsigned int WINAPI ThreadConnectDTU(LPVOID lpVoid);	//DTU�����߳�
	static  unsigned int WINAPI ThreadSendDTU(LPVOID lpVoid);		//DTU�����߳�
	static  unsigned int WINAPI ThreadReceiveDTU(LPVOID lpVoid);	//DTU���մ����߳�
	
	bool	ConnectDTU();							//����DTU
	bool	SendDTU();								//���ͺ���
	bool	ReceiveDTU();							//���պ���
	
private:
	CDataHandler*	m_pDataHandler;					//���ݿ⴦����
	CBEOPDTUCtrl*	m_pBEOPDTUCtrl;					//DTU������
	HANDLE			m_hDTUConnect;					//DTU�����߳̾��
	HANDLE			m_hDTUSend;						//DTU�����߳̾��
	HANDLE			m_hDTUReceive;					//DTU�����߳̾��
	bool			m_bExitThread;					//�˳��̱߳��
	int				m_nTryCount;					//���Դ���
	COleDateTime	m_oleHeartTime;					//������ע��ʱ��

	COleDateTime	m_oleSend;						//����ʱ��
	COleDateTime	m_oleReceive;					//����ʱ��
	COleDateTime	m_oleConnect;					//����ʱ��
};
