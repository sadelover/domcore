#pragma once

#include "wtypes.h"
#include "XSTRING"

using namespace std;

struct  WarningOperation
{
	wstring			strTime;
	wstring			strWarningInfo;	
	wstring			strBindPointName;
	wstring			strOpeartion;	
	wstring			strUser;
};

struct  UserOperation
{
	wstring			strTime;
	wstring			strOpeartion;	
	wstring			strUser;
};

class  CWarningEntry
{
public:
	~CWarningEntry(void);

public:
	CWarningEntry();
	SYSTEMTIME GetTimestamp() const;
	void	SetTimestamp(const SYSTEMTIME& stime);

	wstring	GetWarningInfo() const;
	void	SetWarningInfo(const wstring& warninginfo);


	wstring	GetWarningInfoDetail() const;
	void	SetWarningInfoDetail(const wstring& warninginfo);

	wstring	GetWarningPointName() const;
	void	SetWarningPointName(const wstring& warninginfo);

	int	GetWarningCode() const;
	void	SetWarningCode(int warningcode);

	int	GetWarningLevel() const;
	void	SetWarningLevel(int warninglevel);

	SYSTEMTIME	GetEndTimestamp() const;
	void	SetEndTimestamp(const SYSTEMTIME& stime);

	int		GetWarningConfirmedType() const;
	void	SetWarningConfirmedType(int nConfirmedType);

	int   GetID() const;
	int   GetRuleID() const;
	void   SetID(int nID);
	void  SetRuleID(int nID);
public:
	SYSTEMTIME m_time;
	int	m_warningcode;
	wstring m_warninginfo;
	wstring m_warninginfo_detail;
	int m_warninglevel;
	int	m_nConfirmedType;

	SYSTEMTIME m_timeEnd;

	wstring m_strBindPointName;

	int m_nID;
	int m_nRuleID;

	string m_strOfPosition;
	string m_strOfDepartment;
	string m_strOfSystem;
	string m_strOfGroup;
	string m_strTag;
	string m_strGoodRange;//��ֵ
	string m_strBindPointValue; //��ص�ֵ

	//string m_strUnitProperty01;//�Ѿ�����
	//string m_strUnitProperty02;//�Ѿ�����
	string m_strUnitProperty03;
	string m_strUnitProperty04;
	string m_strUnitProperty05;
};

