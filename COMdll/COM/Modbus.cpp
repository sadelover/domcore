#pragma once

#include "StdAfx.h"
#include "Modbus.h"
#include "../Tools/EngineInfoDefine.h"
//#include "../Tools/Maths/MathCalcs.h"

CModbus::CModbus(void)
{
	memset(m_bSendCmd, 0, _CMD_LENTH_);
}

CModbus::~CModbus(void)
{
}

/*********************************************************
@brief: ��ϢУ��λ //���ֽڵ�CRCУ��������
@param: BYTE *puchMsg,��Ϣ WORD wDataLen ���ݳ���
*@return: WORD CRCУ��
*@author: ZHW 2011-04-01
**********************************************************/
WORD CModbus::CRC16___(BYTE *puchMsg, WORD wDataLen)
{
	BYTE uchCRCHi = 0xFF ; /* high byte of CRC initialized */
	BYTE uchCRCLo = 0xFF ; /* low byte of CRC initialized */
	unsigned uIndex=0 ; /* will index into CRC lookup table */

	while (wDataLen--) /* pass through message buffer */
	{
		uIndex = uchCRCHi ^ *puchMsg++ ; /* calculate the CRC */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	return (uchCRCHi << 8 | uchCRCLo);
}


/***************************************************************
*@brief:TCP/IPͨѶ��������[��ָ��]
*@param: unsigned int nSlaveId //�豸��Slave ID ��
, WORD wAddrFrom		//��ȡ���Ե���ʼ��ַ
, WORD wAddrTo			//��ȡ���Ե���ֹ��ַ
, WORD wFunc			//�豸ͨѶ����
, BYTE &bLength);		//ָ���	
*@return: unsigned char* Э������
*@author: ZHW 2011-04-01
****************************************************************/
unsigned char* CModbus::ReadCmd(unsigned int nSlaveId, WORD wAddrFrom
								  , WORD wAddrTo, WORD wFunc, BYTE &bLength)

{
	WORD wCRC = 0;
	BYTE byteToEx[2] = {0x00, 0x00};			//�ڴ�˳��
	ZeroMemory(m_bSendCmd, sizeof(m_bSendCmd));

	m_bSendCmd[0] = (BYTE)nSlaveId;
	m_bSendCmd[1] = (BYTE)wFunc;
	m_bSendCmd[2] = ((wAddrFrom-1)&0xFF00)>>8;
	m_bSendCmd[3] = ((wAddrFrom-1)&0x00FF);
	m_bSendCmd[4] = ((wAddrTo-wAddrFrom+1)&0xFF00)>>8;
	m_bSendCmd[5] = ((wAddrTo-wAddrFrom+1)&0x00FF);

	wCRC = CRC16___(m_bSendCmd, 6);
	memcpy(byteToEx, &wCRC, 2);
	m_bSendCmd[6] = byteToEx[1];
	m_bSendCmd[7] = byteToEx[0];

	bLength = _CMD_LENTH_;
	return m_bSendCmd;
}

/**********************************************************************
*@brief:TCP/IPͨѶ��������[дָ��]
*@param: unsigned int nSlaveId   //�豸��Slave ID ��
, WORD wAddress			//д�����Եĵ�ַ
, WORD wValue			//����ֵ
, WORD wFunc			//�豸ͨѶ����
, BYTE &bLength);		//ָ���	
*@return: unsigned char* Э������
*@author: ZHW 2011-04-01
***********************************************************************/
unsigned char* CModbus::WriteCmd(unsigned int nSlaveId, WORD wAddress
								  , WORD wValue, WORD wFunc, BYTE &bLength)

{
	WORD wCRC = 0;
	BYTE byteToEx[2] = {0x00, 0x00};		//�ڴ�˳��
	ZeroMemory(m_bSendCmd, sizeof(m_bSendCmd));

	m_bSendCmd[0] = (BYTE)nSlaveId;
	m_bSendCmd[1] = (BYTE)wFunc;
	m_bSendCmd[2] = ((wAddress-1)&0xFF00)>>8;
	m_bSendCmd[3] = ((wAddress-1)&0x00FF);
	memcpy(byteToEx, &wValue, 2);
	m_bSendCmd[4] = byteToEx[1];
	m_bSendCmd[5] = byteToEx[0];

	wCRC = CRC16___(m_bSendCmd, 6);
	memcpy(byteToEx, &wCRC, 2);
	m_bSendCmd[6] = byteToEx[1];
	m_bSendCmd[7] = byteToEx[0];

	bLength = _CMD_LENTH_;
	return m_bSendCmd;
}