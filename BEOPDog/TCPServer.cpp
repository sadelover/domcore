// TCPServer.cpp: implementation of the CTCPServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TCPServer.h"
#include "TCPCustom.h"
//#include "../include/vld.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <afxtempl.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPtrList m_ListClientSocket;   

//���캯��
CTCPServer::CTCPServer()
{
	//�����߳��˳��¼����
	m_exitThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_bexitthread = false;
	m_serverCloseHandle = NULL;

	//�ͻ������ӽ����¼����ص�����   
	OnClientConnect = NULL;   
	//�ͻ������ӶϿ��¼����ص�����   
	OnClientClose = NULL;   
	//�ͻ��˽��������¼����ص�����   
	OnClientRead = NULL;   
	//�ͻ��˷��������¼����ص�����   
	OnClientError = NULL;   
	//�������˷��������¼�,�ص�����   
	OnServerError = NULL;   

	OnServerRestartTask = NULL;

	OnClientRegedit = NULL;
}

//��������
CTCPServer::~CTCPServer()
{
	//�ر��߳��˳��¼����
	m_bexitthread = true;
	CloseHandle(m_exitThreadEvent);
}

/*--------------------------------------------------------------------
���������ܡ�:  ���߳����ڼ������׽����¼���
����ڲ�����:  lparam:������ָ�룬����ͨ���˲��������߳��д�����Ҫ�õ�����Դ��
			   ���������ǽ�CTCPServer��ʵ��ָ�봫����
�����ڲ�����:  (��)
������  ֵ��:  ����ֵû���ر�����壬�ڴ����ǽ�����ֵ��Ϊ0��
---------------------------------------------------------------------*/
DWORD WINAPI CTCPServer::SocketThreadFunc(PVOID lparam)
{
	CTCPServer *pSocket;
	//�õ�CTCPServerʵ��ָ��
	pSocket = (CTCPServer*)lparam;
	//������¼�����
	fd_set fdRead;
	int ret;
	TIMEVAL	aTime;
	aTime.tv_sec = 1;
	aTime.tv_usec = 1;
	while (TRUE)
	{
        //�յ��˳��¼��������߳�
		if (WaitForSingleObject(pSocket->m_exitThreadEvent,0) == WAIT_OBJECT_0)
		{
			break;
		}
		
		FD_ZERO(&fdRead);
		FD_SET(pSocket->m_ServerSocket,&fdRead);
		
		ret = select(0,&fdRead,NULL,NULL,&aTime);
		
		if (ret == SOCKET_ERROR)
		{
			//���������¼�
			int iErrorCode = WSAGetLastError();
			//����������socket�Ĵ����¼�
			pSocket->OnServerError(pSocket->m_pOwnerWnd,pSocket,iErrorCode);
			//�رշ������׽��� 
			closesocket(pSocket->m_ServerSocket);
			break;
		}
		
		if (ret > 0)
		{
			//�ж��Ƿ���¼�
			if (FD_ISSET(pSocket->m_ServerSocket,&fdRead))
			{
				//���������Listen�����ʾ������OnAccept�¼�
				
				SOCKADDR_IN clientAddr;
				CTCPCustom * pClientSocket = new CTCPCustom();
				int namelen = sizeof(clientAddr);
				//�ȴ���������ͻ������ӵ��׽���
				pClientSocket->m_socket = accept(pSocket->m_ServerSocket, (struct sockaddr *)&clientAddr, &namelen);
				//���յ��ͻ�������
				if (pClientSocket->m_socket)
				{
					pClientSocket->m_RemoteHost = inet_ntoa(clientAddr.sin_addr);
					pClientSocket->m_RemotePort = ntohs(clientAddr.sin_port);
					
					//������ͻ��˽��������¼�   
					if (pSocket->OnClientConnect)   
					{   
						pSocket->OnClientConnect(pSocket->m_pOwner,pClientSocket);   
					}   
					//��pClientSocket�����߳�   
					pClientSocket->Open(pSocket);   
					//��ӵ��ͻ������Ӷ�����   
					m_ListClientSocket.AddTail(pClientSocket);   
				}   
				else   
				{   
					//ʧ�ܣ��ͷ��ڴ�   
					delete pClientSocket;   
					pClientSocket = NULL;   
				}      
			}
		}
	}
	//   
	TRACE(L"���������߳��˳�\n");   
	return 0;
}

/*--------------------------------------------------------------------
���������ܡ�:  ��TCP����
����ڲ�����:  (��)
�����ڲ�����:  (��)
������  ֵ��:  <=0:��TCP����ʧ��; =1:��TCP����ɹ�
---------------------------------------------------------------------*/
int CTCPServer::Open()
{
	WSADATA wsa;
	
	//1.��ʼ��socket��Դ
	if (WSAStartup(MAKEWORD(1,1),&wsa) != 0)
	{
		return -1;//����ʧ��
	}
	
	//2.���������׽���
	if ((m_ServerSocket=socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		return -2;
	}
	
    SOCKADDR_IN  serverAddr;
	ZeroMemory((char *)&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(m_LocalPort);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//3.�󶨼����׽���
	if (bind(m_ServerSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr))<0)
	{
		
		return -3 ;
	}
	//4.�����׽��ֿ�ʼ����
	if (listen(m_ServerSocket,8)!=0)
	{
		return -3;
	}
	
	//4.���ü����׽���ͨѶģʽΪ�첽ģʽ
	DWORD ul= 1;
	ioctlsocket(m_ServerSocket,FIONBIO,&ul);

	ResetEvent(m_exitThreadEvent);
	m_bexitthread = false;
	//5.����ͨѶ�̣߳����߳���ȴ��ͻ��˽���
	m_serverThreadHandle = CreateThread(NULL,0,SocketThreadFunc,this,0,NULL);
	m_serverCloseHandle = CreateThread(NULL,0,CloseThreadFunc,this,0,NULL);
	if (m_serverThreadHandle == NULL)
	{
		closesocket(m_ServerSocket);
		return -1;
	}

	return 1;
}


/*--------------------------------------------------------------------
���������ܡ�:  �ر�TCP����
����ڲ�����:  (��)
�����ڲ�����:  (��)
������  ֵ��:  <=0:�ر�TCP����ʧ��; =1:�ر�TCP����ɹ�
---------------------------------------------------------------------*/
int CTCPServer::Close()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	//����ͨѶ�߳�   
	m_bexitthread = true;
	SetEvent(m_exitThreadEvent);   
	//�ȴ�1�룬������߳�û���˳�����ǿ���˳�   
	if (WaitForSingleObject(m_serverThreadHandle,2000) == WAIT_TIMEOUT)   
	{   
		TerminateThread(m_serverThreadHandle,0);   
		TRACE(L"ǿ����ֹ���������߳�\n");   
	}   
	m_serverThreadHandle = NULL;   

	if(m_serverCloseHandle)
	{
		CloseHandle(m_serverCloseHandle);
		m_serverCloseHandle = NULL;
	}
	//���ȣ��ر������пͻ�������   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   
	while (pos != NULL)   
	{   
		//�õ��ͻ��˶���   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos);   
		if (!pTcpCustom->Close())   
		{   
			TRACE(L"�رտͻ���socket����");   
		}   

		//�ͷ��ڴ�   
		delete pTcpCustom;   
		pTcpCustom = NULL;   
	}   
	m_ListClientSocket.RemoveAll();  
	//�ر�Socket���ͷ���Դ   
	int err = closesocket(m_ServerSocket);   
	if (err == SOCKET_ERROR)   
	{   
		return -1;   
	}    
	WSACleanup();   
	return 1;  
}  

void CTCPServer::RemoteClient( CTCPCustom *pClient )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	POSITION posPrior;   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   

	while (pos != NULL)   
	{   
		posPrior = pos;   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos);   
		if (pTcpCustom != NULL && pTcpCustom == pClient)   
		{   
			if (!pTcpCustom->Close())   
			{   
				TRACE(L"closesocket err\n");   
			} 

			//�ͷ��ڴ�   
			delete pTcpCustom;   
			pTcpCustom = NULL;   
			m_ListClientSocket.RemoveAt(posPrior); 
			TRACE(L"RemoteClient\n");   
			break; 
		}   
	}   
}

DWORD WINAPI CTCPServer::CloseThreadFunc( PVOID lparam )
{
	CTCPServer* pthis = (CTCPServer*)lparam;
	if (pthis != NULL)
	{
		while(!pthis->GetExitThread())
		{
			pthis->RemoteInactiveClient();	
			Sleep(2000);	
		}
	}

	return 0;
}

void CTCPServer::RemoteInactiveClient()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	POSITION posPrior;   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   
	while (pos != NULL)   
	{   
		posPrior = pos;   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos); 
		if(pTcpCustom)
		{
			if(pTcpCustom->IsTimeOut())
			{
				CString strOut;
				strOut.Format(_T("RemoteinactiveClient(%s)\n"),pTcpCustom->m_RemoteHost);
				//������ͻ��˶����ӵ�Socket�ر��¼�   
				if (pTcpCustom->m_pTCPServer->OnClientClose)   
				{   
					pTcpCustom->m_pTCPServer->OnClientClose(pTcpCustom->m_pTCPServer->m_pOwner,pTcpCustom);   
				}  

				if (!pTcpCustom->Close())   
				{   
					TRACE(L"closesocket inactive err\n");  
				}   
				//�ͷ��ڴ�   
				delete pTcpCustom;   
				pTcpCustom = NULL;   
				m_ListClientSocket.RemoveAt(posPrior); 
				TRACE(strOut.GetString());   
			}
		}
	}   
}

bool CTCPServer::GetExitThread() const
{
	return m_bexitthread;
}

CTCPCustom* CTCPServer::FindClient( CString strRemoteHost,DWORD nPort )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	POSITION posPrior;   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   
	while (pos != NULL)   
	{   
		posPrior = pos;   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos); 
		if(pTcpCustom)
		{
			if(pTcpCustom->m_RemoteHost == strRemoteHost && pTcpCustom->m_RemotePort == nPort)
			{
				return pTcpCustom;
			}
		}
	}
	return NULL;
}

bool CTCPServer::GetAllClientConnectInfo( vector<CString> &vecConnectInfo )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	vecConnectInfo.clear();
	POSITION posPrior;   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   
	while (pos != NULL)   
	{   
		posPrior = pos;   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos); 
		if(pTcpCustom)
		{
			CString strConnectInfo;
			strConnectInfo.Format(_T("%s:%d...\n"),pTcpCustom->m_RemoteHost,pTcpCustom->m_RemotePort);
			vecConnectInfo.push_back(strConnectInfo);
		}
	}
	return true;
}

bool CTCPServer::AckRestart( E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult )
{
	switch(eReason)
	{
		case E_REASON_SUBMIT_CORE:		//core�����ύ��
		case E_REASON_SUBMIT_LOGIC:		//logic�����ύ��
		case E_REASON_SUBMIT_UPDATE:	//update�����ύ��
			{
				Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
				POSITION posPrior;   
				POSITION pos = m_ListClientSocket.GetHeadPosition();   
				while (pos != NULL)   
				{   
					posPrior = pos;   
					CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos); 
					if(pTcpCustom && pTcpCustom->m_bOK)
					{
						if(pTcpCustom->m_eType == eReason)
						{
							return pTcpCustom->SendAck(eType,eReason,bResult);
						}
					}
				}
				return false;
			}
		default:
			return false;
	}
	return false;
}
