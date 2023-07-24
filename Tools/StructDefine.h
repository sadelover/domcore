#pragma once

enum E_MODBUS_VALUE_TYPE
{
	E_MODBUS_SIGNED = 0,
	E_MODBUS_UNSIGNED,
	E_MODBUS_BITE,
	E_MODBUS_LONG,
	E_MODBUS_LONG_INVERSE,
	E_MODBUS_FLOAT,
	E_MODBUS_FLOAT_INVERSE,
	E_MODBUS_DOUBLE,
	E_MODBUS_DOUBLE_INVERSE,
	E_MODBUS_STRING,
	E_MODBUS_STRING_INVERSE,
	E_MODBUS_POWERLINK,			//11 ����powerLink 3λ�Ĵ�����ǰ��λ�ɶ�����һλֻд
	E_MODBUS_UNSIGNED_LONG,
	E_MODBUS_UNSIGNED_LONG_INVERSE, 
};

struct _ModbusReadUnit
{
	wstring strPointName;
	unsigned int nSlaveId;
	DWORD dwAddFrom;
	DWORD dwAddTo;
	DWORD dwFuncCode;
	int nReadSuccessCount;
	bool	bHasErrPoint;
	bool	bMultiRead;			//true ��������ȡ  false �ǵ����ȡ   ������ȡ���� �л��������һ�� ���л�����
	bool	bModbusEnd;
};

struct _ModbusWriteCmd
{
	WORD dwAddFrom;
	WORD dwFuncCode;
	WORD dwValue;
};

enum E_CO3P_PARAMS_TYPE
{
	E_CO3P_IP = 1,
	E_CO3P_ADDR1,
	E_CO3P_ADDR2,
	E_CO3P_TYPE,
	E_CO3P_CMD,
	E_CO3P_INTEGER,
	E_CO3P_DECIMAL,
};

struct _CKECO3PReadUnit
{
	_CKECO3PReadUnit()
	{
		memset(cRecBuffer,0,1024);
		nLength = 0;
		bResponseSuccess = false;
	}
	wstring strPointName;
	WORD	wAddr1;				//��������ַ
	WORD	wAddr2;				//�Ǳ��ַ
	WORD	wType;				//�Ǳ�����
	WORD	wCmd;				//�Ǳ��������
	char	cRecBuffer[1024];	//�յ������ݰ�����
	int		nLength;			//�յ������ݰ�����
	int		nReadSuccessCount;
	bool	bResponseSuccess;	//�Ƿ�ɹ�����
};

struct _MysqlCrossTableUnit
{
	_MysqlCrossTableUnit()
	{
		nColumnCount = 0;
		nFilterColumnIndex = 0;
		vecFilter.clear();
	}
	wstring strTableName;
	wstring strOrderColumn;
	wstring strFilterColumn;
	int		nColumnCount;
	int		nFilterColumnIndex;
	vector<wstring> vecFilter;
};

enum DTU_CMD_TYPE								//DTU��������
{
	DTU_CMD_DEFINE = 0,							//Ĭ��
	DTU_CMD_REAL_DATA_SEND,						//ʵʱ���ݣ����ͣ�
	DTU_CMD_REAL_DATA_SYN,						//ʵʱ���ݣ�ͬ����
	DTU_CMD_REAL_DATA_EDIT,						//ʵʱ���ݣ��޸ģ�
	DTU_CMD_REAL_DATA_EDIT_MUL,					//ʵʱ���ݣ������޸ģ�
	DTU_CMD_WARNING_DATA_SEND,					//��������(����)
	DTU_CMD_WARNING_DATA_SYN,					//��������(ͬ��)
	DTU_CMD_WARNING_OPERATION_DATA_SEND,		//��������(����)
	DTU_CMD_WARNING_OPERATION_DATA_SYN,			//��������(����)
	DTU_CMD_OPERATION_SEND,						//������¼�����ͣ�
	DTU_CMD_OPERATION_SYN,						//������¼��ͬ����
	DTU_CMD_UNIT_SEND,							//unit01(����)
	DTU_CMD_UNIT_SYN,							//unit01(ͬ��)
	DTU_CMD_UNIT_EDIT,							//unit01(�޸�)
	DTU_CMD_UNIT_EDIT_MUL,						//unit01(�����޸�)
	DTU_CMD_LOG_SEND,							//Log�����ͣ�
	DTU_CMD_LOG_SYN,							//Log��ͬ����
	DTU_CMD_HEART_SEND,							//������(����)
	DTU_CMD_HEART_SYN,							//������(ͬ��)
	DTU_CMD_ERR_LIST,							//ERROR(�����б�)
	DTU_CMD_ERR_CODE,							//ERROR(�������)
	DTU_CMD_ERROR_SEND,							//ERROR�����ͣ�
	DTU_CMD_ERROR_SYN,							//ERROR��ͬ����
	DTU_CMD_REAL_FILE_SEND,						//ʵʱ�ļ������ͣ�
	DTU_CMD_REAL_FILE_SYN,						//ʵʱ�ļ���ͬ����
	DTU_CMD_HISTORY_FILE_SEND,					//��ʷ�ļ������ͣ�
	DTU_CMD_HISTORY_FILE_SYN,					//��ʷ�ļ���ͬ����
	DTU_CMD_UPDATE_EXE,							//���³���
	DTU_CMD_UPDATE_POINT_CSV,					//����CSV���
	DTU_CMD_UPDATE_POINT_EXCEL,					//����EXCEL���
	DTU_CMD_UPDATE_DLL,							//����DLL
	DTU_CMD_UPDATE_S3DB,						//����s3db
	DTU_CMD_UPLOAD_POINT_CSV,					//�ϴ�CSV���
	DTU_CMD_DELETE_POINT_MUL,					//����ɾ����
	DTU_CMD_TIME_SYN,							//ͬ��ʱ��
	DTU_CMD_RESTART_CORE,						//����Core
	DTU_CMD_RESTART_DOG,						//����Dog
	DTU_CMD_RESTART_LOGIC,						//����Logic
	DTU_CMD_RESTART_DTU,						//����DTU
	DTU_CMD_RESTART_DTU_ENGINE,					//����DTUEngine
	DTU_CMD_CORE_STATUS,						//��ȡ������״̬
	DTU_CMD_VERSION_CORE,						//Core�汾��
	DTU_CMD_VERSION_DOG,						//Dog�汾��
	DTU_CMD_VERSION_DTU							//DTU�汾��
};

//DTU���ͽṹ
typedef struct DTU_DATA_INFO_TAG
{
	DTU_DATA_INFO_TAG()
	{
		strID = "0";
		nSubType = 0;
		nTryCount = 0;
		strTime = "";
		strFileName = "";
		strContent = "";
		nCmdType = DTU_CMD_DEFINE;
	}
	string		strID;							//���
	string		strTime;						//ʱ��
	string		strFileName;					//�ļ���
	int			nSubType;						//������   0��ʾ�����ļ�  1��ʾ�����ļ�
	int			nTryCount;						//���Դ���
	string		strContent;						//�ı�����  Ansi
	DTU_CMD_TYPE nCmdType;						//��������
} DTU_DATA_INFO;