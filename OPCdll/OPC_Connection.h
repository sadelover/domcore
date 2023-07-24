#pragma once

#include "OPCdefine.h"

#include <hash_map>
using namespace stdext;

#define WRITE_SYNC		0
#define WRITE_ASYNC10	1
#define WRITE_ASYNC20	2

#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataPoint/DataPointEntry.h"

struct OPC_VALUE_INFO
{
	wstring strOPCName;
	wstring strOPCValue;
	VARTYPE vType;
	double	dMultiple;
	wstring	wstrMultiple;		//����
	int		nPosition;
	DWORD	nIndex;
	bool	bUpdate;
};

// using namespace Project::Logs;
class COPCConnection
{
public:
	COPCConnection();
	~COPCConnection(void);

private:
	void GetValue(VARIANT &vtVal		// [in]
		        , TCHAR *pBuffer				// [out]
		        , int nBufLen);					// [in]
        
	void AddLeaves(LPENUMSTRING pIEnumString);

public:
    //�������Ӳ���
	void	SetServerProgName(const wstring& strProgID);
	wstring	GetServerProgName() const;

	//-----------------------------------------------------------------
    //���ӵ�OPC������
	bool Connect(wstring strOPCServerIP,int nOPCClientIndex = -1);
    
    //��opc�������Ͽ�
	void DisConnect(void);

	//����OPC
	bool ReConnect();

	//��Զ�̼����ͨ���������ƻ�ȡCLSID
	bool	GetCLSIDByProgID(wstring strOPCServerIP,wstring strProgID,CLSID& clsid);

    //���OPC group
	virtual bool AddGroup(int nUpdateRate = 500,int nLanguageID=0);

    //�������opc����
	virtual bool AddItems(int nMultilAddItems =1000);
	
	//------------------------------------------------------------------------------
    virtual bool GetItemValue(const wstring& strItemID, wstring &strValue, bool bFromDevice = false);

	virtual bool GetMutilItemValue( map<wstring,OPC_VALUE_INFO>& mapItem );
    
    bool	UpdateItemValue(const wstring& strItemID, wstring &strValue);
		
	virtual bool SetItemValue(const wstring& strItemID, const wstring& strValue);
	

	virtual bool	RefreshAsync20(bool bDeviceRead);

	VARTYPE GetVarType(const wstring& strItemName);
	LPCTSTR	GetVarQuality(const wstring& strItemName);
	bool	GetItemValueBuffer(const wstring& strItemName, wstring &strValue);		//ͨ��ItemName ��ֵ
	bool	GetUpdateTime20(COleDateTime& oleUpdate,bool& bUpdateTimeAsync20, bool bOutAsync20 = true);					//��ȡ�첽���ݸ���ʱ��
	CKItem* FindItem(const wstring& strItemName);
    void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
public:
	
	OPCSERVER_TYPE	GetServerType()const;
	bool		IsConnect()const ;
	
protected:
	wstring		m_strserverprogid;
	wstring		m_strserverhost;

	wstring		m_strFilterLeaf;
	BOOL		m_bBrowseFlat;
	
	CObArray	m_itemlist;	//�������е�Item
    
    hash_map<wstring, CKItem*> m_searchmap;    //��������item�ļ�����
	hash_map<wstring, DataPointEntry> m_searchEntrymap;    //��������item entity�ļ�����
    
	CKServer	*m_pServer;     // server����
	CKGroup		*m_pGroup;      //group����
	bool			m_isconnected;
	bool		m_bAddItemSuccess;		//����Ҫ���һ������
	bool		m_bAddGroupSuccess;		//�����ɹ�
    Project::Tools::Mutex m_lock;
	
};