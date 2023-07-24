
/*
 *���������֧��upppc data engine����㹦��
 *��㲻�������ط���ȡ,ֻά�����ڴ���.
 *
 *
 */


#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include <vector>
#include <utility>
#include <map>
#include "Tools/CustomTools/CustomTools.h"

class CBEOPDataLinkManager;
class CMemoryLink
{
public:
	CMemoryLink(CBEOPDataLinkManager* datalinker);
	~CMemoryLink(void);

	//set pointlist;
	void	SetPointList(const std::vector<DataPointEntry>& pointlist);
	int	GetPointType(const wstring& pointname);

	//init
	// set point must be called before init.
	bool	Init();
	bool    Exit();

	//return the value set.
	void	GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist );
	
	void	GetValue(const wstring& pointname, int& ival);
	void	GetValue(const wstring& pointname, double& dval);
	void	GetValue(const wstring& pointname, bool& bval);
	void	GetValue(const wstring& pointname, wstring& strvalue);
	

	bool	SetValue(const wstring& pointname, double dval);
	bool	SetValue(const wstring& pointname, int ival);
	bool	SetValue(const wstring& pointname, bool bval);

	bool	SetValue(const wstring& pointname, const wstring& value);


	void    AddVPoint(wstring wstrPointName);
	void    RemoveVPoint(wstring wstrPointName);


private:

	class MemoryPointEntry
	{
	public:
		MemoryPointEntry(const DataPointEntry& pointentry, CBEOPDataLinkManager* datalinker);
		virtual ~MemoryPointEntry();

		void SetPointname(const wstring& pointname);
		wstring GetPointname() const;

		virtual void SetDouble(double dval);
		virtual void SetInt(int intval);
		virtual void SetBool(bool bval);

		virtual void SetString(const wstring strunicode);

		virtual bool  GetDouble(double &dVal);
		virtual bool	GetInt(int &iVal);
		virtual bool	GetBool(bool &bVal);
		virtual wstring GetUnicodeString();

		const DataPointEntry& GetRefEntry();


		void	InitValue();
	private:
		wstring m_pointname;
		Project::Tools::Mutex m_lock;
		wstring       m_strValue;
		DataPointEntry	m_pointentry;
	protected:
		CBEOPDataLinkManager*	m_datalinker;
	};


private:
	std::vector<DataPointEntry>	m_pointlist;
	
	std::map<wstring, MemoryPointEntry*> m_valuemap;
	std::map<wstring, int>	m_typemap;

	CBEOPDataLinkManager*	m_datalinker;
};

