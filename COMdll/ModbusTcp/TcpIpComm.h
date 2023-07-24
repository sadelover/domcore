#ifndef __TCP_IP_COMMUNICATION_H__
#define __TCP_IP_COMMUNICATION_H__
#pragma once

#include "../ComInterface.h"

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include "mswsock.h"
#include <process.h>
#include <string>
using namespace std;

#define _MDTCP_CMD_LENTH_		  0x0C     //ModbusTcpЭ�鳤��
#define MAX_RECEIVED_BUFFER_SIZE  8192

class CTcpIpComm:public  CComBase
{
public: 
	CTcpIpComm();             	 						          // default consructor
	virtual ~CTcpIpComm();    	 						          // default destructor

protected:
	void InitParams();                                           // initialize the parameters and objects
	bool TcpIpConnectComm(const string& strIp, u_short uPort);   // initialize the tcp ip connect communication

public:
	bool            SendPackage(const char *pSrcPackage, unsigned int nSrcPackSize, int &nErrCode);   // send package	
	inline void     GetSendBuffer(char* pszSendBuffer, unsigned int& nSendSize);// get the send package buffer
	void            SetSendBuffer(const char *pSendBuff, unsigned int nSendSize);      // set the send package buffer

	inline SOCKET   GetConnectedSocket() const;                    // get the connected socket
	inline void     SetConnectedSocket(SOCKET sockConnected);        // set the connected socket

    static UINT WINAPI RecvThread(LPVOID lpVoid);

	void	Disconnect();

	void	CloseSocket();
	void	CloseThreadHandle();
protected:
	SOCKET          m_sockConnected;      	 						   	 // the connected socket
	char            m_SendBuffer[_MDTCP_CMD_LENTH_];      	       		 // the send package buffer
	unsigned int    m_nSendSize;                                    // the send package size
	bool			m_bConnectOK;	
	HANDLE			m_hthreadhandle;
	HANDLE			m_hEventClose;
};

#endif  // __TCP_IP_COMMUNICATION_H__