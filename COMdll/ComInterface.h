//ComInterface.h
#pragma once
#pragma warning(disable:4251)

//////////////////////////////////////////////////////////////////////////
class CComBase
{
public:
    CComBase();
    virtual ~CComBase();

public:
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

    //Get Revdatas with nSlaveID

protected:
    virtual bool OnCommunication(const unsigned char* pRevData, DWORD length) = 0;
};
