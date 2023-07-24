// BEOPGatewayCore.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "BEOPGatewayCore.h"
#include "winsock.h"
#include <Winsvc.h>
#include "BeopGatewayCoreWrapper.h"
#include "CoreUnitTest.h"
#include "conio.h"
#include "redis/hiredis.h"

#pragma comment(lib, "advapi32")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//#pragma comment(linker,"/subsystem:windows")
//#pragma comment(linker,"/subsystem:\"Windows\" /entry:\"wmainCRTStartup\"")

// Ψһ��Ӧ�ó������

CWinApp theApp;


using namespace std;
CBeopGatewayCoreWrapper *m_pGatewayCoreWrapper = NULL;


// ע��������ͷ�����ʾ����һ����ͬ��������һ���Ѻ���
bool IsSvcRun(LPCTSTR lpszSvcName)
{ 
	SERVICE_STATUS svcStatus = {0};
	return QueryServiceStatus(OpenService(OpenSCManager(NULL, NULL, GENERIC_READ), lpszSvcName, GENERIC_READ), &svcStatus) ? (svcStatus.dwCurrentState == SERVICE_RUNNING) : false; 
}

bool ctrlhandler( DWORD fdwctrltype )
{
	switch( fdwctrltype )
	{
		// handle the ctrl-c signal.
	case CTRL_C_EVENT:
		printf( "ctrl-c event\n" );
		break;
		// ctrl-close: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		printf( "ctrl-close event\n" );
		return m_pGatewayCoreWrapper->Exit();
	case CTRL_BREAK_EVENT:
		printf( "ctrl-break event\n" );
		break;
	case CTRL_LOGOFF_EVENT:
		printf( "ctrl-logoff event\n" );
		break;
	case CTRL_SHUTDOWN_EVENT:
		printf( "ctrl-shutdown event\n" );
		break;
	default:
		break;
	}
	return false;
}


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		setlocale(LC_ALL, ".936"); 
		// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: ���Ĵ�������Է���������Ҫ
			_tprintf(L"ERROR: MFC Init Failed\n");
			nRetCode = 1;
		}
		else
		{
/*
			unsigned int j;
			redisContext *c;
			redisReply *reply;

			struct timeval timeout = { 1, 500000 }; // 1.5 seconds
			c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
			if (c->err) {
				printf("Connection error: %s\n", c->errstr);
				exit(1);
			}


			reply = static_cast<redisReply *> (redisCommand(c,"PING"));
			printf("PING: %s\n", reply->str);
			freeReplyObject(reply);

			// PING server 
			reply = static_cast<redisReply *> (redisCommand(c,"GET FOO"));
			printf("PING: %s\n", reply->str);
			freeReplyObject(reply);
			*/

			// TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
			//��OPC�ͻ��˵Ľ��̽��а�ȫ��ʼ��
			//HRESULT hrinitcode = CoInitialize(NULL);
			//if(hrinitcode != RPC_E_CHANGED_MODE){
			//	//If it was successful it would return S_OK or if COM was already initialized on the thread it would return S_FALSE, in this case even hrinitcode == S_OK would work since it is at the start of main, but if it were multithreaded app and if this was start of thread COM could have been already initialized by another thread.
			//	//Perform all your operations
			//	//At the end 
			//	CoInitializeEx(NULL, COINIT_MULTITHREADED);
			//}
			//
			//HRESULT hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
			//	RPC_C_AUTHN_LEVEL_NONE,
			//	RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL);
			//int nErrCode = GetLastError();
			/*if (FAILED(CoInitializeSecurity(NULL, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_NONE,
				RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL)))
			{
				int nErrCode = GetLastError();
				_tprintf(L"ERROR: CoInitializeSecurity fail(%d).\n",nErrCode);
			}*/

#ifdef BEOP_RUNTEST

			if(SetConsoleCtrlHandler( (PHANDLER_ROUTINE) ctrlhandler, true ))
			{
				m_pGatewayCoreWrapper = new CBeopGatewayCoreWrapper;
				m_pGatewayCoreWrapper->Run();
			}


			CCoreUnitTest coreTest;
			if(coreTest.RunTest())
			{
				AfxMessageBox(L"TEST APPROVED....\r\n");	
			}
			else
				AfxMessageBox(L"TEST ERROR!!!!!!!ERROR!!!!!ERROR!!!!!ERROR!!!!!....\r\n");
			kbhit();


#else
			if(SetConsoleCtrlHandler( (PHANDLER_ROUTINE) ctrlhandler, true ))
			{
				m_pGatewayCoreWrapper = new CBeopGatewayCoreWrapper;
				m_pGatewayCoreWrapper->Run();
			}
#endif
		}
	}
	else
	{
		// TODO: ���Ĵ�������Է���������Ҫ
		_tprintf(L"ERROR: GetModuleHandle fails.\n");
		nRetCode = 1;
	}

	return nRetCode;
}
