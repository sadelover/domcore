#include "StdAfx.h"
#include "CoreUnitTest.h"
#include "BeopGatewayCoreWrapper.h"


CCoreUnitTest::CCoreUnitTest(void)
{
}


CCoreUnitTest::~CCoreUnitTest(void)
{
}

bool CCoreUnitTest::RunTest()
{
	_tprintf(L"=====Enter Testing for gatewaycore!=====\n");
	bool bSuccess = true;
	
	Test08();


	//if(!Test03())
	//	return false;

	//if(!TestDeleteDBAndRebuild())
	//	return false;

	//if(!TestDeleteDBAndRebuild())
	//	return false;


	//if(!Test00())
	//	return false;
	//if(!Test01())
	//	return false;
	//if(!Test02())
	//	return false;

	//if(!Test04())
	//	return false;

	/*if(!Test05())
		return false;*/

	/*if(!Test06())
		return false;*/
	return true;
}

bool CCoreUnitTest::RunCommon()
{
	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Run();

	return true;

}

bool CCoreUnitTest::Test08()//by golding, testing  GetHistoryValue
{
	CBeopGatewayCoreWrapper coreWrapper;
	wstring m_strDBFilePath = coreWrapper.PreInitFileDirectory();

	if(coreWrapper.Init(m_strDBFilePath))
	{
		SYSTEMTIME  stStart;
		SYSTEMTIME  stEnd;


		COleDateTime	OleTimeStart, OleTimeEnd;
		OleTimeEnd = COleDateTime::GetCurrentTime();
		OleTimeEnd.GetAsSystemTime( stEnd );

		OleTimeStart = OleTimeEnd - COleDateTimeSpan(3,0,0,0);
		OleTimeStart.GetAsSystemTime( stStart );

		stStart = stEnd;
		stStart.wHour = 0;
		stStart.wMinute = 0;
		stStart.wSecond = 0;
		stStart.wMilliseconds = 0;

		wstring wstrResult;
		coreWrapper.m_pDataAccess_Arbiter->GetHistoryValue(_T("ChEvaValveStatus01"), stStart, stEnd, 2, wstrResult);
		_tprintf(wstrResult.c_str());

	}

	return true;
}


bool CCoreUnitTest::Test00()
{

	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\TEST01.s3db";
	wstring strTestFilePath2 = cstrFile + L"\\Test\\TEST02.s3db";

	//��������
	CBeopGatewayCoreWrapper coreWrapper;
	for(int i=0;i<2;i++)
	{
		coreWrapper.Init(strTestFilePath);
		coreWrapper.Reset();

	}

	//��������

	coreWrapper.Init(strTestFilePath2);
	for(int i=0;i<100;i++)
	{
		Sleep(100);

		_tprintf(L"Loop once test00\r\n");
		//��ȡ���룬����input��ִ�����
		coreWrapper.UpdateInputOutput();


		//update restart
		coreWrapper.UpdateResetCommand();

		//update file
		coreWrapper.UpdateS3DBDownloaded();

		//check mode change
		coreWrapper.UpdateSiteModeChanged();
	}

	coreWrapper.Reset();

	return true;
}

bool CCoreUnitTest::Test01()
{
	bool bSuccess = true;


	return bSuccess;
}

bool CCoreUnitTest::Test02()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\TEST02.s3db";
	wstring strTestDllFilePath = cstrFile + L"\\Test\\PIDMultiIOtest.dll";
	wstring strTestOldDllFilePath = cstrFile + L"\\Test\\PIDMultiIOcommon.dll";


	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Init(strTestFilePath);
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"CT_PID", L"1");
	coreWrapper.m_pDataAccess_Arbiter->InsertLogicParameters(L"CT_PID", L"CT_PID.dll", L"2", L"1", L"", L"", L""); //�޸Ĳ�������


	coreWrapper.Reset();

	return true;
}


bool CCoreUnitTest::Test03()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\Wanda_Qingdao_Store_20140820.s3db";


	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Init(strTestFilePath);
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"sitemode", L"0");
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"1Click", L"1");


	coreWrapper.Reset();

	return true;
}

bool CCoreUnitTest::Test04()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\TEST04.s3db";

	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.m_pDataEngineCore = new CBeopDataEngineCore(strTestFilePath);
	coreWrapper.m_pDataEngineCore->Init(true,true,true,6,2,false,100,true,false,L"",L"",9500,1,false);
	coreWrapper.m_pDataEngineCore->InitDBConnection();

	string strSendBuffer;
	bool	bResult = true;
	for(int i=0;i<10;i++)
	{
		if(coreWrapper.m_pDataEngineCore->GetDataLink()->InitDTUEngine(true,false))
		{
			Sleep(65000);

			coreWrapper.m_pDataEngineCore->GetDataLink()->SendDTUTestPoint("test1",L"11");
			Sleep(1000);

			coreWrapper.m_pDataEngineCore->GetDataLink()->SendDTUTestPoint("test2",L"22.01");
			Sleep(1000);

			coreWrapper.m_pDataEngineCore->GetDataLink()->SendDTUTestPoint("test3",L"test3");

			Sleep(5000);

			strSendBuffer = coreWrapper.m_pDataEngineCore->GetDataLink()->GetDTUSender()->GetSuccessBuffer();
			if(strSendBuffer.size() == 0 || strSendBuffer.find("pdp:time") == strSendBuffer.npos)
			{
				bResult = false;
			}
		}
		
		coreWrapper.m_pDataEngineCore->GetDataLink()->Exit();

		Sleep(5000);
	}
	coreWrapper.m_pDataEngineCore->Exit();
	delete coreWrapper.m_pDataEngineCore;
	coreWrapper.m_pDataEngineCore = NULL;
	return bResult;
}

bool CCoreUnitTest::Test05()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\TEST04.s3db";

	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.m_pDataEngineCore = new CBeopDataEngineCore(strTestFilePath);
	coreWrapper.m_pDataEngineCore->Init(false,false,false,1,2,false,100,true,false,L"",L"",9500,1,false);
	coreWrapper.m_pDataEngineCore->InitDBConnection();

	vector<DataPointEntry> vecmodbuspoint;
	DataPointEntry entry;
	entry.SetPointIndex(1);
	entry.SetShortName(L"a1027");
	entry.SetSourceType(L"modbus");
	entry.SetParam(1,L"3");
	entry.SetParam(2,L"16642");
	entry.SetParam(3,L"3");
	entry.SetParam(4,L"1");
	entry.SetParam(5,L"192.168.1.230");
	entry.SetStoreCycle(E_STORE_NULL);
	vecmodbuspoint.push_back(entry);

	entry.SetShortName(L"a101");
	entry.SetParam(1,L"2");
	entry.SetParam(2,L"101");
	entry.SetParam(3,L"1");
	vecmodbuspoint.push_back(entry);

	string strSendBuffer;
	bool	bResult = true;
	for(int i=0;i<5;i++)
	{
		if(coreWrapper.m_pDataEngineCore->GetDataLink()->InitModbusEngine(vecmodbuspoint))
		{
			Sleep(5000);

			 Beopdatalink::CRealTimeDataEntry tempentry;
			 tempentry.SetName("a1027");
			 tempentry.SetValue(L"-1");
			 coreWrapper.m_pDataEngineCore->GetDataLink()->WriteModbusValue(tempentry);
			 Sleep(1000);

			 tempentry.SetName("a101");
			 tempentry.SetValue(L"1");
			 coreWrapper.m_pDataEngineCore->GetDataLink()->WriteModbusValue(tempentry);
			 Sleep(1000);

			vector<pair<wstring, wstring> >	valuesets;
			coreWrapper.m_pDataEngineCore->GetDataLink()->GetModbusValueSets(valuesets);
			if(valuesets.size() < 2)
			{
				bResult = false;
			}
			else
			{
				for(int i=0; i<valuesets.size(); ++i)
				{
					if(valuesets[i].first == L"a1027")
					{
						if(_wtof(valuesets[i].second.c_str()) != -1)
							bResult = false;
					}
					else if(valuesets[i].first == L"a101")
					{
						if(_wtof(valuesets[i].second.c_str()) != 1)
							bResult = false;
					}
				}
			}

		}
		coreWrapper.m_pDataEngineCore->GetDataLink()->Exit();
		Sleep(2000);
	}
	coreWrapper.m_pDataEngineCore->Exit();
	delete coreWrapper.m_pDataEngineCore;
	coreWrapper.m_pDataEngineCore = NULL;
	return bResult;
}

bool CCoreUnitTest::Test06()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\TEST04.s3db";

	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.m_pDataEngineCore = new CBeopDataEngineCore(strTestFilePath);
	coreWrapper.m_pDataEngineCore->Init(false,false,false,1,2,false,100,true,false,L"",L"",9500,1,false);
	coreWrapper.m_pDataEngineCore->InitDBConnection();

	vector<DataPointEntry> vecbacnetpoint;
	DataPointEntry entry;
	entry.SetPointIndex(1);
	entry.SetShortName(L"b1");
	entry.SetSourceType(L"bacnet");
	entry.SetParam(1,L"260001");
	entry.SetParam(2,L"AI");
	entry.SetParam(3,L"1");
	entry.SetParam(4,L"1");
	entry.SetStoreCycle(E_STORE_NULL);
	vecbacnetpoint.push_back(entry);

	entry.SetShortName(L"b2");
	entry.SetParam(1,L"260001");
	entry.SetParam(2,L"BI");
	entry.SetParam(3,L"2");
	vecbacnetpoint.push_back(entry);

	string strSendBuffer;
	bool	bResult = true;
	for(int i=0;i<1;i++)
	{
		if(coreWrapper.m_pDataEngineCore->GetDataLink()->InitBacnetEngine(vecbacnetpoint))
		{
			Sleep(5000);

			vector<pair<wstring, wstring> >	valuesets;
			coreWrapper.m_pDataEngineCore->GetDataLink()->GetBacnetValueSets(valuesets);
			if(valuesets.size() < 2)
			{
				bResult = false;
			}
			else
			{
				for(int i=0; i<valuesets.size(); ++i)
				{
					if(valuesets[i].first == L"b1")
					{
						if(_wtof(valuesets[i].second.c_str()) != 1)
							bResult = false;
					}
					else if(valuesets[i].first == L"b2")
					{
						if(_wtof(valuesets[i].second.c_str()) != 2)
							bResult = false;
					}
				}
			}

		}
		coreWrapper.m_pDataEngineCore->GetDataLink()->Exit();
		Sleep(2000);
	}
	coreWrapper.m_pDataEngineCore->Exit();
	delete coreWrapper.m_pDataEngineCore;
	coreWrapper.m_pDataEngineCore = NULL;
	return bResult;
}
