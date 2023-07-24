#pragma once
#include "BEOPDataLinkManager.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "BEOPDataPoint/DataPointEntry.h"

class COPCEngine;
class CModbusEngine;
class CMemoryLink;
class CBacnetEngine;
class CDataPointManager;


#include <map>
using std::map;

#include "DataEngineMode.h"
/*
*�����������engine�������
*
 */
class CBEOPDataLinkManager_Redundacy: public CBEOPDataLinkManager
{
public:
	CBEOPDataLinkManager_Redundacy(CDataPointManager* pointmanager,
							 Beopdatalink::CRealTimeDataAccess* pdbcon,
							 bool bRealSite);

	virtual ~CBEOPDataLinkManager_Redundacy();

	virtual bool	Init();
	bool	InitPointManagerRedundancy();

	virtual void	GetOPCValueSets(vector<pair<wstring, wstring> >& opcvaluesets);
	virtual	bool	WriteOPCValue(const Beopdatalink::CRealTimeDataEntry& entry);
	
	COPCEngine*		GetMainOPCSession();		//��Ϊ��ʱ,����PLC��������.


	/*
	 *�ӷ�������һֱping��������,����ղ�����Ӧ,����Զ���Ϊ��������
	 */
	virtual void			UpdateMainServerMode();		//��������.

	void	InitMasterSlave();
private:
	CDataPointManager*	m_pointmanager_redundancy;
	COPCEngine*			m_opcengine_redundancy;
};