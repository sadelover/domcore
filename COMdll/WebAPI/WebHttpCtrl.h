#include <string>
#include <afxinet.h>
#include <hash_map>
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
using namespace std;

/*
���
    ���ַ�����ĺţ�IP/�˿ڣ����ʣ��û���������

���Ľṹ��
    ���ַ�����ĺţ�IP/�˿ڣ��û��������룬ʵ�ʶ��ĺţ�ʵ����Ϣ�ţ�ʧЧʱ��

*/

/*		���޸�
	2���仯�����ģʽ ���ȡһ��ȫ������
	5�����ز������Ϳ�ѡ�𣿣�̫�ࣩ
*/

struct SubscriptionInfo
{
	SubscriptionInfo()
	{
		strSubscriptionID = "";
		strSubscriptionID_Active = "";
		strNotificationID_Active = "";
		strNotificationIDAll_Active = "";
		strExpireTime = "";
		bActive = false;
		bUpdateAll = false;
		mapSubPoint.clear();
		mapValue.clear();
	}

	string strSubscriptionID;				//���ⶩ��ID
	string strSubscriptionID_Active;		//ʵ�ʶ���ID
	string strNotificationID_Active;		//ʵ����ϢID
	string strNotificationIDAll_Active;		//ʵ����Ϣȫ��ID
	string strExpireTime;					//ʧЧʱ��
	bool   bActive;							//�״̬
	bool   bUpdateAll;						//���ٻ�ȡһ��ȫ������
	hash_map<string,int>	mapSubPointAddress;	//���ĵ�(���ַ����ֵ)
	hash_map<string,string>	mapValue;		//���ĵ�(���ַ����ֵ)
	hash_map<wstring,DataPointEntry*> mapSubPoint;	//���ĵ�(����������)
};

class __declspec(dllexport) CWebHttpCtrl
{
public:
	CWebHttpCtrl(Beopdatalink::CCommonDBAccess*	dbsession_history,int nDurationInMinutes=30,int nUpdateInterval = 60,int nActiveMinutes=30,string strSubscriptionType="ValueItemChanged",bool bChangesOnly = false,bool bDeleteSubcription=false,wstring strSubcriptions= L"");
	virtual ~CWebHttpCtrl(void);

	bool	CloseConnection();												//�ر�����
	bool	CreateDefaultConnection();										//����Http Post����
	bool	CreateConnection(CString strHost, INTERNET_PORT nPort);			//����Http Post����

	bool	Init();															//��ʼ��
	bool	Exit();															//�˳�
	bool	SetConnection(CString strHost, INTERNET_PORT nPort);			//����Http Post����
	void	SetPointList(const vector<DataPointEntry>& pointlist);			//���õ��
	void	SetIndex(int nIndex);											//���������
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );		//��ȡ��ֵ
	
	void	SumReadAndResponse();											//ÿ�η���ǰͳ����һ�ε��շ����
	//////////////////////////////////////////////////////////////////////////
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);				//��ѯ
	bool	GenerateSubcription();											//���ɶ��Ľṹ��
	bool	SendSuccriptions();												//��ѯ���ж���
	bool	SaveSuccriptions();												//�洢���ж���
	bool	SendOneSuccription(SubscriptionInfo& subcription);				//��ѯ��������
	bool	CreateOneSuccription(SubscriptionInfo& subcription);			//����һ������
	bool	DeleteOneSuccription(SubscriptionInfo& subcription);			//ɾ��һ������
	bool	DeleteOneSuccription(string strSubcription);					//ɾ��һ������
	bool	GetAllSuccription(vector<string>& vecSuccription);				//��ȡ���ж���
	bool	DeletAllSuccription();											//ɾ�����ж���
	bool	ActiveOneSuccription(SubscriptionInfo& subcription);			//����һ������
	bool	CreateOneNotification(SubscriptionInfo& subcription);			//����һ����Ϣ
	bool	CreateOneNotificationAll(SubscriptionInfo& subcription);		//����һ����Ϣ(ȫ������)
	bool	DeleteOneNotificationAll(SubscriptionInfo& subcription);		//ɾ��һ����Ϣ(ȫ������)
	bool	DeleteOneNotification(SubscriptionInfo& subcription);			//ɾ��һ����Ϣ
	bool	PollOneSuccription(SubscriptionInfo& subcription);				//��ѯһ������
	bool	GetToken();														//��ȡ��Ȩ
	//////////////////////////////////////////////////////////////////////////
	bool	JsonParseToken(string strJson);									//����Token
	bool	JsonParseCreateSuccription(string strJson,SubscriptionInfo& subcription);		//������������
	bool	JsonParseActiveSuccription(string strJson,SubscriptionInfo& subcription);		//���������
	bool	JsonParseCreateNotification(string strJson,SubscriptionInfo& subcription);		//����������Ϣ
	bool	JsonParseCreateNotificationAll(string strJson,SubscriptionInfo& subcription);	//����������Ϣ
	bool	JsonParsePollSuccription(string strJson,SubscriptionInfo& subcription,bool bNotificationAll =false);			//������ѯ��Ϣ
	bool	JsonParseAllSuccription(string strJson,vector<string>& vecSuccription);			//������ȡȫ������
	//////////////////////////////////////////////////////////////////////////

	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
	bool	OutPutWebServerInfo(SubscriptionInfo& subcription);				//���������Ϣ
	bool	OutPutLogString(wstring strOut);
public:
	CInternetSession					m_Isession;
	CHttpConnection*					m_pConnection;
	CString								m_strHost;							//Http IP
	INTERNET_PORT						m_nPort;							//Http Port
	CString								m_strUser;							//Http User
	CString								m_strPsw;							//Http Psw
	vector<DataPointEntry>				m_pointlist;						//���.
	hash_map<string,SubscriptionInfo>	m_mapSubcription;					//���Ļ���  ���ĺ�+����
	bool								m_bExitThread;						//�˳��߳�
	int									m_nUpdateInterval;					//��ѯ���
	Project::Tools::Mutex				m_mtuex_http;						//HTTP�ӿڻ�����
	int									m_nTimeOutCount;					//������ʱ����
	int									m_nInvalidResponseCount;			//������Ч��Ӧ����
	string								m_strToken;							//��Ȩ
	int									m_nDurationInMinutes;				//���Ĳ���ʱ��
	string								m_strSubscriptionType;				//���Ĳ�������
	int									m_nActiveMinutes;					//�����ʱ��
	bool								m_bChangesOnly;						//���ر仯ֵ  If true, all items will be returned. If false, only changes 
	COleDateTime						m_oleUpdateTime;					//����ʱ��
	int									m_nEngineIndex;						//������
	HANDLE								m_hSendCmd;							//��ѯ����
	bool								m_bDeleteSubcription;				//ɾ��ָ�����ĺ�
	wstring								m_strSubcriptions;					//���ĺ�
	bool								m_bCreateSubcription;				//�������ĳɹ��� �����ĺŴ浽���ݿ���
	Beopdatalink::CCommonDBAccess*		m_dbsession_history;				//���ݿ�����
	int									m_nCmdCount;						//�������
	int									m_nResponseCount;					//����ظ�����
	int									m_nUpdatePointCount;				//���µ�
	string								m_strUpdateLog;						//����log
};