#pragma once

#include "Modbus.h"
#include "SerialPort.h"

//hash_map 4251
#pragma warning(disable:4251)

#define  _CMD_REV_LENTH_  0x100   //�������ݵĳ���

class CCOMCtrl : public CSerialPort
{
public:
	CCOMCtrl(void);
	virtual ~CCOMCtrl(void);

protected:
    virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

	//Initialize hash_map Info
	bool Init_HashMap(void);

	void SendCmd(char *pData, BYTE bLength);
	BOOL Is_DataValid(BYTE *pData, WORD dwLength);

public:
	//Initialize COMPort Info
	bool InitCOMPort(UINT nPortnr, UINT bBaud, char parity = 'N');

	//Send Read Command
	virtual bool SendReadCmd(unsigned int nSlaveId   //�豸��Slave ID ��
		, WORD wAddrFrom		//��ȡ���Ե���ʼ��ַ
		, WORD wAddrTo			//��ȡ���Ե���ֹ��ַ
		, WORD wFunc);			//�豸ͨѶ����

	//Send Write Command
	virtual bool SendWriteCmd(unsigned int nSlaveId   //�豸��Slave ID ��
		, WORD wAddress			//д�����Եĵ�ַ
		, WORD wValue			//����ֵ
		, WORD wFunc);			//�豸ͨѶ����

private:
	char *m_pSendData;					//the send package buffer
	CModbus m_Modbus;
	CRITICAL_SECTION m_CriticalSection; //�ٽ�������
	
	WORD	m_StartAddress;		//��ʼ��ַ��
};