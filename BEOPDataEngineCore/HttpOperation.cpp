#include "StdAfx.h"
#include "HttpOperation.h"
#include <string>
#include "../Tools/CustomTools/CustomTools.h"

CHttpOperation::CHttpOperation(const CString strIp, const int nPort, const CString strDir)
	: m_pConnection(NULL)
	, m_strIp(strIp)
	, m_nPort(nPort)
	, m_strDir(strDir)
{

	m_nReceiveTimeOut = 10000;
	m_nServerTimeOut= 10000;

	CreateConnection(m_strIp, m_nPort);


}


CHttpOperation::~CHttpOperation(void)
{
	CloseConnection();
}

bool CHttpOperation::CreateConnection(const CString strHost, const INTERNET_PORT nPort)
{
	if (NULL == m_pConnection)
	{
		try
		{

			m_Isession.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, m_nReceiveTimeOut);
			m_Isession.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, m_nServerTimeOut);

			m_pConnection = m_Isession.GetHttpConnection((LPCTSTR)strHost, nPort);
		}
		catch (...)
		{
			return false;
		}

		if (NULL == m_pConnection)
		{
			return false;
		}
	}

	return true;
}

bool CHttpOperation::CloseConnection(void)
{
	if (m_pConnection != NULL)
	{
		m_pConnection->Close();
		delete m_pConnection;
		m_pConnection = NULL;
	}

	m_Isession.Close();
	return true;
}

bool CHttpOperation::HttpPostGetRequest(CString& strResult)
{
	if (NULL == m_pConnection)
	{
		return false;
	}

	string strData = "name=beopfactory";
	wstring HTTP_HEADER = L"Content-Type: application/x-www-form-urlencoded";
	CHttpFile*	pFile = NULL;
	WCHAR*	pUnicode = NULL;

	try
	{
		pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, m_strDir);
		if (pFile != NULL && pFile->SendRequest(HTTP_HEADER.c_str(), (DWORD)HTTP_HEADER.length(), (LPVOID*)strData.c_str(), strData.length()))
		{
			string strHtml;
			char szChars[1025] = {0};
			while ((pFile->Read((void*)szChars, 1024)) > 0)
			{
				strHtml += szChars;
			}
			int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strHtml.c_str(), -1, NULL, 0);
			pUnicode = new WCHAR[unicodeLen + 1];
			memset(pUnicode, 0, (unicodeLen+1)*sizeof(wchar_t));

			MultiByteToWideChar(CP_UTF8, 0, strHtml.c_str(), -1, pUnicode, unicodeLen);
			strResult.Format(_T("%s"), pUnicode);

			SAFE_DELETE_ARRAY(pUnicode);
			if (pFile != NULL)
			{
				pFile->Close();
				delete pFile;
				pFile = NULL;
			}
		}
	}
	catch (...)
	{
		SAFE_DELETE_ARRAY(pUnicode);
		if (pFile != NULL)
		{
			pFile->Close();
			delete pFile;
			pFile = NULL;
		}

		return false;
	}


	return true;
}

bool CHttpOperation::HttpPostGetRequestEx(const int nType, const char* pData, CString& strResult, CString strHeader)
{
	if (NULL == m_pConnection)
	{
		return false;
	}

	wstring HTTP_HEADER = strHeader.GetString();
	CHttpFile*	pFile = NULL;
	WCHAR*	pUnicode = NULL;

	int nHttpType = CHttpConnection::HTTP_VERB_POST;
	if (0 == nType)
	{
		nHttpType = CHttpConnection::HTTP_VERB_POST;
	}
	else if (1 == nType)
	{
		nHttpType = CHttpConnection::HTTP_VERB_GET;
	}
	else
	{	// error
		return false;
	}

	try
	{
		pFile = m_pConnection->OpenRequest(nHttpType, m_strDir);
		int nDataLen = strlen(pData);
		if (pFile != NULL && pFile->SendRequest(HTTP_HEADER.c_str(), (DWORD)HTTP_HEADER.length(), (LPVOID*)pData, nDataLen))
		{
			string strHtml;
			char szChars[1025] = {0};
			while ((pFile->Read((void*)szChars, 1024)) > 0)
			{
				strHtml += szChars;
			}
			int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strHtml.c_str(), -1, NULL, 0);
			pUnicode = new WCHAR[unicodeLen + 1];
			memset(pUnicode, 0, (unicodeLen+1)*sizeof(wchar_t));

			MultiByteToWideChar(CP_UTF8, 0, strHtml.c_str(), -1, pUnicode, unicodeLen);
			strResult.Format(_T("%s"), pUnicode);

			SAFE_DELETE_ARRAY(pUnicode);

			if (pFile != NULL)
			{
				pFile->Close();
				delete pFile;
				pFile = NULL;
			}
		}
	}
	catch (CException *e)
	{
		wchar_t strMsg[255];
		e->GetErrorMessage(strMsg,255);
		wstring wstrMsg(strMsg);
		
		return false;
	}

	return true;
}




CString CHttpOperation::UploadPic(CString strPicName, CString strIP, int nPort, CString strRemote)
{
	CString strErrInfoReturn = _T("");
	CInternetSession Session;

	Session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, m_nReceiveTimeOut);
	Session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, m_nServerTimeOut);
	CHttpConnection *pHttpConnection = NULL;
	//CFile fTrack;
	CFile fTrack;
	CFileException ex;
	CHttpFile* pHTTP = NULL;
	CString strHTTPBoundary;
	CString strPreFileData;
	CString strPostFileData;
	DWORD dwTotalRequestLength = 0;
	DWORD dwChunkLength = 0;
	DWORD dwReadLength = 0;
	DWORD dwResponseLength = 0;
	TCHAR szError[MAX_PATH] = {0};
	void* pBuffer = NULL;
	LPSTR szResponse;
	CString strResponse;
	BOOL bSuccess = TRUE;
	CString strDebugMessage;
	//读取文件
	if (!fTrack.Open(strPicName, CFile::modeRead|CFile::shareDenyWrite, &ex))//if (FALSE == fTrack.Open(strPicName, CFile::modeRead | CFile::shareDenyWrite))
	{
		//CLIENT_ERROR("Open File Failed path = %s", strPicName);
		return _T("文件打开失败");
	}
	int iRecordID = 1;
	strHTTPBoundary = _T("---------------------------7b4a6d158c9");//定义边界值
	CString pcmname = strPicName;
	pcmname = pcmname.Mid(pcmname.ReverseFind('\\') + 1);//获取抓取的图片名字
	

	USES_CONVERSION; 


	//PREVIEW_INFO("获取的文件名：%s ",pcmname);

	strPreFileData = MakePreFileData(strHTTPBoundary, pcmname, iRecordID);
	strPostFileData = MakePostFileData(strHTTPBoundary);
	dwTotalRequestLength = strPreFileData.GetLength() + strPostFileData.GetLength() + fTrack.GetLength();//计算整个包的总长度


	dwChunkLength = 64 * 1024;
	pBuffer = malloc(dwChunkLength);
	if (NULL == pBuffer)
	{
		//PREVIEW_ERROR("Fun_UploadPic 申请内存失败 长度 = %d", dwChunkLength);
		fTrack.Close();
		return _T("内存不足");
	}

	try
	{
		pHttpConnection = Session.GetHttpConnection(strIP,(INTERNET_PORT)nPort);
		pHTTP = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, strRemote);
		if(NULL==pHTTP)
			return _T("服务器连接失败");
		pHTTP->AddRequestHeaders(MakeRequestHeaders(strHTTPBoundary));//发送包头请求

		pHTTP->SendRequestEx(dwTotalRequestLength, HSR_SYNC | HSR_INITIATE);

#ifdef _UNICODE
		pHTTP->Write(W2A(strPreFileData), strPreFileData.GetLength());
#else
		pHTTP->Write((LPSTR)(LPCSTR)strPreFileData, strPreFileData.GetLength());
#endif

		dwReadLength = -1;
		while (0 != dwReadLength)
		{
			strDebugMessage.Format(_T("%u / %u\n"), fTrack.GetPosition(), fTrack.GetLength());

			TRACE(strDebugMessage);
			dwReadLength = fTrack.Read(pBuffer, dwChunkLength);
			if (0 != dwReadLength)
			{
				pHTTP->Write(pBuffer, dwReadLength);//写入服务器本地文件，用二进制进行传送
			}
		}

#ifdef _UNICODE
		pHTTP->Write(W2A(strPostFileData), strPostFileData.GetLength());
#else
		pHTTP->Write((LPSTR)(LPCSTR)strPostFileData, strPostFileData.GetLength());
#endif

		pHTTP->EndRequest(HSR_SYNC);
		dwResponseLength = pHTTP->GetLength();
		while (0 != dwResponseLength)
		{
			szResponse = (LPSTR)malloc(dwResponseLength + 1);
			szResponse[dwResponseLength] = '\0';
			pHTTP->Read(szResponse, dwResponseLength);
			strResponse += szResponse;
			free(szResponse);
			dwResponseLength = pHTTP->GetLength();
		}
	}
	catch (CException* e)
	{
		e->GetErrorMessage(szError, MAX_PATH);
		e->Delete();
		//AfxMessageBox(szError);
		bSuccess = FALSE;
		strErrInfoReturn = _T("上传请求返回失败");
	}

	pHTTP->Close();
	delete pHTTP;

	fTrack.Close();

	if (NULL != pBuffer)
	{
		free(pBuffer);
	}

	return strErrInfoReturn;
}


CString CHttpOperation::MakeRequestHeaders(CString& strBoundary)
{
	CString strFormat;
	CString strData;
	strFormat = _T("Content-Type: multipart/form-data; boundary=%s\r\n");
	strData.Format(strFormat, strBoundary);
	return strData;
}

CString CHttpOperation::MakePreFileData(CString &strBoundary, CString &strFileName, int iRecordID)
{
	/**/////////////////////////////////////////////////////////////////////////////////
	//Content-Type:
	//JPG image/pjpeg
	//PNG image/x-png
	//BMP image/bmp
	//TIF image/tiff
	//GIF image/gif
	CString strFormat;
	CString strData;

	strFormat += _T("--%s");
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"file\"; filename=\"%s\"");

	strFormat += _T("\r\n");
	strFormat += _T("Content-Type: image/pjpeg");

	strFormat += _T("\r\n\r\n");
	strData.Format(strFormat,  strBoundary, strFileName);//

	return strData;
}

CString CHttpOperation::MakePostFileData(CString &strBoundary)//发送请求包
{
	CString strFormat;
	CString strData;
	strFormat = _T("\r\n");	
	strFormat += _T("--%s");
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"upload\"");
	strFormat += _T("\r\n\r\n");
	strFormat += _T("hello");
	strFormat += _T("\r\n");
	strFormat += _T("--%s--");
	strFormat += _T("\r\n");
	strData.Format(strFormat, strBoundary, strBoundary);
	return strData;
}

bool    CHttpOperation::SetInternetTimeOut(int nServerTimeOut, int nReceiveTimeOut)
{
	m_nServerTimeOut = nServerTimeOut;
	m_nReceiveTimeOut = nReceiveTimeOut;

	return true;
}

bool  CHttpOperation::HttpGetFile( CString strFilePath)
{
	CString strIP = m_strIp;
	int nPort=  m_nPort;
	CString strRemote = m_strDir;

	CString strFileName;
	strFileName.Format(_T("http://%s:%d/%s"), strIP, nPort, m_strDir);

	CInternetSession sess;
	sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, m_nReceiveTimeOut);
	sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, m_nServerTimeOut);
	CHttpFile* pHttpFile;
	try
	{
		pHttpFile=(CHttpFile*)sess.OpenURL(strFileName);
	}
	catch(CException* e)
	{
		pHttpFile = 0;
		throw;
	}
	if(pHttpFile)
	{
		DWORD dwStatus;
		DWORD dwBuffLen = sizeof(dwStatus);
		BOOL bSuccess = pHttpFile->QueryInfo(HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, &dwStatus, &dwBuffLen);
		char strarray[1024];
		if( bSuccess && dwStatus>= 200&& dwStatus<300 )
		{
			CFile NetFile;
			DWORD dwFileLen = 0,dwIndex = 0;
			dwFileLen = pHttpFile->SeekToEnd();
			pHttpFile->SeekToBegin();
			if(NetFile.Open(strFilePath, CFile::modeWrite|CFile::modeCreate))
			{
				int nCount = 0;
				while(dwIndex<dwFileLen)
				{
					nCount = pHttpFile->Read(strarray,1023);
					NetFile.Write(strarray,nCount);
					dwIndex += nCount;                
				}
				NetFile.Close();
				AfxMessageBox(L"下载完毕");
			}
			else
			{
				AfxMessageBox(L"本地文件"+strFilePath+"打开出错.");
				return false;
			}
		}
		else
		{
			CString strSentence;
			strSentence.Format(L"打开网页文件出错，错误码：%d", dwStatus);
			AfxMessageBox(strSentence);
			return false;
		}
		pHttpFile->Close();
		delete pHttpFile;
	}
	else
	{
		AfxMessageBox(L"不能找到网页文件！");
	    return false;
	}

	sess.Close();
	return true;

	
};