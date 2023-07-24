#include "StdAfx.h"
#include "S7UDPCtrl.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nodaveinclude/nodave.h"
#include <fcntl.h>
#include "nodaveinclude/log2.h"
#include "nodaveinclude/openSocket.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../LAN_WANComm/Tools/ToolsFunction/PingIcmp.h"
#include <iostream>
#include <sstream>
#include "../Tools/EngineInfoDefine.h"
#define maxPBlockLen 0xDe	// real maximum 222 bytes


/*
VT_I1	指示 char 值。

VT_I2	指示 short 整数。

VT_I4	指示 long 整数。

VT_I8	指示 64 位整数。

VT_INT	指示整数值。

VT_LPSTR	指示一个以 NULL 结尾的字符串。

VT_LPWSTR	指示由 null 终止的宽字符串。

VT_NULL	指示空值（类似于 SQL 中的空值）。

VT_PTR	指示指针类型。

VT_R4	指示 float 值。

VT_R8	指示 double 值。

VT_UI1	指示 byte。

VT_UI2	指示 unsignedshort。

VT_UI4	指示 unsignedlong。

VT_UI8	指示 64 位无符号整数。

VT_UINT	指示 unsigned 整数值。

VT_DATE	指示 DATE 值。
*/


/*
VT_EMPTY            [V]   [P]         // Not specified.
VT_NULL             [V]               // SQL-style Null.
VT_I2               [V][T][P][S]      // 2-byte signed int.
VT_I4               [V][T][P][S]      // 4-byte-signed int.
VT_R4               [V][T][P][S]      // 4-byte real. 
VT_R8               [V][T][P][S]      // 8-byte real.
VT_CY               [V][T][P][S]      // Currency.
VT_DATE             [V][T][P][S]      // Date.
VT_BSTR             [V][T][P][S]      // Automation string.
VT_DISPATCH         [V][T]   [S]      // IDispatch.Far*
VT_ERROR            [V][T]   [S]      // Scodes.
VT_BOOL             [V][T][P][S]      // Boolean; True=-1, False=0.
VT_VARIANT          [V][T][P][S]      // VARIANT FAR*.
VT_DECIMAL          [V][T]   [S]      // 16 byte fixed point.
VT_RECORD           [V]   [P][S]      // User defined type
VT_UNKNOWN          [V][T]   [S]      // IUnknown FAR*.
VT_I1               [V][T]   [S]      // Char.
VT_UI1              [V][T]   [S]      // Unsigned char.
VT_UI2              [V][T]   [S]      // 2 byte unsigned int.
VT_UI4              [V][T]   [S]      // 4 byte unsigned int. 
VT_INT              [V][T]   [S]      // Signed machine int.
VT_UINT             [V][T]   [S]      // Unsigned machine int.
VT_VOID                [T]            // C-style void.
VT_HRESULT             [T]                                    
VT_PTR                 [T]            // Pointer type.
VT_SAFEARRAY           [T]            // Use VT_ARRAY in VARIANT.
VT_CARRAY              [T]            // C-style array.
VT_USERDEFINED         [T]            // User-defined type.
VT_LPSTR               [T][P]         // Null-terminated string.
VT_LPWSTR              [T][P]         // Wide null-terminated string.
VT_FILETIME               [P]         // FILETIME
VT_BLOB                   [P]         // Length prefixed bytes
VT_STREAM                 [P]         // Name of the stream follows
VT_STORAGE                [P]         // Name of the storage follows
VT_STREAMED_OBJECT        [P]         // Stream contains an object
VT_STORED_OBJECT          [P]         // Storage contains an object
VT_BLOB_OBJECT            [P]         // Blob contains an object
VT_CF                     [P]         // Clipboard format
VT_CLSID                  [P]         // A Class ID
VT_VECTOR                 [P]         // Simple counted array
VT_ARRAY            [V]               // SAFEARRAY*.
VT_BYREF            [V]
VT_RESERVED
*/


using namespace Project::Tools;

CS7UDPCtrl::CS7UDPCtrl(wstring strPLCIPAddress, int nSlack, int nSlot)
	:m_strCPUAddress(strPLCIPAddress)
	,m_nSlack(nSlack)
	,m_nSlot(nSlot)
	,m_bReadOneByOneSuccess(false)				//尚未测试，逐个读取先不开放
	,m_bPLCConnectionSuccess(false)
	,m_dbsession_history(NULL)
{


	m_pPLCInterface = NULL;
	m_pPLCConnection = NULL;
	m_hTCPSocket = NULL;
	m_nPLCReadErrorCount = 0;

	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_strErrInfo = "";
	m_nEngineIndex = 0;
}


CS7UDPCtrl::~CS7UDPCtrl(void)
{

}

bool    CS7UDPCtrl::Exit()
{

	ExitPLCConnection();

	return true;
}

void CS7UDPCtrl::AddLog( const wchar_t* loginfo )
{
	if (m_logsession)
	{
		m_logsession->InsertLog(loginfo);
		Sleep(10);
	}
}

void	CS7UDPCtrl::SetLogSession(Beopdatalink::CLogDBAccess* logsesssion)
{
	m_logsession = logsesssion;
}

void	CS7UDPCtrl::SetPointList(const vector<DataPointEntry>& pointlist)
{
	m_pointlist = pointlist;
}

void	CS7UDPCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{

	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry&  entry = m_pointlist[i];
		if(entry.GetShortName() == L"Fi4")
		{
			int a = 0;
		}
		if(m_pointlist[i].GetUpdateSignal()<5) //连续5次没有读到数据将被认为丢失，不再计入实时表。
		{
			wstring strValueSet = entry.GetSValue();
			//wstring strValueSet = Project::Tools::RemoveDecimalW(entry.GetValue());
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
	}

	if(m_pointlist.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		CString strName;
		strName.Format(_T("%s%d"),TIME_UPDATE_S7UDP,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
		strName.Format(_T("%s%d"),ERROR_S7UDP,m_nEngineIndex);
		//valuelist.push_back(make_pair(strName, Project::Tools::AnsiToWideChar(m_strErrInfo.c_str())));
	}
}

DataPointEntry*	CS7UDPCtrl::FindEntry(int nFrameType, int nPointID)
{

	return NULL;
}

void	CS7UDPCtrl::SetPLCError()
{
	if(m_nPLCReadErrorCount<=10000)
		m_nPLCReadErrorCount++;

}

void	CS7UDPCtrl::ClrPLCError()
{
	m_nPLCReadErrorCount = 0;
}

int   CS7UDPCtrl::GetErrCount()
{
	return m_nPLCReadErrorCount;
}

void	CS7UDPCtrl::EnableLog(BOOL bEnable)
{

}

bool	CS7UDPCtrl::SetValue(const wstring& pointname, double fValue)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	if(!m_bPLCConnectionSuccess)
		return false;

	map<wstring,int>::iterator itername = m_mapNameIndex.find(pointname);
	if(itername== m_mapNameIndex.end())
		return false;

	int nPointIndex = m_mapNameIndex[pointname];
	if(nPointIndex<0)
		return false;

	if(nPointIndex>=m_pointlist.size())
		return false;

	double fCurValue = m_pointlist[nPointIndex].GetValue();

	if(fabs(fCurValue - fValue)<=1e-10)
		return false; //设置值与当前值相同，不继续执行，避免PLC写入数据量繁冗


	int nDBPos = m_pointDBIndexList[nPointIndex];
	int nOffset = m_pointOffsetIndexList[nPointIndex];
	int nBitOffset = m_pointOffsetBitList[nPointIndex];
	int nBlock = m_pointBlockArea[nPointIndex];

	wchar_t wcChar[1024];
	if(m_pointVarTypeList[nPointIndex]==VT_R4)
	{
		//write

		swprintf(wcChar,L"%.2f",fValue);
		double fDataValue = fValue;
		wstring wstrScale = m_pointlist[nPointIndex].GetParam(6);
		Project::Tools::CalculateTestByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);

		float fWriteInto = fDataValue;
		fWriteInto = toPLCfloat(fWriteInto);
		int nOperResuslt = daveWriteBytes(m_pPLCConnection,nBlock,nDBPos,nOffset,4,&fWriteInto);
		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n", nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);
			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_R8)
	{
		//write
	
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_INT)
	{
		//write
		double fDataValue = fValue;
		wstring wstrScale = m_pointlist[nPointIndex].GetParam(6);
		Project::Tools::CalculateTestByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);

		short nWriteInto = (int) fDataValue;
		nWriteInto = daveSwapIed_16(nWriteInto);
		int nOperResuslt = daveWriteBytes(m_pPLCConnection,nBlock,nDBPos,nOffset,2,&nWriteInto);

		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n", nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_I1)
	{
		//write
		unsigned char buffer[256] = {0};
		double fDataValue = fValue;
		wstring wstrScale = m_pointlist[nPointIndex].GetParam(6);
		Project::Tools::CalculateTestByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);
		davePut8(buffer,(int)fDataValue);
		int nOperResuslt = daveWriteBytes(m_pPLCConnection, nBlock, nDBPos, nOffset, 1, buffer);
		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n",nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_I4)
	{
		//write
		unsigned char buffer[256] = {0};
		double fDataValue = fValue;
		wstring wstrScale = m_pointlist[nPointIndex].GetParam(6);
		Project::Tools::CalculateTestByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);

		davePut32(buffer,(int)fDataValue);
		int nOperResuslt = daveWriteBytes(m_pPLCConnection, nBlock, nDBPos, nOffset, 4, buffer);
		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n",nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_UINT)
	{
		//write
		unsigned char buffer[256] = {0};
		double fDataValue = fValue;
		wstring wstrScale = m_pointlist[nPointIndex].GetParam(6);
		Project::Tools::CalculateTestByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);
		davePut16(buffer,(int)fDataValue);
		int nOperResuslt = daveWriteBytes(m_pPLCConnection, nBlock, nDBPos, nOffset, 2, buffer);
		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n",nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_UI1)
	{
		//write
		unsigned char buffer[256] = {0};
		double fDataValue = fValue;
		wstring wstrScale = m_pointlist[nPointIndex].GetParam(6);
		Project::Tools::CalculateTestByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);
		davePut8(buffer,(int)fDataValue);
		int nOperResuslt = daveWriteBytes(m_pPLCConnection, nBlock, nDBPos, nOffset, 1, buffer);
		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n",nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_UI2)
	{
		//write
		unsigned char buffer[256] = {0};
		double fDataValue = fValue;
		wstring wstrScale = m_pointlist[nPointIndex].GetParam(6);
		Project::Tools::CalculateTestByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);
		davePut16(buffer,(int)fDataValue);
		int nOperResuslt = daveWriteBytes(m_pPLCConnection, nBlock, nDBPos, nOffset, 2, buffer);
		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n",nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_UI4)
	{
		//write
		unsigned char buffer[256] = {0};
		double fDataValue = fValue;
		wstring wstrScale = m_pointlist[nPointIndex].GetParam(6);
		Project::Tools::CalculateTestByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);
		davePut32(buffer,(int)fDataValue);
		int nOperResuslt = daveWriteBytes(m_pPLCConnection, nBlock, nDBPos, nOffset, 4, buffer);
		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n",nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}
	else if(m_pointVarTypeList[nPointIndex]==VT_BOOL)
	{
		//write
		bool bValue = (((int)fValue) ==1);
		int nOperResuslt = 0;
		//if(bValue)
			//nOperResuslt = daveSetBit(m_pPLCConnection, daveDB, nDBPos, nOffset, nBitOffset);
		//else
			//nOperResuslt = daveClrBit(m_pPLCConnection, daveDB, nDBPos, nOffset, nBitOffset);
		short nValue = (short) fValue;
		//char cValue = (nValue==1)?1:0;
		nOperResuslt = daveWriteBits(m_pPLCConnection, nBlock, nDBPos, nOffset*8+nBitOffset, 1, &nValue);

		if(nOperResuslt!=0)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP write operation: %d=%s\r\n",nOperResuslt, Project::Tools::AnsiToWideChar(daveStrerror(nOperResuslt)).c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			SetPLCError();
		}
	}

	return true;

}

void	CS7UDPCtrl::InitDataEntryValue(const wstring& pointname, double fvalue)
{

}


bool	CS7UDPCtrl::GetValue(const wstring& pointname, double& result)
{
	int nPointIndex = -1;
	nPointIndex = m_mapNameIndex[pointname];
	if(nPointIndex<0)
		return false;

	const DataPointEntry& entry = m_pointlist[nPointIndex];
	
	result = entry.GetValue();
	return true;
	
}


bool CS7UDPCtrl::Init()
{

	m_pointDBIndexList.clear();
	m_pointOffsetIndexList.clear();
	m_pointBlockArea.clear();

	int nDBPos = 0;
	int nOffset = 0;
	int nBit = 0;
	int nBlock = daveDB;
	for(int i=0;i<m_pointlist.size();i++)
	{
		VARENUM varType = GetPointDBPos(m_pointlist[i].GetPlcAddress(), nDBPos, nOffset, nBit,nBlock,m_pointlist[i].GetPlcValueType());
		m_pointVarTypeList.push_back(varType);
		m_pointDBIndexList.push_back(nDBPos);
		m_pointOffsetIndexList.push_back(nOffset);
		m_pointOffsetBitList.push_back(nBit);
		m_pointBlockArea.push_back(nBlock);
		m_mapNameIndex[m_pointlist[i].GetShortName()] = i;
	}

	bool bConnect = InitPLCConnection();
	return bConnect;
}

wstring CS7UDPCtrl::GetIP()
{
	return m_strCPUAddress;
}

VARENUM CS7UDPCtrl::GetPointDBPos(wstring strPLCAddress, int &nPos, int &nOffset, int &nBit,int &nBlock,VARENUM varType)
{
	CString strTemp = strPLCAddress.c_str();
	int nDB = strTemp.Find(L"DB");
	nBlock = daveDB;
	if(nDB>=0)
	{
		varType = VT_INT;
		//find real
		int nDB2 = strTemp.Find(L",REAL", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_R4;

			CString strOffset = strTemp.Mid(nDB2+5);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		//find int
		nDB2 = strTemp.Find(L",INT", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_INT;

			CString strOffset = strTemp.Mid(nDB2+4);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		//find BYTE
		nDB2 = strTemp.Find(L",BYTE", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_UI1;

			CString strOffset = strTemp.Mid(nDB2+5);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		//find WORD
		nDB2 = strTemp.Find(L",WORD", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_UI2;

			CString strOffset = strTemp.Mid(nDB2+5);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		//find DWORD
		nDB2 = strTemp.Find(L",DWORD", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_UI4;

			CString strOffset = strTemp.Mid(nDB2+6);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		////find DATE
		//nDB2 = strTemp.Find(L",DATE", nDB);
		//if(nDB2>nDB)
		//{
		//	CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
		//	nPos = _wtoi(strDB.GetString());
		//	varType = VT_DATE;

		//	CString strOffset = strTemp.Mid(nDB2+5);
		//	nOffset =  _wtoi(strOffset.GetString());

		//	return varType;
		//}

		//find DINT
		nDB2 = strTemp.Find(L",DINT", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_I4;

			CString strOffset = strTemp.Mid(nDB2+5);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		////find LREAL
		//nDB2 = strTemp.Find(L",LREAL", nDB);
		//if(nDB2>nDB)
		//{
		//	CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
		//	nPos = _wtoi(strDB.GetString());
		//	varType = VT_R8;

		//	CString strOffset = strTemp.Mid(nDB2+6);
		//	nOffset =  _wtoi(strOffset.GetString());

		//	return varType;
		//}

		//find SINT
		nDB2 = strTemp.Find(L",SINT", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_I1;

			CString strOffset = strTemp.Mid(nDB2+5);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		////find STRING
		//nDB2 = strTemp.Find(L",STRING", nDB);
		//if(nDB2>nDB)
		//{
		//	CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
		//	nPos = _wtoi(strDB.GetString());
		//	varType = VT_BSTR;

		//	CString strOffset = strTemp.Mid(nDB2+7);
		//	nOffset =  _wtoi(strOffset.GetString());

		//	return varType;
		//}

		////find TIME
		//nDB2 = strTemp.Find(L",TIME", nDB);
		//if(nDB2>nDB)
		//{
		//	CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
		//	nPos = _wtoi(strDB.GetString());
		//	varType = VT_I4;

		//	CString strOffset = strTemp.Mid(nDB2+5);
		//	nOffset =  _wtoi(strOffset.GetString());

		//	return varType;
		//}

		//find UDINT
		nDB2 = strTemp.Find(L",UDINT", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_UI4;

			CString strOffset = strTemp.Mid(nDB2+6);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		//find UINT
		nDB2 = strTemp.Find(L",UINT", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_UINT;

			CString strOffset = strTemp.Mid(nDB2+5);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		//find USINT
		nDB2 = strTemp.Find(L",USINT", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_UI1;

			CString strOffset = strTemp.Mid(nDB2+6);
			nOffset =  _wtoi(strOffset.GetString());

			return varType;
		}

		//find bool
		nDB2 = strTemp.Find(L",X", nDB);
		if(nDB2>nDB)
		{
			CString strDB =  strTemp.Mid(nDB+2, nDB2-nDB-2);
			nPos = _wtoi(strDB.GetString());
			varType = VT_BOOL;

			int nDotPos = strTemp.Find(L".", nDB2+1);
			if(nDotPos>nDB2)
			{
				CString strOffset = strTemp.Mid(nDB2+2, nDotPos - nDB2-2);
				nOffset =  _wtoi(strOffset.GetString());

				CString strBit = strTemp.Mid(nDotPos+1);
				nBit =  _wtoi(strBit.GetString());

				return varType;
			}
		}
	}
	else
	{
		return GetPointDBPos_200(strPLCAddress,nPos,nOffset,nBit,nBlock,varType);	
	}

	return varType;
}

bool    CS7UDPCtrl::ReadDataIndex(int nFromIndex, int nToIndex)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(!m_bPLCConnectionSuccess)
		return false;

	if(nFromIndex>nToIndex)
		return false;
	int number, blockCont;
	int nResult;
	int blockNumber,rawLen,netLen;
	char blockType;
	PDU p;

	if(m_pPLCConnection)
		davePrepareReadRequest(m_pPLCConnection, &p);

	for(int i=nFromIndex;i<=nToIndex;i++)
	{
		DataPointEntry &pt =  m_pointlist[i];
		pt.SetToUpdate(); //刷新标记先+1
		if(m_pointVarTypeList[i] == VT_R4)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,4);
		}
		/*else if(m_pointVarTypeList[i] == VT_R8)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			daveAddVarToReadRequest(&p,daveDB, nDBPos,nOffset,8);

		}*/
		else if(m_pointVarTypeList[i] == VT_INT)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,2);

		}
		else if(m_pointVarTypeList[i] == VT_UINT)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,2);

		}
		else if(m_pointVarTypeList[i] == VT_UI1)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,1);

		}
		else if(m_pointVarTypeList[i] == VT_UI2)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,2);

		}
		else if(m_pointVarTypeList[i] == VT_UI4)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,4);

		}
		/*else if(m_pointVarTypeList[i] == VT_DATE)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			daveAddVarToReadRequest(&p,daveDB, nDBPos,nOffset,2);

		}*/
		else if(m_pointVarTypeList[i] == VT_I1)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,1);

		}
		else if(m_pointVarTypeList[i] == VT_I4)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,4);

		}
		/*else if(m_pointVarTypeList[i] == VT_BSTR)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			daveAddVarToReadRequest(&p,daveDB, nDBPos,nOffset,256);

		}*/
		else if(m_pointVarTypeList[i] == VT_UINT)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,2);

		}
		else if(m_pointVarTypeList[i] == VT_BOOL)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			int nBit = m_pointOffsetBitList[i];
			int nBlock = m_pointBlockArea[i];
			daveAddBitVarToReadRequest(&p, nBlock, nDBPos, nOffset*8+nBit, 1);

		}
	}

	if(!m_bPLCConnectionSuccess)
		return false;

	//daveSetTimeout(m_pPLCInterface,5000000);

	daveResultSet rs;
	nResult = daveExecReadRequest(m_pPLCConnection, &p, &rs);
	wchar_t wcChar[1024];
	if(nResult!=daveResOK)
	{
		wsprintf(wcChar, L"ERROR: SiemensTCP reading From %s To %s\r\n", m_pointlist[nFromIndex].GetShortName().c_str(), m_pointlist[nToIndex].GetShortName().c_str());
		PrintLog(wcChar);

		wstring wstrErr = AnsiToWideChar(daveStrerror(nResult));
		wsprintf(wcChar, L"ERROR: SiemensTCP(%s) read: %d=%s\r\n",m_strCPUAddress.c_str(),nResult, wstrErr.c_str());
		PrintLog(wcChar);

		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
		SetPLCError();
		return false;
	}
	int nGetErrCount = 0;
	for(int i=nFromIndex;i<=nToIndex;i++)
	{
		if(!m_bPLCConnectionSuccess)
			return false;
		nResult = daveUseResult(m_pPLCConnection, &rs, i-nFromIndex); // first result
		if(nResult!=daveResOK)
		{
			wsprintf(wcChar, L"ERROR: SiemensTCP query %s\r\n", m_pointlist[i].GetShortName().c_str());
			PrintLog(wcChar);

			wstring wstrErr = AnsiToWideChar(daveStrerror(nResult));
			wsprintf(wcChar, L"ERROR: SiemensTCP(%s) query: %d=%s\r\n",m_strCPUAddress.c_str(),nResult, wstrErr.c_str());
			PrintLog(wcChar);

			m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
			nGetErrCount++;
			if(nGetErrCount >= 6)
			{	
				wsprintf(wcChar, L"ERROR: SiemensTCP reading From %s To %s(daveUseResult faile six time in a row)\r\n",m_pointlist[nFromIndex].GetShortName().c_str(), m_pointlist[nToIndex].GetShortName().c_str());
				PrintLog(wcChar);

				SetPLCError();
				daveFreeResults(&rs);
				return false;
			}
			continue;
		}
		nGetErrCount = 0;
		DataPointEntry &pt =  m_pointlist[i];
		if(m_pointVarTypeList[i] == VT_R4)
		{
			float fValue = daveGetFloat(m_pPLCConnection); 
			double fDataValue = fValue;
			wstring wstrScale = pt.GetParam(6);
			if(wstrScale.length()>0)
			{
				Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);
				double fscale = _wtof(wstrScale.c_str());
				if(fscale>1e-10)
				{
					fDataValue = fDataValue/fscale;
				}
			}

			swprintf(wcChar,L"%.4f",fDataValue);
			pt.SetSValue(wcChar);
			pt.SetUpdated();
		}
		/*else if(m_pointVarTypeList[i] == VT_R8)
		{
			float fValue = daveGetFloat(m_pPLCConnection); 
			swprintf(wcChar,L"%.4f",fValue);
			pt.SetSValue(wcChar);
			pt.SetUpdated();
		}*/
		else if(m_pointVarTypeList[i] == VT_INT)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];

			int nValue = daveGetS16(m_pPLCConnection);

			double fDataValue = (double)nValue;
			wstring wstrScale = pt.GetParam(6);
			Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

			pt.SetValue(fDataValue);
			pt.SetUpdated();
		}
		else if(m_pointVarTypeList[i] == VT_UINT)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];

			unsigned int nValue = daveGetU16(m_pPLCConnection);
			double fDataValue = (double)nValue;
			wstring wstrScale = pt.GetParam(6);
			Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

			pt.SetValue(fDataValue);
			pt.SetUpdated();
		}
		else if(m_pointVarTypeList[i] == VT_UI1)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];
			unsigned short nValue = daveGetU8(m_pPLCConnection);
			double fDataValue = (double)nValue;
			wstring wstrScale = pt.GetParam(6);
			Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

			pt.SetValue(fDataValue);
			pt.SetUpdated();
		}
		else if(m_pointVarTypeList[i] == VT_UI2)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];

			unsigned int nValue = daveGetU16(m_pPLCConnection);
			double fDataValue = (double)nValue;
			wstring wstrScale = pt.GetParam(6);
			Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

			pt.SetValue(fDataValue);
			pt.SetUpdated();
		}
		else if(m_pointVarTypeList[i] == VT_UI4)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];

			unsigned long nValue = daveGetU32(m_pPLCConnection);
			double fDataValue = (double)nValue;
			wstring wstrScale = pt.GetParam(6);
			Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

			pt.SetValue(fDataValue);
			pt.SetUpdated();
		}
		else if(m_pointVarTypeList[i] == VT_I1)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];

			short nValue = daveGetS8(m_pPLCConnection);
			double fDataValue = (double)nValue;
			wstring wstrScale = pt.GetParam(6);
			Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

			pt.SetValue(fDataValue);
			pt.SetUpdated();
		}
		else if(m_pointVarTypeList[i] == VT_I4)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];

			long nValue = daveGetS32(m_pPLCConnection);
			double fDataValue = (double)nValue;
			wstring wstrScale = pt.GetParam(6);
			Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

			pt.SetValue(fDataValue);
			pt.SetUpdated();
		}
		else if(m_pointVarTypeList[i] == VT_BOOL)
		{
			int nDBPos = m_pointDBIndexList[i];
			int nOffset = m_pointOffsetIndexList[i];

			short  nValue = daveGetU8(m_pPLCConnection);
			double fDataValue = (double)nValue;
			wstring wstrScale = pt.GetParam(6);
			Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

			pt.SetValue(fDataValue);
			pt.SetUpdated();
		}
		m_oleUpdateTime = COleDateTime::GetCurrentTime();

	}
	daveFreeResults(&rs);	


	return true;
}

//读取一次PLC所有数据
bool CS7UDPCtrl::ReadDataOnce()
{
	if(!m_bPLCConnectionSuccess)
		return false;

	if(!m_pPLCConnection)
	{
		return false;
	}

	//一次性读取最多19个数据
	int nMAXReadCountOnce = 19;
	int nFromIndex = 0;
	int nToIndex =0;
	bool bReadSuccess = true;
	for(int i=0;i<m_pointlist.size();i++)
	{

		nToIndex = nFromIndex+nMAXReadCountOnce-1;

		if((nToIndex+1) >= m_pointlist.size())
		{
			nToIndex = m_pointlist.size()-1;
		}

		bReadSuccess = ReadDataIndex(nFromIndex, nToIndex) && bReadSuccess;

		nFromIndex = nToIndex+1;
		if((nFromIndex+1) > m_pointlist.size())
		{
			break;
		}

		Sleep(50);
	}

	if(bReadSuccess)
		UpdateSimensTime();
	return bReadSuccess;
}

bool CS7UDPCtrl::ExitPLCConnection()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	m_bPLCConnectionSuccess = false;

	if(m_hTCPSocket)
	{
		closeSocket(m_hTCPSocket);
		m_hTCPSocket = NULL;
	}

	if(m_pPLCConnection)
		daveDisconnectPLC(m_pPLCConnection);

	if(m_pPLCInterface)
		daveDisconnectAdapter(m_pPLCInterface);

	m_pPLCConnection = NULL;
	m_pPLCInterface = NULL;
	return true;
}

bool CS7UDPCtrl::InitPLCConnection()
{
	int i, initSuccess,useProto, useSlot;
	HANDLE fd;
	unsigned long res;

	
	_daveOSserialType fds;
	PDU p,p2;

	uc pup[]= {			// Load request
		0x1A,0,1,0,0,0,0,0,9,
		0x5F,0x30,0x42,0x30,0x30,0x30,0x30,0x34,0x50, // block type code and number
		//     _    0    B   0     0    0    0    4    P
		//		SDB		
		0x0D,
		0x31,0x30,0x30,0x30,0x32,0x30,0x38,0x30,0x30,0x30,0x31,0x31,0x30,0	// file length and netto length
		//     1   0     0    0    2    0    8    0    0    0    1    1    0
	};

	uc pablock[]= {	// parameters for parts of a block
		0x1B,0
	};

	uc progBlock[1260]= {
		0,maxPBlockLen,0,0xFB,	// This seems to be a fix prefix for program blocks
	};

	uc paInsert[]= {		// sended after transmission of a complete block,
		// I this makes the CPU link the block into a program
		0x28,0x00,0x00,0x00, 
		0x00,0x00,0x00,0xFD, 
		0x00,0x0A,0x01,0x00, 
		0x30,0x42,0x30,0x30,0x30,0x30,0x34,0x50, // block type code and number	
		0x05,'_','I','N','S','E',
	};

	daveSetDebug(daveDebugPrintErrors);


	wstring wstrISOProtoType = m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"dave_proto_iso_tcp243");
	if(wstrISOProtoType ==L"1")
	{
		printf("PLC DaveProto Type: daveProtoISOTCP243");
		useProto= daveProtoISOTCP243;
	}
	else
	{

		printf("PLC DaveProto Type: daveProtoISOTCP Standard");
		useProto=daveProtoISOTCP;
	}
	useSlot=0; //golding

	wchar_t wcChar[1024];

	wsprintf(wcChar, L"INFO : Try connect to PLC(%s, slack=%d, slot=%d)...\n", m_strCPUAddress.c_str(), m_nSlack, m_nSlot);
	PrintLog(wcChar,false);

	string strIPAnsi = Project::Tools::WideCharToAnsi(m_strCPUAddress.c_str());

	m_pPLCConnection = NULL;
	m_bPLCConnectionSuccess = false;
	fds.rfd = openSocket(102, strIPAnsi.data());
	m_hTCPSocket = fds.rfd;
	fds.wfd=fds.rfd;

	if (fds.rfd>0) 
	{ 
		m_pPLCInterface = daveNewInterface(fds,"IF1",0, useProto, daveSpeed187k);
		m_pPLCInterface->timeout=5000000;
		m_pPLCConnection = daveNewConnection(m_pPLCInterface, m_nSlack, m_nSlack, m_nSlot);  // insert your rack and slot here

		daveSetCommunicationType(m_pPLCConnection, daveOPCommunication);
		initSuccess=0;
		for(i=0;i<=3;i++)
		{
			if (0==daveConnectPLC(m_pPLCConnection)) 
			{
				initSuccess=1;
				break;
			} 
			else  
			{
				daveDisconnectPLC(m_pPLCConnection);
			}
		}
		if (initSuccess) //PLC连接成功
		{ 
			daveSetTimeout(m_pPLCInterface,5000000);
			printf("Success!\r\n");	
			OutPutLogString(L"Connect to PLC Success");
			m_strErrInfo = Project::Tools::OutTimeInfo("Connect to PLC Success");
			m_bPLCConnectionSuccess = true;
		}
		else 
		{
			m_pPLCConnection = NULL;
			daveDisconnectAdapter(m_pPLCInterface);
			m_bPLCConnectionSuccess = false;
			printf("Couldn't connect to PLC\n");
			OutPutLogString(L"Couldn't connect to PLC");
			m_strErrInfo = Project::Tools::OutTimeInfo("Couldn't connect to PLC");
			return false;
		}    
		
	} 
	else
	{
		CString strLog;
		strLog.Format(_T("ERROR: Couldn't connect to PLC addressing %s \r\n"),m_strCPUAddress.c_str());
		PrintLog(strLog.GetString());
		return false;
	}
	return true;
}

bool CS7UDPCtrl::ReadOneByOneAndRemoveErrPoint()
{
	if(!m_bPLCConnectionSuccess)
		return false;

	if(!m_pPLCConnection)
	{
		return false;
	}

	vector<int>	vecReadErr;					//初始化读失败的点
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		if(!ReadOneDataByIndex(i))			//读失败
		{
			vecReadErr.push_back(i);
		}
	}

	//移除错误的点
	for(int j=vecReadErr.size()-1; j>=0; --j)
	{
		m_pointlist.erase(m_pointlist.begin()+vecReadErr[j]);
	}

	//重新初始化
	m_pointDBIndexList.clear();
	m_pointOffsetIndexList.clear();
	m_pointOffsetBitList.clear();
	m_pointVarTypeList.clear();
	m_pointBlockArea.clear();
	m_mapNameIndex.clear();
	int nDBPos = 0;
	int nOffset = 0;
	int nBit = 0;
	int nBlock = daveDB;
	for(int i=0;i<m_pointlist.size();i++)
	{
		VARENUM varType = GetPointDBPos(m_pointlist[i].GetPlcAddress(), nDBPos, nOffset, nBit,nBlock,m_pointlist[i].GetPlcValueType());
		m_pointVarTypeList.push_back(varType);
		m_pointDBIndexList.push_back(nDBPos);
		m_pointOffsetIndexList.push_back(nOffset);
		m_pointOffsetBitList.push_back(nBit);
		m_pointBlockArea.push_back(nBlock);
		m_mapNameIndex[m_pointlist[i].GetShortName()] = i;
	}
	m_bReadOneByOneSuccess = true;
	return true;
}

bool CS7UDPCtrl::ReadOneDataByIndex( int nIndex )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(!m_bPLCConnectionSuccess)
		return false;

	if(nIndex <0 || nIndex>=m_pointlist.size())
		return false;
	int number, blockCont;
	int nResult;
	int blockNumber,rawLen,netLen;
	char blockType;

	PDU p;
	if(m_pPLCConnection)
		davePrepareReadRequest(m_pPLCConnection, &p);
	if(m_pointVarTypeList[nIndex] == VT_R4)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,4);
	}
	/*else if(m_pointVarTypeList[nIndex] == VT_R8)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		daveAddVarToReadRequest(&p,daveDB, nDBPos,nOffset,8);

	}*/
	else if(m_pointVarTypeList[nIndex] == VT_INT)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,2);

	}
	else if(m_pointVarTypeList[nIndex] == VT_UINT)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,2);

	}
	else if(m_pointVarTypeList[nIndex] == VT_UI1)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,1);

	}
	else if(m_pointVarTypeList[nIndex] == VT_UI2)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,2);

	}
	else if(m_pointVarTypeList[nIndex] == VT_UI4)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,4);

	}
	/*else if(m_pointVarTypeList[nIndex] == VT_DATE)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		daveAddVarToReadRequest(&p,daveDB, nDBPos,nOffset,2);

	}*/
	else if(m_pointVarTypeList[nIndex] == VT_I1)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,1);

	}
	else if(m_pointVarTypeList[nIndex] == VT_I4)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,4);

	}
	/*else if(m_pointVarTypeList[nIndex] == VT_BSTR)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		daveAddVarToReadRequest(&p,daveDB, nDBPos,nOffset,256);

	}*/
	else if(m_pointVarTypeList[nIndex] == VT_UINT)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddVarToReadRequest(&p,nBlock, nDBPos,nOffset,2);

	}
	else if(m_pointVarTypeList[nIndex] == VT_BOOL)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		int nBit = m_pointOffsetBitList[nIndex];
		int nBlock = m_pointBlockArea[nIndex];
		daveAddBitVarToReadRequest(&p, nBlock, nDBPos, nOffset*8+nBit, 1);

	}

	if(!m_bPLCConnectionSuccess)
		return false;
	daveResultSet rs;
	nResult = daveExecReadRequest(m_pPLCConnection, &p, &rs);
	wchar_t wcChar[1024];
	if(nResult!=daveResOK)
	{
		wsprintf(wcChar, L"ERROR: SiemensTCP reading %s\r\n", m_pointlist[nIndex].GetShortName().c_str());
		PrintLog(wcChar);

		wstring wstrErr = AnsiToWideChar(daveStrerror(nResult));
		wsprintf(wcChar, L"ERROR: SiemensTCP(%s) read: %d=%s\r\n",m_strCPUAddress.c_str(),nResult, wstrErr.c_str());
		PrintLog(wcChar);

		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
		SetPLCError();
		return false;
	}
	nResult = daveUseResult(m_pPLCConnection, &rs,0); // first result
	if(nResult!=daveResOK)
	{
		wsprintf(wcChar, L"ERROR: SiemensTCP query %s\r\n", m_pointlist[nIndex].GetShortName().c_str());
		PrintLog(wcChar);

		wstring wstrErr = AnsiToWideChar(daveStrerror(nResult));
		wsprintf(wcChar, L"ERROR: SiemensTCP(%s) query: %d=%s\r\n",m_strCPUAddress.c_str(),nResult, wstrErr.c_str());
		PrintLog(wcChar);

		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(wcChar));
		daveFreeResults(&rs);	
		return false;
	}
	DataPointEntry &pt =  m_pointlist[nIndex];
	if(m_pointVarTypeList[nIndex] == VT_R4)
	{
		float fValue = daveGetFloat(m_pPLCConnection); 

		double fDataValue = fValue;
		wstring wstrScale = pt.GetParam(6);

		Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), fValue, fDataValue);

		swprintf(wcChar,L"%.2f",fDataValue);
		pt.SetSValue(wcChar);
		pt.SetUpdated();
	}
	/*else if(m_pointVarTypeList[nIndex] == VT_R8)
	{
		float fValue = daveGetFloat(m_pPLCConnection); 
		swprintf(wcChar,L"%.4f",fValue);
		pt.SetSValue(wcChar);
		pt.SetUpdated();
	}*/
	else if(m_pointVarTypeList[nIndex] == VT_INT)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];

		int nValue = daveGetS16(m_pPLCConnection);
		double fDataValue = (double)nValue;
		wstring wstrScale = pt.GetParam(6);
		
		Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);
		pt.SetValue(fDataValue);
		pt.SetUpdated();
	}
	else if(m_pointVarTypeList[nIndex] == VT_UINT)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];

		unsigned int nValue = daveGetU16(m_pPLCConnection);
		double fDataValue = (double)nValue;
		wstring wstrScale = pt.GetParam(6);

		Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);

		pt.SetValue(fDataValue);
		pt.SetUpdated();
	}
	else if(m_pointVarTypeList[nIndex] == VT_UI1)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];
		unsigned short nValue = daveGetU8(m_pPLCConnection);

		double fDataValue = (double)nValue;
		wstring wstrScale = pt.GetParam(6);

		Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);
		pt.SetValue(fDataValue);
		pt.SetUpdated();
	}
	else if(m_pointVarTypeList[nIndex] == VT_UI2)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];

		unsigned int nValue = daveGetU16(m_pPLCConnection);

		double fDataValue = (double)nValue;
		wstring wstrScale = pt.GetParam(6);

		Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);
		pt.SetValue(fDataValue);
		pt.SetUpdated();
	}
	else if(m_pointVarTypeList[nIndex] == VT_UI4)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];

		unsigned long nValue = daveGetU32(m_pPLCConnection);
		double fDataValue = (double)nValue;
		wstring wstrScale = pt.GetParam(6);

		Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);
		pt.SetValue(fDataValue);
		pt.SetUpdated();
	}
	else if(m_pointVarTypeList[nIndex] == VT_I1)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];

		short nValue = daveGetS8(m_pPLCConnection);
		double fDataValue = (double)nValue;
		wstring wstrScale = pt.GetParam(6);

		Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);
		pt.SetValue(fDataValue);
		pt.SetUpdated();
	}
	else if(m_pointVarTypeList[nIndex] == VT_I4)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];

		long nValue = daveGetS32(m_pPLCConnection);
		double fDataValue = (double)nValue;
		wstring wstrScale = pt.GetParam(6);

		Project::Tools::CalculateRealByScaleOrMapString(wstrScale.c_str(), (double)nValue, fDataValue);
		pt.SetValue(fDataValue);
		pt.SetUpdated();
	}
	/*else if(m_pointVarTypeList[nIndex] == VT_BSTR)
	{

	}
	else if(m_pointVarTypeList[nIndex] == VT_DATE)
	{

	}*/
	else if(m_pointVarTypeList[nIndex] == VT_BOOL)
	{
		int nDBPos = m_pointDBIndexList[nIndex];
		int nOffset = m_pointOffsetIndexList[nIndex];

		short  nValue = daveGetU8(m_pPLCConnection);
		pt.SetValue(nValue);
		pt.SetUpdated();
	}
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	daveFreeResults(&rs);	
	return true;
}

bool CS7UDPCtrl::GetReadOneByOneSuccess()
{
	return m_bReadOneByOneSuccess;
}

bool CS7UDPCtrl::GetConnectSuccess()
{
	return m_bPLCConnectionSuccess;
}

string CS7UDPCtrl::GetExecuteOrderLog()
{
	if(m_strExecuteLog.length()<=0)
		return "";

	string strOut;
	strOut += Project::Tools::SystemTimeToString(m_sExecuteTime);
	strOut += "(S7UDPCtrl-";
	strOut += Project::Tools::WideCharToAnsi(m_strCPUAddress.c_str());
	strOut += ")::";
	strOut += m_strExecuteLog;

	m_strExecuteLog = "";
	return strOut;
}

bool CS7UDPCtrl::OutPutLogString( wstring strOut )
{
	char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
	setlocale( LC_ALL, "chs" );

	CString strLog;
	SYSTEMTIME st;
	GetLocalTime(&st);
	wstring strTime;
	Project::Tools::SysTime_To_String(st,strTime);
	strLog += strTime.c_str();
	strLog += _T(" - ");

	strLog += strOut.c_str();
	strLog += _T("\n");

	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		wstring strPath;
		GetSysPath(strPath);
		CString strLogPath;
		strLogPath.Format(_T("%s\\s7udpctrl_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
	}
	setlocale( LC_CTYPE, old_locale ); 
	free( old_locale ); 
	return false;
}

bool CS7UDPCtrl::UpdateSimensTime()
{
	/** 20200520 golding
	SYSTEMTIME st;
	GetLocalTime(&st);
	wstring strTime;
	Project::Tools::SysTime_To_String(st,strTime);

	if(m_dbsession_history)
	{
		m_dbsession_history->WriteCoreDebugItemValue(L"runs7",L"1");
		m_dbsession_history->WriteCoreDebugItemValue(L"s7updateuime",strTime);
	}

	*/
	return true;
}

void CS7UDPCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

void CS7UDPCtrl::SetHistoryDbCon( Beopdatalink::CCommonDBAccess* phisdbcon )
{
	m_dbsession_history = phisdbcon;
}

VARENUM CS7UDPCtrl::GetPointDBPos_200( wstring strPLCAddress, int &nPos, int &nOffset, int &nBit,int &nBlock ,VARENUM varType)
{
	CString strTemp = strPLCAddress.c_str();
	nBlock = daveDB;
	nBit = 0;
	CString strFirstLetter = strTemp.Mid(0, 1);
	if(strFirstLetter == _T("V"))				//变量存储器
	{
		nBlock = daveDB;
		nPos = 1;
		return GetPointPos_200(strTemp.GetString(),nOffset,nBit,varType);
	}
	else if(strFirstLetter == _T("I") || strFirstLetter == _T("E"))			//输入映像寄存器
	{
		nBlock = daveInputs;
		nPos = 0;
		return GetPointPos_200(strTemp.GetString(),nOffset,nBit,varType);
	}
	else if(strFirstLetter == _T("Q"))			//输出映像寄存器
	{
		nBlock = daveOutputs;
		nPos = 0;
		return GetPointPos_200(strTemp.GetString(),nOffset,nBit,varType);
	}
	else if(strFirstLetter == _T("M") || strFirstLetter == _T("F"))			//内部标志位
	{
		nBlock = daveFlags;
		nPos = 0;
		return GetPointPos_200(strTemp.GetString(),nOffset,nBit,varType);
	}
	else if(strFirstLetter == _T("T"))			//定时器
	{
		nPos = 0;
		nBlock = daveTimer200;
		if(varType == VT_NULL)
			varType = VT_UI2;
		CString strOffset = strTemp.Mid(1);
		nOffset =  _wtoi(strOffset.GetString());
		return varType;
	}
	else if(strFirstLetter == _T("C") || strFirstLetter == _T("Z"))			//计数器
	{
		nPos = 0;
		nBlock = daveCounter200;
		if(varType == VT_NULL)
			varType = VT_UI2;
		CString strOffset = strTemp.Mid(1);
		nOffset =  _wtoi(strOffset.GetString());
		return varType;
	}
	else if(strFirstLetter == _T("S"))		//SF(SM)   S
	{
		CString strTwoLetter = strTemp.Mid(0, 2);
		if(strTwoLetter == _T("SF") || strTwoLetter == _T("SM"))
		{
			nBlock = daveSysFlags;
			nPos = 0;
			return GetPointPos_200_(strTemp.GetString(),nOffset,nBit,varType);
		}
		else
		{
			nBlock = daveSysDataS5;
			nPos = 0;
			return GetPointPos_200(strTemp.GetString(),nOffset,nBit,varType);
		}
	}
	else if(strFirstLetter == _T("A"))		//AA(AQ)  AE(AI)
	{
		CString strTwoLetter = strTemp.Mid(0, 2);
		if(strTwoLetter == _T("AA") || strTwoLetter == _T("AQ"))
		{
			nBlock = daveAnaOut;
			nPos = 0;
			return GetPointPos_200_(strTemp.GetString(),nOffset,nBit,varType);
		}
		else if(strTwoLetter == _T("AE") || strTwoLetter == _T("AI"))
		{
			nBlock = daveAnaIn;
			nPos = 0;
			return GetPointPos_200_(strTemp.GetString(),nOffset,nBit,varType);
		}
	}
	else if(strFirstLetter == _T("P"))		//PI 或 PE
	{
		CString strTwoLetter = strTemp.Mid(0, 2);
		if(strTwoLetter == _T("PE") || strTwoLetter == _T("PI"))
		{
			nBlock = daveP;
			nPos = 0;
			return GetPointPos_200_(strTemp.GetString(),nOffset,nBit,varType);
		}
	}
	if(varType == VT_NULL)
		varType = VT_INT;			//没有默认给个int
	return varType;
}

VARENUM CS7UDPCtrl::GetPointPos_200( wstring strPLCAddress, int &nOffset, int &nBit ,VARENUM varType)
{
	CString strTemp = strPLCAddress.c_str();
	CString strTypeLetter = strTemp.Mid(1, 1);
	if(strTypeLetter == _T("B"))			//字节
	{
		if(varType == VT_NULL)
			varType = VT_UI1;
		CString strOffset = strTemp.Mid(2);
		nOffset =  _wtoi(strOffset.GetString());
		return varType;
	}
	else if(strTypeLetter == _T("W"))		//字
	{
		if(varType == VT_NULL)
			varType = VT_UI2;
		CString strOffset = strTemp.Mid(2);
		nOffset =  _wtoi(strOffset.GetString());
		return varType;
	}
	else if(strTypeLetter == _T("D"))		//双字
	{
		if(varType == VT_NULL)
			varType = VT_R4;
		CString strOffset = strTemp.Mid(2);
		nOffset =  _wtoi(strOffset.GetString());
		return varType;
	}
	else									//位
	{
		if(varType == VT_NULL)
			varType = VT_BOOL;
		int nDotPos = strTemp.Find(L".",1);
		if(nDotPos>1)
		{
			CString strOffset = strTemp.Mid(1, nDotPos - 1);
			nOffset =  _wtoi(strOffset.GetString());
			CString strBit = strTemp.Mid(nDotPos+1);
			nBit =  _wtoi(strBit.GetString());
			return varType;
		}
	}
	if(varType == VT_NULL)
		varType = VT_INT;
	return varType;
}

VARENUM CS7UDPCtrl::GetPointPos_200_( wstring strPLCAddress, int &nOffset, int &nBit ,VARENUM varType)
{
	CString strTemp = strPLCAddress.c_str();
	CString strTypeLetter = strTemp.Mid(2, 1);
	if(strTypeLetter == _T("B"))			//字节
	{
		if(varType == VT_NULL)
			varType = VT_UI1;
		CString strOffset = strTemp.Mid(3);
		nOffset =  _wtoi(strOffset.GetString());
		return varType;
	}
	else if(strTypeLetter == _T("W"))		//字
	{
		if(varType == VT_NULL)
			varType = VT_UI2;
		CString strOffset = strTemp.Mid(3);
		nOffset =  _wtoi(strOffset.GetString());
		return varType;
	}
	else if(strTypeLetter == _T("D"))		//双字
	{
		//varType = VT_UI4;
		if(varType == VT_NULL)
			varType = VT_R4;
		CString strOffset = strTemp.Mid(3);
		nOffset =  _wtoi(strOffset.GetString());
		return varType;
	}
	else									//位
	{
		if(varType == VT_NULL)
			varType = VT_BOOL;
		int nDotPos = strTemp.Find(L".",2);
		if(nDotPos>1)
		{
			CString strOffset = strTemp.Mid(2, nDotPos - 2);
			nOffset =  _wtoi(strOffset.GetString());
			CString strBit = strTemp.Mid(nDotPos+1);
			nBit =  _wtoi(strBit.GetString());
			return varType;
		}
	}
	if(varType == VT_NULL)
		varType = VT_INT;
	return varType;
}

void CS7UDPCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
	if(bSaveLog)
	{
		OutPutLogString(strLog);
	}
}
