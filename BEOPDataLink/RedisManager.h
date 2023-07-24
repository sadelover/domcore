#pragma once

#include "xstring"
#include "VECTOR"
using namespace std;

class CRedisManager
{
public:
	CRedisManager(void);
	~CRedisManager(void);

	bool init(wstring wstrRedisIP, int nRedisPort);
	bool is_valid();
	bool set_value(wstring wstrKey, wstring wstrValue);
	bool set_value_mul(vector<wstring> wstrKeyList, vector<wstring> wstrValueList);
	bool set_value_mul_utf8(vector<string> wstrKeyList, vector<string> wstrValueList);
	bool get_value(wstring wstrKey, wstring &wstrValue);
	bool get_value_string(wstring wstrKey, string &wstrValue);

	std::string m_strRedisIP;
	int m_nRedisPort;
	bool m_bRedisValid;
	string m_strPassword;
};

