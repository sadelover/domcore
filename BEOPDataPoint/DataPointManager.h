
/*
 this class define the plc data point.
*/



#ifdef WIN32
	#pragma once
#endif

#ifndef CPLCDATAPOINT_H__
#define CPLCDATAPOINT_H__

#include "../Tools/DllDefine.h"
#include "DataPointEntry.h"


#pragma warning(disable:4251)
#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::wstring;
using std::string;


//////////////////////////////////////////////////////////////////////////

class DLLEXPORT CDataPointManager
{
public:
	enum enumPointSavePeriod
	{
		PS_MINUTE,
		PS_HOUR,
		PS_DAY,
		PS_MONTH,
		PS_YEAR
	};

	// constructor
	CDataPointManager();

	//destructor
	~CDataPointManager();
    
	bool	Init(vector<wstring> & ignorePointList);
public:

	void    AddVPoint(wstring wstrPointName);
	void    RemoveVPoint(wstring wstrPointName);
	
	void	Clear();
	void	ClearOPCList();
	void	ClearAllPoint();
	void	ClearAllPointListExceptOPC();
	void	AddEntry(const DataPointEntry& pentry);
	void	ParsePointlist();
	void    SetAllPointTypeAsVirtual();

	void	GetOpcPointList(vector<DataPointEntry>& opclist);
	void	GetModbusPointList(vector<DataPointEntry>& modbuspointlist);
	void	GetCO3PPointList(vector<DataPointEntry>& co3ppointlist);
	void	GetFCBusPointList(vector<DataPointEntry>& fcbuspointlist);
	void	GetBacnetPointList(vector<DataPointEntry>& bacnetpointlist);
	void	GetMemoryPointList(vector<DataPointEntry>& memorypointlist);
	void	GetProtocol104PointList(vector<DataPointEntry>& protocol104pointlist);
	void	GetMysqlPointList(vector<DataPointEntry>& mysqlpointlist);
	void	GetWebPointList(vector<DataPointEntry>& webpointlist);
	void	GetSqlServerPointList(vector<DataPointEntry>& sqlpointlist);
	void	GetSqlitePointList(vector<DataPointEntry>& sqlitepointlist);
	void    GetSiemensPointList(vector<DataPointEntry>& ptList);

	void    GetModbusEquipmentPointList(vector<DataPointEntry>& ptList);
	void    GetCoreStationPointList(vector<DataPointEntry>& ptList);
	void    GetKNXPointList(vector<DataPointEntry>& ptList);
	void    GetDLT645PointList(vector<DataPointEntry>& ptList);
	void    GetThirdPartyPointList(vector<DataPointEntry>& ptList);
	void    GetPersagyControllerPointList(vector<DataPointEntry>& ptList);
	void    GetOPCUAControllerPointList(vector<DataPointEntry>& ptList);
	void    GetBacnetpyPointList(vector<DataPointEntry>& ptList);
	void    GetObixPointList(vector<DataPointEntry>& ptList);
	void    GetLogixPointList(vector<DataPointEntry>& ptList);
	void    GetMoxaTCPServerPointList(vector<DataPointEntry>& ptList);
	void    GetABSLCPointList(vector<DataPointEntry>& ptList);

	int    GetOPCPointCount();
	int    GetModbusPointCount();
	int    GetCO3PPointCount();
	int    GetFCbusPointCount();
	int    GetBacnetPointCount();
	int    GetMemoryPointCount();
	int    GetProtocal104PointCount();
	int    GetMySQLPointCount();
	int    GetWebPointCount();
	int    GetSqlServerPointCount();
	int    GetSqlitePointCount();
	int		GetSiemensPointCount();
	int	   GetAllPointCount();




	const vector<DataPointEntry>& GetPointList() const;

	void	SetFilePath(const wstring& filepath);

	DataPointEntry	GetEntry(const wstring& shortname);

	POINT_STORE_CYCLE GetPointSavePeriod(const string & pointname);

	POINT_STORE_CYCLE GetMinCycle();

	map<int, DataPointEntry>& GetPointMap();

	void	SetOPClientIndex(const wstring & pointname,int nIndex);
	void	OutDuplicateEntry(const CString strOut);

private:

	//===================map for pointindex and entry
	vector<DataPointEntry>  m_pointlist;
	
	//=================== map for long name and pointindex
	vector<DataPointEntry>	 m_opcpointlist;
	
	//===================map for short name and pointindex
	vector<DataPointEntry>	 m_modbuspointlist;

	//===================map for short name and pointindex
	vector<DataPointEntry>	 m_co3ppointlist;

	//===================map for short name and pointindex
	vector<DataPointEntry>	 m_fcbuspointlist;
    
	//=================== map for short name and long name.
	vector<DataPointEntry>  m_bacnetpointlist;

	//=================== map for short name and long name.
	vector<DataPointEntry>  m_protocol104pointlist;

	//=================== map for short name and long name.
	vector<DataPointEntry>  m_mysqlpointlist;

	vector<DataPointEntry>  m_webpointlist;

	vector<DataPointEntry>  m_sqlserverpointlist;

	vector<DataPointEntry>  m_sqlitepointlist;

	//=================== map for short name and long name.
	vector<DataPointEntry>  m_siemenspointlist;

	//===================
	vector<DataPointEntry>	m_memorypointlist;

	vector<DataPointEntry>  m_persagyControllerPointlist;
	vector<DataPointEntry>  m_opcuaPointlist;
	vector<DataPointEntry>  m_thirdpartyPointlist;
	vector<DataPointEntry>  m_modbusEquipmentPointlist;
	vector<DataPointEntry>  m_bacnetpyPointlist;
	vector<DataPointEntry>  m_obixPointlist;
	vector<DataPointEntry>  m_logixPointlist;
	vector<DataPointEntry>  m_ABSLCPointList;
	vector<DataPointEntry>  m_moxaTCPServerPointlist;
	vector<DataPointEntry>  m_coreStationPointList;
	vector<DataPointEntry>  m_KNXPointList;
	vector<DataPointEntry>  m_DLT645PointList;

	map<wstring, DataPointEntry>	m_namemap;

	map<string,wstring>		m_mapDuplicate;				//属性重复点位： Key : 	点类型+参数1-7
	map<CString,wstring>		m_mapBigLowCaseMap;
	
	bool	m_ifparsed;

	string		m_filepath;

	POINT_STORE_CYCLE m_nMinTCycle;

	CString					m_strDuplicatePath;
};

#endif