// TcpIpComm.cpp The implement of the TcpIpComm.h file
#pragma once
#include "stdafx.h"
#include "TcpIpComm.h"
#include "assert.h"
#include "Tools/CustomTools/CustomTools.h"

#pragma warning(disable:4267)

//=====================================================================
// The receiving thread.
UINT WINAPI CTcpIpComm::RecvThread(LPVOID lpVoid)
//=====================================================================
{
	CTcpIpComm* tcpIpComm = reinterpret_cast<CTcpIpComm*>(lpVoid);
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

	//golding add 2014-08-12, ��ֹ�߳��˲�����
	int timeout = 2000;
	setsockopt(sockConnected, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));


	if (sockConnected != NULL)
	{
		while(true)
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
				//tcpIpComm->m_bConnectOK = false;
				int nLastErrorCode =  WSAGetLastError(); //.10060��WSAETIMEDOUT
				CString strInfo;

				strInfo.Format(_T("TcpIpComm recv failed with error: %d.  \r\n"), WSAGetLastError());
				_tprintf(strInfo.GetString());

				if(nLastErrorCode==10054) //time out drop
				{
					Sleep(5000);
				}

				Sleep(5000);
				/*tcpIpComm->CloseSocket();
				tcpIpComm->CloseThreadHandle();
				tcpIpComm->InitParams();
				return 0;*/
			}

			Sleep(200);
		}
	}

	return 1;
}

//=====================================================================
// Default consructor.
//=====================================================================
CTcpIpComm::CTcpIpComm() : m_nSendSize(0)
{
	// Initialize the parameters.
	ZeroMemory(&m_SendBuffer, sizeof(m_SendBuffer));
	InitParams();
}

//=====================================================================
// Default destructor.
//=====================================================================
CTcpIpComm:: ~CTcpIpComm()
{
	m_bConnectOK = false;
	// Clear the objects.
	if (m_sockConnected != NULL)
	{
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
void CTcpIpComm::InitParams()
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
bool CTcpIpComm::TcpIpConnectComm(const string& strIp, u_short uPort)
{
	SOCKADDR_IN sockAddrIn;
	memset(&sockAddrIn, 0, sizeof(SOCKADDR_IN));

	// Config the socket address information.
	sockAddrIn.sin_family = AF_INET;
	sockAddrIn.sin_addr.s_addr = inet_addr(const_cast<char*>(strIp.c_str()));
	sockAddrIn.sin_port = htons(uPort);

	// Create the listener socket.
	m_sockConnected = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sockConnected == SOCKET_ERROR)
	{
		TRACE("Create the listener socket failed with error: %d.  \r\n", WSAGetLastError());
		closesocket(m_sockConnected);
		m_bConnectOK = false;
		return false;
	}

	// Connect to the assign ip address.
	if (connect(m_sockConnected, (sockaddr*)&sockAddrIn, sizeof(sockAddrIn)) == SOCKET_ERROR)
	{
		// Connect again.
		closesocket(m_sockConnected);
		_tprintf(L"ERROR: TcpIpComm Cannot connect to %s:%d...\r\n",Project::Tools::AnsiToWideChar(strIp.c_str()).c_str(),uPort);
		m_sockConnected = NULL;
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
bool CTcpIpComm::SendPackage(const char *pSrcPackage, unsigned int nSrcPackSize, int &nErrCode)
{
	nErrCode = 0;
	BOOL bTcpNoDelay = TRUE;
	DWORD dwTimeout = 2000;

	if (nSrcPackSize != 0)
	{
		//Set TCP/IP NODELAY
		setsockopt(m_sockConnected, IPPROTO_TCP, TCP_NODELAY, (char*)&bTcpNoDelay, sizeof(bTcpNoDelay));
		// Set the send timeout.
		setsockopt(m_sockConnected, SOL_SOCKET, SO_SNDTIMEO, (char*)&dwTimeout, sizeof(dwTimeout));

		// Send datas.
		CString strLog;
		//wstring wstrPackage;
		//wstrPackage = Project::Tools::UTF8ToWideChar(pSrcPackage);
		//strLog.Format(_T("%s TcpIpComm SendPackage Once(size:%d).\r\n"), COleDateTime::GetCurrentTime().Format(_T("%H:%M:%S")), nSrcPackSize);
		//_tprintf(strLog.GetString());
		if (send(m_sockConnected, pSrcPackage, nSrcPackSize/*+1*/, 0) == SOCKET_ERROR)
		{
			nErrCode = GetLastError();//10054��ʾ�������
			CString strErrInfo;
			strErrInfo.Format(_T("ERROR: TcpIpComm Send datas failed. Code:%d  \r\n"), nErrCode);

 			_tprintf(strErrInfo.GetString());
			Sleep(5000);
			return false;
		}

		//TRACE("Send datas:%s.  \r\n", pSrcPackage);
	}

	return true;
}

//=====================================================================
// Get the send package buffer.
//=====================================================================
void CTcpIpComm::GetSendBuffer(char* pszSendBuffer, unsigned int& nSendSize)
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
void CTcpIpComm::SetSendBuffer(const char *pSendBuff, unsigned int nSendSize)
{
	assert(strcmp(pSendBuff, "") != 0 && nSendSize != 0);

	memcpy(m_SendBuffer, pSendBuff, nSendSize);
	m_nSendSize = nSendSize;
}

//=====================================================================
// Get the connected socket.
//=====================================================================
SOCKET CTcpIpComm::GetConnectedSocket() const
{
	return m_sockConnected;
}

//=====================================================================
// Set the connected socket.
//=====================================================================
void CTcpIpComm::SetConnectedSocket(SOCKET sockConnected)
{
	m_sockConnected = sockConnected;
}

void CTcpIpComm::Disconnect()
{
	m_bConnectOK = false;
	SetEvent(m_hEventClose);
	WaitForSingleObject(m_hthreadhandle, INFINITE);

	CloseSocket();
	CloseThreadHandle();
}

void CTcpIpComm::CloseSocket()
{
	if (m_sockConnected != NULL)
	{
		closesocket(m_sockConnected);
		m_sockConnected = NULL;
	}
}

void CTcpIpComm::CloseThreadHandle()
{
	if (m_hthreadhandle != NULL){
		CloseHandle(m_hthreadhandle);
		m_hthreadhandle = NULL;
	}
}
