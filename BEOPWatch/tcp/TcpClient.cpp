// TcpIpComm.cpp The implement of the TcpIpComm.h file
#pragma once
#include "stdafx.h"
#include "TcpClient.h"
#include "assert.h"
#pragma warning(disable:4267)

//=====================================================================
// The receiving thread.
UINT WINAPI CTcpClient::RecvThread(LPVOID lpVoid)
//=====================================================================
{
	CTcpClient* tcpIpComm = reinterpret_cast<CTcpClient*>(lpVoid);
	if (NULL == tcpIpComm)
	{
		return 0;
	}
	assert(tcpIpComm);

	SOCKET sockConnected = NULL;
	char szBuff[MAX_RECEIVED_BUFFER_SIZE] = {0};
	
	ResetEvent(tcpIpComm->m_hEventClose);
	// Get the accept socket.
	sockConnected = tcpIpComm->GetConnectedSocket();

	if (sockConnected != NULL)
	{
		while(1)
		{
			if (WaitForSingleObject(tcpIpComm->m_hEventClose,0) == WAIT_OBJECT_0) //�߳��˳���ʽ
				break;

			// Receive datas
			memset(szBuff, 0, MAX_RECEIVED_BUFFER_SIZE);
			int nLen = recv(sockConnected, szBuff, MAX_RECEIVED_BUFFER_SIZE, 0);
			if (nLen > 0)
			{
				//trace("Receive datas:%s.  \r\n", szBuff);
				// Received datas.
				tcpIpComm->OnCommunication((unsigned char*)szBuff, nLen);
			}
			else if (nLen == 0)
			{
				// The server connected closed.
				tcpIpComm->m_bConnectOK = false;
				TRACE("Connection closed.  \r\n");
				return 0;
			}
			else
			{
				// Received failed.
				tcpIpComm->m_bConnectOK = false;
				TRACE("recv failed with error: %d.  \r\n", WSAGetLastError());
			}
		}
	}

	return 1;
}

//=====================================================================
// Default consructor.
//=====================================================================
CTcpClient::CTcpClient() : m_nSendSize(0)
{
	// Initialize the parameters.
	ZeroMemory(&m_SendBuffer, sizeof(m_SendBuffer));
	InitParams();
}

//=====================================================================
// Default destructor.
//=====================================================================
CTcpClient:: ~CTcpClient()
{
	m_bConnectOK = false;
	// Clear the objects.
	if (m_sockConnected != NULL)
	{
		shutdown(m_sockConnected, SD_BOTH);
		closesocket(m_sockConnected);
		m_sockConnected = NULL;
	}

	// Clear up the winsock2.
	WSACleanup();

	if (m_hthreadhandle != NULL){
		CloseHandle(m_hthreadhandle);
		m_hthreadhandle = NULL;
	}
}

//=====================================================================
// Initialize the parameters and objects.
//=====================================================================
void CTcpClient::InitParams()
{
	m_sockConnected = NULL;
	m_hthreadhandle = NULL;
	m_bConnectOK = false;

	m_hEventClose   = CreateEvent(NULL,TRUE,FALSE,NULL);

	// Initialize the winsock2.
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != NO_ERROR)
	{
		TRACE("Winsock 2 initialize failed with error: %d.  \r\n", nResult);
		return ;
	}
}

//=====================================================================
// Initialize the tcp ip connect communication.
//=====================================================================
bool CTcpClient::TcpIpConnectComm(const string& strIp, u_short uPort)
{
	SOCKADDR_IN sockAddrIn;
	memset(&sockAddrIn, 0, sizeof(SOCKADDR_IN));

	if(isalpha(strIp[0]))
	{
		//��������
		LPHOSTENT lpHost=gethostbyname(strIp.c_str());
		if(lpHost==NULL)
		{
			return FALSE;
		}

		sockAddrIn.sin_family=AF_INET;
		sockAddrIn.sin_addr.s_addr=*((u_long FAR*)(lpHost->h_addr));
		sockAddrIn.sin_port=htons(uPort);
	}
	else
	{
		// Config the socket address information.
		sockAddrIn.sin_family = AF_INET;
		sockAddrIn.sin_addr.s_addr = inet_addr(const_cast<char*>(strIp.c_str()));
		sockAddrIn.sin_port = htons(uPort);
	}

	// Create the listener socket.
	m_sockConnected = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sockConnected == SOCKET_ERROR)
	{
		TRACE("Create the listener socket failed with error: %d.  \r\n", WSAGetLastError());
		shutdown(m_sockConnected, SD_BOTH);
		closesocket(m_sockConnected);
		m_bConnectOK = false;
		return false;
	}

	//�������ñ��ص�ַ�Ͷ˿�:�����ĺô��ǣ���ʹsocket���ˣ�����ǰ���socket����Ҳ����ռ����һ��������ʼ�վ���һ���˿�
	int nREUSEADDR = 1;
	setsockopt(m_sockConnected,SOL_SOCKET,SO_REUSEADDR,(const char *)&nREUSEADDR,sizeof (int));
	// closesocket ���ý��롰������״̬���ȴ���ɣ��������Ƿ����Ŷ�����δ���ͻ�δ��ȷ�ϡ����ֹرշ�ʽ��Ϊ��ǿ�йرա�
	linger m_sLinger;
	m_sLinger.l_onoff = 1;  // ( ��closesocket()����,���ǻ�������û������ϵ�ʱ��������)
	m_sLinger.l_linger = 0; // ( ��������ʱ��Ϊ0��)
	setsockopt(m_sockConnected,SOL_SOCKET,SO_LINGER,(const char *)&m_sLinger,sizeof (linger));
	
	//���÷�������ʽ����
	unsigned long ul = 1;
	int  ret = ioctlsocket(m_sockConnected, FIONBIO, (unsigned long*)&ul);
	if(ret==SOCKET_ERROR)
	{
		TRACE("set the listener socket failed with error: %d.  \r\n", WSAGetLastError());
		shutdown(m_sockConnected, SD_BOTH);
		closesocket(m_sockConnected);
		m_bConnectOK = false;
		return false;
	}

	// Connect to the assign ip address.
	connect(m_sockConnected, (sockaddr*)&sockAddrIn, sizeof(sockAddrIn));

	//select ģ�ͣ������ó�ʱ
	struct timeval timeout ;
	fd_set r;

	FD_ZERO(&r);
	FD_SET(m_sockConnected, &r);
	timeout.tv_sec = 5; //���ӳ�ʱ10��
	timeout.tv_usec =0;
	ret = select(0, 0, &r, 0, &timeout);
	if ( ret <= 0 )
	{
		TRACE("connect time out: %d.  \r\n", WSAGetLastError());
		shutdown(m_sockConnected, SD_BOTH);
		closesocket(m_sockConnected);
		m_bConnectOK = false;
		return false;
	}

	//�������ģʽ
	unsigned long ul1= 0 ;
	ret = ioctlsocket(m_sockConnected, FIONBIO, (unsigned long*)&ul1);
	if(ret==SOCKET_ERROR)
	{
		TRACE("set the listener socket failed with error: %d.  \r\n", WSAGetLastError());
		shutdown(m_sockConnected, SD_BOTH);
		closesocket(m_sockConnected);
		m_bConnectOK = false;
		return false;
	}

	// Create the send/receive datas worker threads.
    m_hthreadhandle = (HANDLE)_beginthreadex(NULL, 0, RecvThread, this, 0, NULL);
	m_bConnectOK = true;
	return true;
}

//=====================================================================
// Send package.
//=====================================================================
bool CTcpClient::SendPackage(const char *pSrcPackage, unsigned int nSrcPackSize)
{
	if(!m_bConnectOK)
		return false;

	BOOL bTcpNoDelay = TRUE;
	DWORD dwTimeout = 2000;

	if (nSrcPackSize != 0)
	{
		//Set TCP/IP NODELAY
		setsockopt(m_sockConnected, IPPROTO_TCP, TCP_NODELAY, (char*)&bTcpNoDelay, sizeof(bTcpNoDelay));
		// Set the send timeout.
		setsockopt(m_sockConnected, SOL_SOCKET, SO_SNDTIMEO, (char*)&dwTimeout, sizeof(dwTimeout));

		// Send datas.
		if (send(m_sockConnected, pSrcPackage, nSrcPackSize/*+1*/, 0) == SOCKET_ERROR)
		{
 			_tprintf(L"ERROR: TcpIpComm Send datas failed.  \r\n");
			return false;
		}
	}

	return true;
}

//=====================================================================
// Get the send package buffer.
//=====================================================================
void CTcpClient::GetSendBuffer(char* pszSendBuffer, unsigned int& nSendSize)
{
	if (strcmp(m_SendBuffer, "") != 0 && m_nSendSize != 0)
	{
		memcpy(pszSendBuffer, m_SendBuffer, m_nSendSize);
		nSendSize = m_nSendSize;
	}
}

//=====================================================================
// Set the send package buffer.
//=====================================================================
void CTcpClient::SetSendBuffer(const char *pSendBuff, unsigned int nSendSize)
{
	assert(strcmp(pSendBuff, "") != 0 && nSendSize != 0);

	memcpy(m_SendBuffer, pSendBuff, nSendSize);
	m_nSendSize = nSendSize;
}

//=====================================================================
// Get the connected socket.
//=====================================================================
SOCKET CTcpClient::GetConnectedSocket() const
{
	return m_sockConnected;
}

//=====================================================================
// Set the connected socket.
//=====================================================================
void CTcpClient::SetConnectedSocket(SOCKET sockConnected)
{
	m_sockConnected = sockConnected;
}

void CTcpClient::Disconnect()
{
	m_bConnectOK = false;
	SetEvent(m_hEventClose);
	WaitForSingleObject(m_hthreadhandle, 2000);

	CloseSocket();
	CloseThreadHandle();
}

void CTcpClient::CloseSocket()
{
	if (m_sockConnected != NULL)
	{
		shutdown(m_sockConnected, SD_BOTH);
		closesocket(m_sockConnected);
		m_sockConnected = NULL;
	}
}

void CTcpClient::CloseThreadHandle()
{
	if (m_hthreadhandle != NULL){
		CloseHandle(m_hthreadhandle);
		m_hthreadhandle = NULL;
	}
}
