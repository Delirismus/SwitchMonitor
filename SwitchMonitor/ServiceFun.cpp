
#include "stdafx.h"
#include <winsvc.h>
#include "ServiceFun.h"
//--------------------------------------------------------------------------- 
//�ж�ϵͳ���Ƿ����ĳ������
//---------------------------------------------------------------------------
BOOL ChkHasServc(WCHAR* strServiceName)
{
	//�򿪷�����ƹ�����
	SC_HANDLE hSC = ::OpenSCManager(NULL,NULL,GENERIC_EXECUTE);
	if(hSC == NULL)
	{
		OutputDebugString(TEXT("open SCManager error\r\n"));
		return FALSE;
	}
	// ��DC_ClientService����
	SC_HANDLE hSvc = ::OpenService(hSC,strServiceName,SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_STOP);
	if(hSvc == NULL)
	{
		OutputDebugString(TEXT("Open eventlog erron.\r\n"));
		::CloseServiceHandle(hSC);
		return FALSE;
	}
	::CloseServiceHandle(hSvc);
	::CloseServiceHandle(hSC);
	return TRUE;
}
//--------------------------------------------------------------------------- 
//�ж�ĳ�����Ƿ�������״̬
//--------------------------------------------------------------------------- 
BOOL ChkServcRun(WCHAR* strServiceName) 
{ 
	BOOL bRet = FALSE;
	SC_HANDLE scm,svc; 
	SERVICE_STATUS ServiceStatus; 
	scm = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS); 
	if(scm!=NULL) 
	{ 
		svc = OpenService(scm, strServiceName,SERVICE_QUERY_STATUS); 
		if(svc!=NULL) 
		{ 
			QueryServiceStatus(svc,&ServiceStatus); 
			if(ServiceStatus.dwCurrentState == SERVICE_RUNNING) 
			{
				bRet = TRUE;
			}
			CloseServiceHandle(svc); 
		} 
		CloseServiceHandle(scm); 
	} 	
	return bRet;
} 
