

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

	//ȡ�������2���ӵĲ�����¼
	static bool GetLatestOperationRecord(string& buffer, int internalminutes = 2, int internalseconds = 30);

	//ȡ�������10���ӵ��Ż���������


};

