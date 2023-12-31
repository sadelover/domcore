// -*- c++ -*-
// ***********************************************************
//
// Copyright (C) 2013 R&B
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Module:   
// Author:   david
// Date:     2010/7/13
// Revision: 1.0
//
// ***********************************************************


#include "stdafx.h"
#include "DBInterfaceExp.h"

#include "DBAccessContainer.h"
#include "comdef.h"
#include "assert.h"

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <vector>

#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/Scope_Ptr.h"
#include "../Tools/Util/UtilString.h"

#include "MysqlDeleter.h"


using namespace Project::Tools;
using std::ostringstream;


//设备开关表名
const TCHAR* DEVICE_ONOFF_POINT_LIST_TABLE = _T("Device_OnOff_Point_List"); 

//设备频率表名
const TCHAR* DEVICE_FREQ_POINT_LIST_TABLE = _T("Device_Freq_Point_List");

//功率记录表
const TCHAR* POWER_POINT_LIST_TABLE = _T("Power_Point_List");

//流量记录表
const TCHAR* FLOW_POINT_LIST_TABLE = _T("Flow_Point_List");

//温度记录表。
const TCHAR* TEMPERATURE_POINT_LIST_TABLE = _T("Temperature_Point_List");

//压差记录表。
const TCHAR* PRESSUREDIFFERENCE_POINT_LIST_TABLE = _T("PressureDifference_Point_List");

// 用电量记录表。
const TCHAR* POWERCONSUMTION_POINT_LIST_TABLE = _T("PowerConsumtion_Point_List");

// 操作记录表
const char* OPERATION_RECORD_TABLE = ("operation_record");

// 运行时间表
const TCHAR* EQM_RUNNINGTIME_POINT_LIST_TABLE = _T("RunningTime_Point_List");

// 月度报表
const char* MONTHREPORT_TABLE = ("MonthReport_List");

const char* NAMEPREFIX = "Beop";
const char* DATABASE_TEMPLATE = "Beop_template";

//排热量
const char* HEATLOAD_YEAR_TABLE = "HeatLoad_Year";
const char* HEATLOAD_MONTH_TABLE = "HeatLoad_Month";

#define STATEMENTSIZE 14
const char* AddPrimaryKeySql[STATEMENTSIZE] = {\
				"alter table chiller_plant_room_load_define add primary key(Time,OF_CPR_ID);",
				"alter table chillerplantroom add primary key(CPR_ID);",
				"alter table project add primary key(projectid);",
				"alter table room_contain_chiller add primary key(Chiller_ID_In_Room, OF_CPR_ID) ;",
				"alter table room_contain_coolingtower add primary key(Tower_ID_In_Room, OF_CPR_ID);",
				"alter table room_contain_corner add primary key(Corner_ID_InRoom, OF_CPR_ID);",
				"alter table room_contain_hx add primary key(Hx_ID_In_Room, OF_CPR_ID);",
				"alter table room_contain_pipe add primary key(Pipe_ID_In_Room, OF_CPR_ID);",
				"alter table room_contain_pump add primary key(Pump_ID_In_Room, OF_CPR_ID);",
				"alter table room_contain_tank add primary key(id_in_room, OF_CPR_ID);",
				"alter table room_contain_terminal add primary key(Terminal_ID_InRoom, OF_CPR_ID);",
				"alter table room_contain_valve add primary key(Valve_ID_In_Room, OF_CPR_ID);",
				"alter table room_contain_waternet add primary key(waternetid);",
				"alter table room_contain_watersoil add primary key(id);"};



DBAccessContainer * DBAccessContainer::g_DBAccessContainer = NULL;
Mutex DBAccessContainer::m_mutex;

//////////////////////////////////////////////////////////////////////////

void MakeUpper(wstring &s)
{
	for (wstring::iterator it = s.begin(); it != s.end(); it++)
	{
		*it = toupper(*it);
	}
}

struct CompareProject
{
	bool operator()(StartUpInfo* s1, StartUpInfo* s2)
	{
		assert( (s1 != NULL) && (s2 != NULL));
		wstring s1_comp = s1->projectdb.strprojectname;
		wstring s2_comp = s2->projectdb.strprojectname;
		MakeUpper(s1_comp);
		MakeUpper(s2_comp);
		return s1_comp.compare(s2_comp) < 0;
	}
};

struct CompareprojectDB
{
	bool operator()(Project_DB s1, Project_DB s2)
	{
		
		wstring s1_comp = s1.strprojectname;
		wstring s2_comp = s2.strprojectname;

		MakeUpper(s1_comp);
		MakeUpper(s2_comp);
		return s1_comp.compare(s2_comp) < 0;
	}
};



// 构造函数
DBAccessContainer::DBAccessContainer()
{

}

DBAccessContainer::~DBAccessContainer()
{	
}


DBAccessContainer * DBAccessContainer::GetInstance()
{
	//Scoped_Lock<Mutex> mut(m_recordmutex);
	if ( NULL == g_DBAccessContainer )
	{
        Scoped_Lock<Mutex> mut(m_mutex); //bird 3.4
        if (!g_DBAccessContainer){
            g_DBAccessContainer = new DBAccessContainer();
            atexit(DestroyInstance);
        }
	}
	return g_DBAccessContainer;
}

void DBAccessContainer::DestroyInstance()
{
	if ( NULL != g_DBAccessContainer )
	{
		delete g_DBAccessContainer;
		g_DBAccessContainer = NULL;
	}
}

bool DBAccessContainer::InsertWarningRecord(SYSTEMTIME st, const wstring& warninginfo)
{
    Project::Tools::Scoped_Lock<Mutex> scopelock(m_recordmutex);
	SetConnectionCharacterSet();
	
	ostringstream sqlstream;
	string recordtime = SystemTimeToString(st);
	
	string warninginfo_utf8;
	Project::Tools::WideCharToUtf8(warninginfo, warninginfo_utf8);
	
	sqlstream << "insert into warning_record values('" << recordtime << "', '" << warninginfo_utf8 << "')";

	bool bresult =  Execute(sqlstream.str());
	if (!bresult)
	{
		LogDBError(sqlstream.str());
	}
	return bresult;
}


//bool DBAccessContainer::InsertOperationRecord( SYSTEMTIME st, wstring user, SYSTEMOPERATIONS optindex)
//{
//	Scoped_Lock<Mutex> scopelock(m_recordmutex);
//	
//	string recordtime = Project::Tools::SystemTimeToString(st);
//	ostringstream sqlstream;
//	string user_ansi = Project::Tools::tstringToString(user);
//	sqlstream << "insert into operation_record values('" << recordtime << "'," << user_ansi << "," << optindex << ")";
//
//	bool bresult =  Execute(sqlstream.str());
//	if (!bresult)
//	{
//		LogDBError(sqlstream.str());
//	}
//
//	return bresult;
//}

bool DBAccessContainer::InsertOperationRecord( const SYSTEMTIME& st, const wstring& remark, const wstring& wstrUser )
{
    Project::Tools::Scoped_Lock<Mutex> scopelock(m_recordmutex);
    
    SetConnectionCharacterSet();
    
    const string strRemark = Project::Tools::WideCharToUtf8(remark.c_str());
    const string strUser = Project::Tools::WideCharToUtf8(wstrUser.c_str());

	ostringstream sqlstream;
	sqlstream	<< "insert into "
				<< GetDBName() << "."<<OPERATION_RECORD_TABLE <<" values('"				
				<< Project::Tools::SystemTimeToString(st) << "'"
                << "," <<"'"<< strUser << "'"
                << "," <<"'"<< strRemark << "'"
                << ")";

	string sql = sqlstream.str();
	bool bResult = Execute(sql);
	if (!bResult)
	{
		LogDBError(sql);
	}
	
	return bResult;
}


bool DBAccessContainer::DeleteUser( wstring username )
{
	string user = Project::Tools::tstringToString(username);
	ostringstream sqlstream;
	sqlstream << "delete from Beop_user.users where username = '" << user << "'";

	bool bresult = Execute(sqlstream.str());
	if (!bresult)
	{
		LogDBError(sqlstream.str());
	}

	return bresult;
}

bool DBAccessContainer::ReadAllUsers( vector<wstring>& userlist )
{
	string sql = "select username from Beop_user.users" ;
	
	if (!userlist.empty())
		userlist.clear();

	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
	{
		LogDBError(sql);
		return false;
	}

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


	MYSQL_ROW row;
	while(row = FetchRow(result))
	{
		wstring	username;
		String_To_Tstring(row[0], username);
		userlist.push_back(username);
	}

	return true;

}

bool DBAccessContainer::ReadUserInfo( wstring username, BeopUserInfo* puserinfo )
{
	assert (puserinfo != NULL);

	ostringstream sqlstream;
	string user = Project::Tools::tstringToString(username);
	sqlstream << "select readright, writeright, modelright, simulateright, lastlogintime from Beop_user.users where username = '" << user << "'" ;
	
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		LogDBError(sqlstream.str());
		return false;
	}

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


	MYSQL_ROW row;
	while(row = FetchRow(result))
	{
		puserinfo->username  = username;
		puserinfo->readright = ATOI(row[0]);
		puserinfo->writeright = ATOI(row[1]);
		puserinfo->modelright = ATOI(row[2]);
		puserinfo->simulateright = ATOI(row[3]);			
		String_To_Tstring(row[4], puserinfo->lastlogintime);
	}

	return true;
}

bool DBAccessContainer::ReadALLUserInfo( vector<BeopUserInfo*>& userlist )
{
	string sql = "select * from Beop_user.users";

	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
	{
		LogDBError(sql);
		return false;
	}

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


	MYSQL_ROW row;
	while (row = FetchRow(result))
	{
		BeopUserInfo* pinfo = new BeopUserInfo;
		String_To_Tstring(row[0], pinfo->username);
		pinfo->readright = ATOI(row[2]);
		pinfo->writeright = ATOI(row[3]);
		pinfo->modelright = ATOI(row[4]);
		pinfo->simulateright = ATOI(row[5]);
		String_To_Tstring(row[6], pinfo->lastlogintime); 
			
		userlist.push_back(pinfo);
	}
		
	return true;

}

bool DBAccessContainer::ValidateUser( wstring username, wstring password )
{
	ostringstream	sqlstream;
	string user = Project::Tools:: tstringToString(username);
	string pwd  = Project::Tools::tstringToString(password);

	sqlstream << "select 1 from Beop_user.users where username = '" << user << "' and userpwd = '" << pwd << "'";
	MYSQL_RES *result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	

		if (FetchRow(result) != NULL)
		{
			return true;
		}
	return true;
}

void DBAccessContainer::LogDBError(const char* sql)
{
	wstring sqlstatement;
	Project::Tools::String_To_Tstring(sql, sqlstatement);

	wstring mysqlerror;

	Project::Tools::String_To_Tstring(GetMysqlError(), mysqlerror);

	mysqlerror += L" while ";

	mysqlerror += sqlstatement;
		
}

void DBAccessContainer::LogDBError( const string& sql )
{
	LogDBError(sql.c_str());
}

//bird add, read all operation record
//直接查24h的数据, 在数据库查询排序
bool    DBAccessContainer::ReadAllOperationRecord(VEC_OP_RECORD&  vecOperation, const COleDateTime& timeStart, const COleDateTime& timeStop)
{
    Project::Tools::Scoped_Lock<Mutex> scopelock(m_recordmutex);

    SetConnectionCharacterSet();
    bool ifok = false;

    //const int hourMax = 24;
    wstring wstrStart, wstrStop;
    string strStart, strStop;
    Project::Tools::OleTime_To_String(timeStart, wstrStart);
    Project::Tools::OleTime_To_String(timeStop, wstrStop);
    strStart = UtilString::ConvertWideCharToMultiByte(wstrStart);
    strStop = UtilString::ConvertWideCharToMultiByte(wstrStop);

    ostringstream sqlstream;
    //SELECT * FROM operation_record where recordtime >= '2011-05-30 00:00:00' and recordtime <= '2011-05-30 14:00:00';
    sqlstream << "select * from " << OPERATION_RECORD_TABLE <<" where recordtime >= '"<< strStart.c_str() <<"' and recordtime <= '"<< strStop.c_str() << "'";

    string sql = sqlstream.str();
    COperationRecord  tempOpera;
    vecOperation.clear();

    COleDateTime old;

    MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
	if (result == NULL)
	{
		LogDBError(sql);
		return false;
	}

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	
    MYSQL_ROW row = NULL;
    while (row = mysql_fetch_row(result))
    {
        if (row[0] && row[1] && row[2]){
            //daterange.push_back(row[0]);
            tempOpera.m_strTime = UtilString::ConvertMultiByteToWideChar(row[0] );
            Project::Tools::UTF8ToWideChar(row[1],tempOpera.m_strUser); //bird add, 0630
            Project::Tools::UTF8ToWideChar(row[2],tempOpera.m_strOperation);

            vecOperation.push_back(tempOpera);
        }
    }

    return true;
}

vector<wstring> DBAccessContainer::ReadWarningConfigList()
{
	Scoped_Lock<Mutex>  dblock(m_recordmutex);
	vector<wstring>	vec_result;

	string sql = "select * from warning_config_list";

	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
	{
		LogDBError(sql);
		return vec_result;
	}

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	MYSQL_ROW row = NULL;
	while(row = FetchRow(result))
	{
		wstring shortname = Project::Tools::AnsiToWideChar(row[1]);
		if (!shortname.empty())
		{
			vec_result.push_back(shortname);
		}
	}
	
	return  vec_result;
}

bool DBAccessContainer::ReadWarningRecord( WARNING_LIST& warninglist, const COleDateTime& timebegin, const COleDateTime& timeend )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_recordmutex);
        
    SetConnectionCharacterSet();
	ostringstream sqlstream;
	wstring wstr_begin, wstr_end;
	Project::Tools::OleTime_To_String(timebegin, wstr_begin);
	Project::Tools::OleTime_To_String(timeend, wstr_end);

	string str_begin = Project::Tools::WideCharToAnsi(wstr_begin.c_str());
	string str_end = Project::Tools::WideCharToAnsi(wstr_end.c_str());

	sqlstream << "select recordtime, warninginfo from warning_record "
		<< " where recordtime > '"
		<< str_begin 
		<< "' and recordtime < '"
		<< str_end 
		<< "'";

	string sql = sqlstream.str();

	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
	{
		LogDBError(sql);
		return false;
	}

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


	CWarningRecord record;
	MYSQL_ROW row = NULL;
	while(row = FetchRow(result))
	{
		if (row[0] != NULL && row[1] != NULL ) {
			record.m_strTime = Project::Tools::AnsiToWideChar(row[0]);
			record.m_warninginfo = Project::Tools::UTF8ToWideChar(row[1]);
			warninglist.push_back(record);
		}
	}

	return true;
}               

bool DBAccessContainer::InsertWarningRecord( int warningindex)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_recordmutex);

    SetConnectionCharacterSet();

	ostringstream sqlstream;
	SYSTEMTIME st;
	GetLocalTime(&st);
	string recordtime = SystemTimeToString(st);
	sqlstream << "insert into warning_record values('" << recordtime << "'," << warningindex << ")";

	bool bresult =  Execute(sqlstream.str());
	if (!bresult)
	{
		LogDBError(sqlstream.str());
	}

	return bresult;
}
