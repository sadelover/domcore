#include "StdAfx.h"
#include "DatabaseDataFetcher.h"

using Beopdatalink::DTUDBAccessSingleton;

bool CDatabaseDataFetcher::GetLatestOperationRecord( string& buffer, int internalminutes /*= 2*/, int internalseconds)
{
	if (!DTUDBAccessSingleton::GetInstance()->IsConnected()){
		return false;
	}
	COleDateTimeSpan timespan;
	timespan.SetDateTimeSpan(0,0,internalminutes,internalseconds);
	COleDateTime currenttime = COleDateTime::GetCurrentTime();
	COleDateTime starttime = currenttime - timespan;

	vector<Beopdatalink::optrecordfordtu> recordlist;
	bool bresult = DTUDBAccessSingleton::GetInstance()->GetOperationRecordAsString(recordlist, starttime, currenttime);
	if (bresult)
	{
		for (unsigned int i = 0; i < recordlist.size(); i++)
		{
			const Beopdatalink::optrecordfordtu& cpr = recordlist[i];
			string temp = "('";
			temp += cpr.time;
			temp += "','";
			temp += cpr.username;
			temp += "','";
			temp += cpr.info;
			temp += "');";
			buffer += temp;
		}
		return true;
	}

	return false;
}
