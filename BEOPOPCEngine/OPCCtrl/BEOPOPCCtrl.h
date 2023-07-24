//OPCCtrl.h file


/********************************************************************************
*COPCCtrl.h - declare the class COPCCtrl
*
*       Copyright (c) WeldTeh Corporation. All rights reserved.
*
*Purpose:
*       �ṩ��PLC������ֱ�ӷ���.
*       ����дֵ��PLC��
		��PLC��ֵ������ʵʱ��Ҫ��ϸߵĲ�����
		��OPC���ػ����ֵ����������ֵ�п��ܲ������µġ�ֻ�Ǳ��ػ����ֵ��
*		
		Ϊ�����豸�Ŀ��أ�Ƶ�ʵ�����ֵ�ṩ��ݵĽӿ�.
*Note:
*       
*s
*Author:
*     
*******************************************************************************/
#ifndef Beop_OPCCTRL_H
#define Beop_OPCCTRL_H
#pragma once

#include <sstream>

#include "OPC_Connection.h"
#include "Tools/CustomTools/CustomTools.h"
#include <map>
#include <hash_map>

class COPCConnection_AutoBrowse : public COPCConnection
{
public:
	/*
	 ��������OPC Server������������OPC���������ص�group
	 strProgID :��OPC Server�ĳ��򼯵�����
	 strremotename�� OPC server���е�������(ipҲ��)
	 */
	virtual bool Init(wstring strOPCServerIP,bool bDisableCheckQuality=false,int nOPCClientIndex = -1,int nUpdateRate = 500);
	bool	Exit();	
	bool	ReConnectOPC();
	void	SetDebug(int nDebug);
	void	SetPointList( const vector<OPCDataPointEntry>& pointlist );
	vector<OPCDataPointEntry> GetPointList() const;
	void	ClearPointList();
	void	OutDebugInfo(const wstring strOut,bool bDaily=false);
	bool	ReloadErrorItemsHandle();
	int		GetPosition(const wstring shortname);
public:
	bool	m_bOPCConnect;
	wstring m_strOPCServerIP;
	int		m_nOPCClientIndex;
	int		m_nOutPut;			//1�������Ϣ
	Project::Tools::Mutex		m_lock;		//���ڲ�������
	bool	m_bDisableCheckQuality;			//��������Ʒ�����ݼ��
	int		m_nUpdateRate;			//����������������ˢ����
public:
		
	//ģ�庯��.ͨ��ӳ������
	template <typename T>
	bool	GetValue(const wstring& strName, T& result, bool bFromDevice = false);
	
	//ͨ��plc��������PLC��ȡֵ��
	//���������ͨ�����,��һ���Ӷ�����ȫ����ӳ��
	template <typename T>
	bool    ReadValue(const wstring& strName, T& result);
	
		
	//ͨ��ӳ��������PLCд�������ֵ.
	template<typename T>
	bool SetValue(const wstring& strName, const T& uiParam);
	
protected:
	//ģ�庯��.ͨ����ַ��
	template <typename T>
	bool	GetValueByLongName(const wstring& strName, T& result, bool bFromDevice = false);

	//ͨ��PLCȫ������PLC��ֵ
	// ������ һ����NewResource.Main.��ͷ
	// ������NewResource.��ͷ
	template<typename T>
	bool	ReadValueByLongName(const wstring& strLongName, T& result);

	//ͨ��������ʵ�ʵ�ַ����PLCд�������ֵ.
	template<typename T>
	bool	SetValueByLongName(const wstring& strname, const T& value);

	wstring	GetPlcNameByShortName(const wstring& shortname);

	void	AddErrorLog(const wstring& shortname);

	bool	ReLoadItemsHandle(vector<wstring> vecItemID);

public:
	
	VARTYPE	GetItemTypeByShortName(const wstring& strShortName);
		
	LPCTSTR GetItemQualityByShortName(const wstring& strShortName);
		
protected:
	vector<OPCDataPointEntry> m_pointlist;
	hash_map<wstring, wstring>	m_namemap;			//ӳ������plc��ַ�Ĺ���
	hash_map<wstring, int>	m_Positionmap;		//ӳ������ȡλ�Ĺ���
};

template<typename T>
bool COPCConnection_AutoBrowse::SetValue( const wstring& strName, const T& uiParam )
{
	wstring strLongName = GetPlcNameByShortName(strName);
	if (strLongName.empty()){
		AddErrorLog(strName);
		return false;
	}
	return SetValueByLongName(strLongName, uiParam);
}


template <typename T>
bool COPCConnection_AutoBrowse::GetValue( const wstring& strName, T& result , bool bFromDevice)
{
	wstring strLongName = GetPlcNameByShortName(strName);
	if (strLongName.empty()){
		_tprintf(L"Error::PlcName empty.\r\n");
		return false;
	}
	return GetValueByLongName(strLongName, result, bFromDevice);
}

template <typename T>
bool COPCConnection_AutoBrowse::ReadValue( const wstring& strName, T& result )
{
	wstring strLongName = GetPlcNameByShortName(strName);
	if (strLongName.empty()){
		AddErrorLog(strName);
		return false;
	}
	return ReadValueByLongName(strLongName, result);
}


//--------------------------------------------------------------------
template<typename T>
bool COPCConnection_AutoBrowse::SetValueByLongName( const wstring& strname, const T& value )
{
	std::wostringstream stream_T;
	stream_T << value ;

	return SetItemValue(strname, stream_T.str());
}

template <typename T>
bool COPCConnection_AutoBrowse::GetValueByLongName( const wstring& strName, T& result , bool bFromDevice)
{
	wstring value;
	
	if (!GetItemValue(strName, value,bFromDevice)){
		return false;
	}

	std::wistringstream t_stringstream(value);
	t_stringstream >> result;
	return true;
}


template<typename T>
bool COPCConnection_AutoBrowse::ReadValueByLongName( const wstring& strLongName, T& result )
{
	wstring str_value(_T(""));
	//����ֵ
	if (!UpdateItemValue(strLongName, str_value)){
		return false;
	}
	
	//ת��ΪT����.
	std::wistringstream t_stringstream(str_value);
	t_stringstream >> result;

	return true;
}

////////////////////////
class COPCConnection_ManualBrowse: public COPCConnection_AutoBrowse
{
public:
    
	virtual bool AddItems();

	virtual bool GetItemValue(const wstring& strItemID, wstring &strValue, bool bFromDevice = false);

	virtual bool SetItemValue(const wstring& strItemID, wstring &strValue);

	virtual bool GetMutilItemValue(map<wstring,OPC_VALUE_INFO>& mapItem);

	virtual bool	RefreshAsync20(bool bDeviceRead);
};


#endif  //_OPCCTRL_H
