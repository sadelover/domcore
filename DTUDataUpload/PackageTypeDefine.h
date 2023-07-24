

/*
 *
 *
 * this class will defines package type of the data transferred between 
 * dtu client and dtu server.
 *
 *
 */


#pragma once

namespace rnbtech
{
	class  CPackageType
	{
	public:
		enum DTU_DATATYPE
		{
			Type_InValid,
			Type_PointData,
			Type_OperationRecord,
			Type_OptimizeResult,
			Type_PointData_WithTime,		//��ʱ���������
			Type_DTUServerCmd,				//DTUServerCmd
			Type_DTUServerFrl,				//DTUServerCmd�ļ�����Log
			Type_DTUServerFup,				//DTUServerCmd�ļ�����
			Type_DTUWriteTime,				//DTU����ʱ��
		};

		static DTU_DATATYPE  GetPackageType(const char* buffer);
		static int	RemovePrefix(char* buffer,int length=-1);
	};

}
