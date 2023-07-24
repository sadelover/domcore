#pragma once


#include <vector>
using std::vector;

#include "DTUServerHandler.h"
#include "DTUAddressMap.h"
/*
 *
 *
 *���洦��ء�
 *Ĭ��10���̺߳�10����������������
 *
 *
 *
 */

const int default_handler_num = 10;

class DTUServerHandlerPool
{
public:
	DTUServerHandlerPool(void);
	~DTUServerHandlerPool(void);

	void	CreateDTUServerHandler(mapentry entry);

	CDTUServerHandler* GetHandlerByDTUName(string strName) const;

public:
	vector<CDTUServerHandler*>	m_handlerlist;
};

