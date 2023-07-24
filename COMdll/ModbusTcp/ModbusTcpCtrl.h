#pragma once

#include "TcpIpComm.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"

#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

/*
	param6:����
	0��Signed				1		
	1: UnSigned				1
	2: Bite					0-15   ��WORDȡ��Nλ��
	3: Long					2
	4: Long Inverse			2
	5: Float				2
	6: Float Inverse		2
	7: Double				4
	8: Double Inverse		4
	9��String 				N���Ĵ���
	10��String Inverse 		N���Ĵ���(ÿ���Ĵ�������ȡ��)
	11��PowerLink		 	3  (3���Ĵ��� 6���ֽ� ǰ��λ�ɶ�дֵ����1λ������ֻд���ܶ�)
/*
 *
 *����modbus����������
 *
 */

/*
MODBUS�쳣��
����	����	����
01	�Ƿ�����	���ڷ�����(���վ)��˵��ѯ���н��յ��Ĺ������ǲ�������Ĳ�������Ҳ������Ϊ������������������豸���ڱ�ѡ��Ԫ���ǲ���ʵ�ֵġ�ͬʱ����ָ��������(���վ)�ڴ���״̬�д��������������磺��Ϊ����δ���õģ�����Ҫ�󷵻ؼĴ���ֵ��
02	�Ƿ����ݵ�ַ	���ڷ�����(���վ)��˵��ѯ���н��յ������ݵ�ַ�ǲ�������ĵ�ַ���ر��ǣ��ο��źʹ��䳤�ȵ��������Ч�ġ����ڴ���100���Ĵ����Ŀ�������˵������ƫ����96�ͳ���4�������ɹ�������ƫ����96�ͳ���5�����󽫲����쳣��02��
03	�Ƿ�����ֵ	���ڷ�����(���վ)��˵��ѯ���а�����ֵ�ǲ��������ֵ�����ֵָʾ���������ʣ��ṹ�еĹ��ϣ����磺���������ǲ���ȷ�ġ�������ζ�ţ���ΪMODBUSЭ�鲻֪���κ�����Ĵ������κ�����ֵ����Ҫ���壬�Ĵ����б��ύ�洢����������һ��Ӧ�ó�������֮���ֵ��
04	��վ�豸����	��������(���վ)�����跨ִ������Ĳ���ʱ�������������»�õĲ��
05	ȷ��	��������һ��ʹ�á�������(���վ)�Ѿ��������󣬲������ڴ���������󣬵�����Ҫ���ĳ���ʱ�������Щ���������������Ӧ��ֹ�ڿͻ���(����վ)�з�����ʱ���󡣿ͻ���(����վ)���Լ���������ѯ������ɱ�����ȷ���Ƿ���ɴ���
06	�����豸æ	��������һ��ʹ�á�������(���վ)���ڴ�������ʱ��ĳ�������ŷ�����(���վ)����ʱ���û�(����վ)Ӧ���Ժ����´��䱨�ġ�
08	�洢��ż�Բ��	�빦����20��21�Լ��ο�����6һ��ʹ�ã�ָʾ��չ�ļ�������ͨ��һ����У�顣
������(���վ)�跨��ȡ��¼�ļ��������ڴ洢���з���һ����żУ����󡣿ͻ���(������)�������·������󣬵������ڷ�����(���վ)�豸��Ҫ�����
0A	����������·��	������һ��ʹ�ã�ָʾ���ز���Ϊ���������������˿�������˿ڵ��ڲ�ͨ��·����ͨ����ζ�������Ǵ������õĻ���صġ�
0B	����Ŀ���豸��Ӧʧ��	������һ��ʹ�ã�ָʾû�д�Ŀ���豸�л����Ӧ��ͨ����ζ���豸δ�������С�
*/

/*
�����룺
0 �޴���
1 �ڴ淶Χ����
2 �Ƿ������ʻ�У��
3 �Ƿ�������ַ
4 �Ƿ�Modbus����ֵ
5 ���ּĴ�����Modbus���������ص�
6 �յ�У�����
7 �յ�CRC����
8 �Ƿ��������󣯹��ܲ���֧��
9 �����еķǷ��ڴ��ַ
10 ��������δ����
*/
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "Tools/StructDefine.h"

const unsigned char m_auchCRCHi[]=
{
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40 
};

const unsigned char m_auchCRCLo[]=
{
	0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,
	0x07,0xC7,0x05,0xC5,0xC4,0x04,0xCC,0x0C,0x0D,0xCD,
	0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
	0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
	0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
	0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
	0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
	0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
	0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
	0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
	0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
	0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
	0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
	0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
	0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
	0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
	0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
	0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
	0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
	0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
	0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
	0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
	0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
	0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
	0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
	0x43,0x83,0x41,0x81,0x80,0x40 
};

//enum E_MODBUS_VALUE_TYPE
//{
//	E_MODBUS_SIGNED = 0,
//	E_MODBUS_UNSIGNED,
//	E_MODBUS_BITE,
//	E_MODBUS_LONG,
//	E_MODBUS_LONG_INVERSE,
//	E_MODBUS_FLOAT,
//	E_MODBUS_FLOAT_INVERSE,
//	E_MODBUS_DOUBLE,
//	E_MODBUS_DOUBLE_INVERSE,
//	E_MODBUS_STRING,
//	E_MODBUS_STRING_INVERSE,
//	E_MODBUS_POWERLINK,			//11 ����powerLink 3λ�Ĵ�����ǰ��λ�ɶ�����һλֻд
//};
//
//struct _ModbusReadUnit
//{
//	wstring strPointName;
//	unsigned int nSlaveId;
//	DWORD dwAddFrom;
//	DWORD dwAddTo;
//	DWORD dwFuncCode;
//	int nReadSuccessCount;
//	bool	bHasErrPoint;
//	bool	bMultiRead;			//true ��������ȡ  false �ǵ����ȡ   ������ȡ���� �л��������һ�� ���л�����
//};
//
//struct _ModbusWriteCmd
//{
//	WORD dwAddFrom;
//	WORD dwFuncCode;
//	WORD dwValue;
//};

class CModbusTcpCtrl : public CTcpIpComm
{
public:
	CModbusTcpCtrl(void);
	virtual ~CModbusTcpCtrl(void);

protected:
	
	//����ͨѶ�����İ�.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:
	void	SetHost(const string& strhost);
	void	SetPort(u_short portnumer);

	//Initialize Modbus Tcp Info
	bool Init();
	bool	Exit();

	//Send Read Command
	virtual bool SendReadCmd(unsigned int nSlaveId   //�豸��Slave ID ��
							, WORD wAddrFrom		//��ȡ���Ե���ʼ��ַ
							, WORD wAddrTo			//��ȡ���Ե���ֹ��ַ
							, WORD wFunc            //�豸ͨѶ����
							, int &nErrCode);		//���ش�����

	//Send Write Command
	virtual bool SendWriteCmd(unsigned int nSlaveId   //�豸��Slave ID ��
							, WORD wAddress			//д�����Եĵ�ַ
							, WORD wValue			//����ֵ
							, WORD wFunc);			//�豸ͨѶ����
	
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	void	TerminateSendThread();

	
	void    SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount=99,int nIDInterval = 500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false);
	void	SetReadOneByOneMode(bool bReadOneByOneMode);
	void	SetPointList(const vector<DataPointEntry>& pointlist);
	
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );

	DataPointEntry*	FindEntry(DWORD slaveid, DWORD headaddress, DWORD funccode);

	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);

	bool	InitDataEntryValue(const wstring& pointname, double value);

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	EnableLog(BOOL bEnable);
	void    SortPointByDevice();

	//////////////////////////////////////////////////////////////////////////
	void    SortPointByDevice_();
	void	CreateEntryByDataTypeAndRange(DataPointEntry entry,vector<DataPointEntry>& vecModbus);
	void	GetValueByDataTypeAndRange(DataPointEntry entry,vector< pair<wstring, wstring> >& valuelist);		
	void	GetValueByDataTypeAndRange(DataPointEntry entry,E_MODBUS_VALUE_TYPE type,vector<WORD>& vecValue,vector< pair<wstring, wstring> >& valuelist);
	void	GetValueByDataTypeAndRange(DataPointEntry entry,double dValue,vector<WORD>& vecCmd,bool bOrder);		//tureĬ�ϴӸ��ֽ�������
	void	SetReadCommands_();

	////ʹ�ü�������ģʽ//////////////////////////////////////////////////////////////////////
	int	SendReadCommandsByActive();				//�ظ�����ģʽ
	void	SetSendCmdEvent();						//���ü����¼�
	HANDLE	GetSendCmdEvent();						//��ȡ�����¼�
	void	SendOneByOneByActive();
	bool	SendOneByOneByActive(_ModbusReadUnit& unit, bool &bConnectionStop);
	//////////////////////////////////////////////////////////////////////////

	void	SetReadCommandsWithError();			//��ĳһ��modbus�豸�е�ַ���󣬸ò��ֵ�����ȡ ������Ȼ������ȡ
	DataPointEntry*	FindEntry_(DWORD slaveid, DWORD headaddress, DWORD funccode);
	void	SetModbusReadUnitErrFlag(unsigned int nSlaveId,DWORD dwFuncCode,DWORD dwAdd,bool bHasErrPoint=true);
	//ת������
	int	GetBitFromWord(WORD nData, int nIndex);
	WORD	SetBitToWord(WORD nData, int nIndex,int nValue);
	DWORD MergeTwoWORD(WORD wGao,WORD wDi,bool bOrder = true);		//
	double MergeFourDouble(WORD wGaoGao,WORD wGao,WORD wDi,WORD wDiDi,bool bOrder = true);		//
	double			strtodbl(const wchar_t * str);
	bool	SendWriteCmd(DataPointEntry entry,double fValue);			//�豸ͨѶ����
	bool	SendWriteMutilCmd(DataPointEntry entry,double fValue);			//�豸ͨѶ����
	bool	SendWriteBYTECmd(DataPointEntry entry,double fValue);			//�豸ͨѶ����
	bool	SendWriteWORDCmd(DataPointEntry entry,double fValue);			//�豸ͨѶ����

	bool	SendTestWORDCmd();			//����
	bool	SendTestCloseWORDCmd();			//�ػ�
	bool	SendTestHornResetWORDCmd();			//Horn Reset
	bool	SendTestFaultResetWORDCmd();			//Fault Reset
	bool	SendTestOPenCloseWORDCmd();
	bool	SendTestCloseOPenWORDCmd();

	void	SendOneByOne_();
	bool	SendOneByOne_(_ModbusReadUnit& unit);
	bool	SendReadMutil_(_ModbusReadUnit& unit);
	bool	MutilReadHasErr();
	unsigned short CalcCrcFast(unsigned char*puchMsg,unsigned short usDataLen);
	//////////////////////////////////////////////////////////////////////////
	bool	CombineReadUnit(const _ModbusReadUnit &  mrUnit,bool bModbusEnd=true);

	bool	ReConnect();

	void    SetNetworkError(int nErrCountAdd = 1);
	void    ClearNetworkError();

	void	SendReadCommandOneByOne();			//һ��������
	void	SendOneByOne();
	void    SortBackupPointByDevice();
	void	SetBackupReadCommands();

	void	SetIndex(int nIndex);
	bool	SetDebug(int nDebug);
	bool	OutPutModbusReceive(const unsigned char* pRevData, DWORD length);
	string* byteToHexStr(unsigned char* byte_arr, int arr_len);  
	string  charToHexStr(unsigned char* byte_arr, int arr_len); 

	int		GetMultiReadCount(DWORD slaveid, DWORD headaddress, DWORD funccode);
	void	SumReadAndResponse();	
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	string	m_host;
	u_short	m_port;
	HANDLE	m_hsendthread;
	bool m_bExitThread;
	vector<DataPointEntry> m_pointlist;	//���.
	vector<DataPointEntry> m_backuppointlist;	//���.
	vector<DataPointEntry>	m_vecPointList;
	vector<_ModbusReadUnit> m_ReadUnitList; //��ȡ���ݰ���Ԫ����ϲ���Ӷ����ٶ�ȡ
	map<string,WORD>		m_ModbusValue;		//string �豸ID_���ַ_������
	map<wstring,DataPointEntry*>	m_mapErrPoint;		//��ȡ�����
	Project::Tools::Mutex	m_lock;
	BOOL m_bLog;
	Beopdatalink::CLogDBAccess* m_logsession;
	int m_nSendReadCmdTimeInterval;				//����������
	int							m_nRecevieTimeOut;						//���ճ�ʱ
	int	m_nPollSendInterval;				//��ѯ���
	int m_nRecvedSlave;

	bool	m_bSortPoint;		//���ŵ����
	int		m_nSendCmdCount;	//����100���ж��µ���Ƿ�������������һ��
	int		m_nMutilReadCount;		//������������ Ĭ��99

	bool	m_bReadOneByOneMode;
	bool	m_bDisSingleRead;
	//�Զ���������
	Project::Tools::Mutex	m_lock_connection;
	HANDLE m_hDataCheckRetryConnectionThread;
	int m_nModbusNetworkErrorCount;

	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
	int				m_nEngineIndex;
	int				m_nIDInterval;			//��ͬID֮��ļ�� Ĭ��500ms
	int				m_nOutPut;				//������յ���cmd
	int				m_nPrecision;				//���ݾ��� Ĭ��6

	int									m_nCmdCount;						//�������
	int									m_nResponseCount;					//����ظ�����
	int									m_nUpdatePointCount;				//���µ�
	string								m_strUpdateLog;						//����log

	_ModbusReadUnit				m_lastReadUnit;						//��һ�ζ�ȡ�ṹ
	DataPointEntry				m_lastReadEntry;
	HANDLE						m_sendCmdEvent;
};

