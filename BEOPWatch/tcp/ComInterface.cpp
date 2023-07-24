//ComInterface.cpp
#include "stdafx.h"
#include "ComInterface.h"

//////////////////////////////////////////////////////////////////////////
CComBase::CComBase()
{

}

CComBase::~CComBase()
{

}

/**********************************************************************
*@brief: �����豸��Slave ID��ȡ�����յ���Ϣ����
*@param: unsigned int nSlaveId          //�豸��Slave ID ��
, gDeviceRevData &RevData				//��Ϣ����
*@return: bool,true�ɹ���falseʧ��
*@author: ZHW 2011-04-01
***********************************************************************/


bool CComBase::SendReadCmd( unsigned int nSlaveId, WORD wAddrFrom, WORD wAddrTo, WORD wFunc )
{
	return true;
}

bool CComBase::SendWriteCmd( unsigned int nSlaveId, WORD wAddress, WORD wValue, WORD wFunc )
{
	return true;
}
