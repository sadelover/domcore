#pragma once



/*
 *
 * this server handler will insert the data into the database.
 *
 */

#include "DataHandler.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "DTUAddressMap.h"

class CDTUServerHandler
{
public:
	CDTUServerHandler();
	~CDTUServerHandler(void);


	static UINT WINAPI ThreadFunc(LPVOID lparam);			//�����յ�����
	static UINT WINAPI ThreadSendCmd(LPVOID lparam);
	static UINT WINAPI ThreadUpdateInput(LPVOID lparam);
	static UINT	WINAPI ThreadUpadateHistory(LPVOID lparam);		//��ʷ���ݿ��¼

	bool	GetExitThread_History() const;

public:
	void	SetDataEvent();
	void	ResetDataEvent();

	void	HandleData();
	void	SendData();

	void	UpdateInput();

	bool	Init(mapentry entry);
		
	void	SetData(const char* rawbuffer, const int rawsize, const char* id);
	bool	IsBusy() const;

	string	GetDTUHandleName();

	//DTUÿ���ֻ�ܴ���1024���ֽڡ���ȫ�����ÿ�����ֻ����512���ֽڵ����ݡ�
	//��Ҫ�����
	void BuildPackage(const string& buffer, vector<string>& resultlist);

	bool	UpdateOutput(const string& strtime,const string& pointname,const string& pointvalue);

	void	UpdateDTUEntryList(const char* buffer);
	void	AnzlyzeDTUData(const char* buffer);


	void	AnalyzeRealData(const char* buffer);
	void	AnalyzeWarningData(const char* buffer);
	void	AnalyzeWarningOperation(const char* buffer);

	 void   SplitString(const std::string& source, const char* sep, std::vector<string>& resultlist);

	void	GetDataEntryList(vector<spointwatch> &entrylist,const POINT_STORE_CYCLE &tCycle);
	void	GetDataEntryList(vector<spointwatch> &entrylist);
	void	UpdateHistoryTable(const POINT_STORE_CYCLE &tCycle);

	HANDLE	GetUpdateHistoryEvent();
	bool	UpdateHistoryData();
	bool	SetUpdateHistoryEventAndType(int nUpdateHistoryType);

	bool	GetIfUpdateData();
	bool	SetUpdateData(bool bUpdateData);

	bool	InitRealDataFromInput();

	bool	AddLog(CString strLog);
	CString GetLog();

private:
	static const int BUFFERSIZE = 6000;
	static const int NAMESIZE = 12;
	char	m_rawbuffer[BUFFERSIZE];
	int		m_rawbuffersize;
	char	m_id[NAMESIZE];
		
	HANDLE m_datathread;
	HANDLE m_cmdthread;
	HANDLE m_sendthread;
	HANDLE m_hUpdateHistoryThread;
	HANDLE m_event;
	HANDLE m_eUpdateHistory;

	CDataHandler m_dl;

	Project::Tools::Mutex m_lock;
	
	//DTUAddressMap m_addressmap;
	bool	m_isbusy;

	COleDateTime m_tLastHistorySaved;
	COleDateTime m_tLastReceiveData;
	bool	m_bexitthread_history;
	hash_map<string,spointwatch> m_mapPoint;
	hash_map<string,warning>	m_mapWarning;
	hash_map<string,warningoperation>	m_mapWarningOpeartion;
	int		m_nUpdateHistoryType;			//0:ʵʱ����   1����������  2��������������
	bool		m_bUpdateData;
	CString		m_strLog;
	int			m_nLogCount;
};

