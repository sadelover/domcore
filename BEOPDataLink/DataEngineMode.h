
#ifndef Beop_DATA_ENGINE_MODE_H__
#define Beop_DATA_ENGINE_MODE_H__

#ifdef WIN32
#pragma once
#endif // WIN32


//���ദ��Ľ�ɫ

class BeopDataEngineMode
{
public:
	enum EngineMode{
		Mode_Slave,	 //�ӻ�,slave mode, read only,may copy database from master.
		Mode_Master,  //����, master mode, 
		Mode_NULL
	};
};


#endif
