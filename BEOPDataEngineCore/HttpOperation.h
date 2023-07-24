#pragma once

#include <afxinet.h>

class CHttpOperation
{
public:
	CHttpOperation(const CString strIp, const int nPort, const CString strDir);
	~CHttpOperation(void);

public:
	bool	HttpPostGetRequest(CString& strResult);
	bool	HttpPostGetRequestEx(const int nType, const char* pData, CString& strResult, CString strHeader = _T("Content-Type: application/x-www-form-urlencoded"));

	CString UploadPic(CString strPicName, CString strIP, int nPort, CString strRemote);

	bool    HttpGetFile(CString strFilePath);
	bool    SetInternetTimeOut(int nServerTimeOut, int nReceiveTimeOut);

private:
	bool	CreateConnection(const CString strHost, const INTERNET_PORT nPort);	//创建Http Post连接
	bool	CloseConnection(void);												//关闭连接




	CString MakeRequestHeaders(CString& strBoundary);

	CString MakePreFileData(CString &strBoundary, CString &strFileName, int iRecordID);
	CString MakePostFileData(CString &strBoundary);//发送请求包


	CHttpConnection*	m_pConnection;
	CInternetSession	m_Isession;

	CString				m_strIp;
	int					m_nPort;
	CString				m_strDir;

	int m_nServerTimeOut;
	int m_nReceiveTimeOut;
};

