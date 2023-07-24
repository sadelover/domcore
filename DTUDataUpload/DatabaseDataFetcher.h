

/*
 *this class will define two methods:
 *
 *1. read operation record in the database.
 *
 *2. read optimize result data in the database.
 *
 *
 */
#pragma once

#include <string>
using std::string;

#include "DB_BasicIO/RealTimeDataAccess.h"





class CDatabaseDataFetcher
{
public:
	
	void	Init();

	//取得最近的2分钟的操作记录
	static bool GetLatestOperationRecord(string& buffer, int internalminutes = 2, int internalseconds = 30);

	//取得最近的10分钟的优化计算结果。


};

