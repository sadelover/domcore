#include "StdAfx.h"
#include "RedisManager.h"

#include "../redis/hiredis.h"
#include "../Tools/CustomTools/CustomTools.h"

CRedisManager::CRedisManager(void)
{
	m_bRedisValid = false;
}


CRedisManager::~CRedisManager(void)
{
}


bool CRedisManager::init(wstring wstrRedisIP, int nRedisPort)
{
	unsigned int j;
	redisContext *c;
	redisReply *reply;

	bool bRedisConnected = true;

	m_nRedisPort = nRedisPort;
	Project::Tools::WideCharToUtf8(wstrRedisIP, m_strRedisIP );

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(m_strRedisIP.data(), m_nRedisPort, timeout);
	if (c->err) {
		bRedisConnected=  false;
	}

	redisFree(c);

	m_bRedisValid = bRedisConnected;
	return bRedisConnected;
}

bool CRedisManager::is_valid()
{
	//Connect Redis
	return m_bRedisValid;
}

bool CRedisManager::get_value(wstring wstrKey, wstring &wstrValue)
{
	string strValue;
	if(get_value_string(wstrKey, strValue))
	{
		 Project::Tools::UTF8ToWideChar(strValue, wstrValue);
		return true;
	}


	return false;
}


bool CRedisManager::get_value_string(wstring wstrKey, string &strValue)
{
	unsigned int j;
	redisContext *c;
	redisReply *reply;

	bool bSuccess = false;

	bool bRedisConnected = true;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(m_strRedisIP.data(), m_nRedisPort, timeout);
	if (c->err) {
		bSuccess=  false;

	}
	else
	{

		string strKey_utf8;
		Project::Tools::WideCharToUtf8(wstrKey,  strKey_utf8);




		reply = (redisReply *)  redisCommand(c,"GET %s",strKey_utf8.data());
		if(!reply)
		{
			bSuccess = false;
		}
		else if(reply->str==NULL)
		{
			bSuccess = false;
		}
		else
		{

			strValue = reply->str;

			bSuccess = true;
		}
		freeReplyObject(reply);
	}


	redisFree(c);     

	return bSuccess;
}

bool CRedisManager::set_value(wstring wstrKey, wstring wstrValue)
{
	unsigned int j;
	redisContext *c;
	redisReply *reply;

	bool bSuccess = false;

	bool bRedisConnected = true;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(m_strRedisIP.data(), m_nRedisPort, timeout);
	if (c->err) {
		bSuccess=  false;

	}
	else
	{
		if(m_strPassword.length()>0)
		{

			reply = (redisReply *)  redisCommand(c,"AUTH %s",m_strPassword.data());
			if (reply->type == REDIS_REPLY_ERROR) {
				//printf("Redis认证失败！\n");
			}
		}
		string strKey_utf8, strValue_utf8;
		Project::Tools::WideCharToUtf8(wstrKey, strKey_utf8 );

		Project::Tools::WideCharToUtf8(wstrValue, strValue_utf8);
		reply = (redisReply *)  redisCommand(c,"SET %s %s",strKey_utf8.data(), strValue_utf8.data());
		freeReplyObject(reply);

		bSuccess = true;
	}


	redisFree(c);     

	return bSuccess;
}


bool CRedisManager::set_value_mul_utf8(vector<string> strKeyList, vector<string> strValueList)
{
	if(strKeyList.size()!= strValueList.size())
		return false;

	unsigned int j;
	redisContext *c;
	redisReply *reply;

	bool bSuccess = false;

	bool bRedisConnected = true;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(m_strRedisIP.data(), m_nRedisPort, timeout);
	if (c->err) {
		bRedisConnected=  false;

		bSuccess  = false;
	}
	else
	{
		if(m_strPassword.length()>0)
		{

			reply = (redisReply *)  redisCommand(c,"AUTH %s",m_strPassword.data());
			if (reply->type == REDIS_REPLY_ERROR) {
				//printf("Redis认证失败！\n");
			}
		}

		for(int i=0;i<strKeyList.size();i++)
		{
			reply = (redisReply *)  redisCommand(c,"SET %s %s",strKeyList[i].data(), strValueList[i].data());
			freeReplyObject(reply);

		}

		bSuccess = true;
	}

	redisFree(c);     
	return bSuccess;
}

bool CRedisManager::set_value_mul(vector<wstring> wstrKeyList, vector<wstring> wstrValueList)
{
	if(wstrKeyList.size()!= wstrValueList.size())
		return false;

	unsigned int j;
	redisContext *c;
	redisReply *reply;

	bool bSuccess = false;

	bool bRedisConnected = true;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(m_strRedisIP.data(), m_nRedisPort, timeout);
	if (c->err) {
		bRedisConnected=  false;

		bSuccess  = false;
	}
	else
	{
		if(m_strPassword.length()>0)
		{

			reply = (redisReply *)  redisCommand(c,"AUTH %s",m_strPassword.data());
			if (reply->type == REDIS_REPLY_ERROR) {
				//printf("Redis认证失败！\n");
			}
		}

		for(int i=0;i<wstrKeyList.size();i++)
		{
			wstring wstrKey = wstrKeyList[i];
			wstring wstrValue = wstrValueList[i];

			string strKey_utf8, strValue_utf8;
			Project::Tools::WideCharToUtf8(wstrKey, strKey_utf8 );

			Project::Tools::WideCharToUtf8(wstrValue, strValue_utf8);
			reply = (redisReply *)  redisCommand(c,"SET %s %s",strKey_utf8.data(), strValue_utf8.data());
			freeReplyObject(reply);

		}

		bSuccess = true;
	}
	
	redisFree(c);     
	return bSuccess;
}