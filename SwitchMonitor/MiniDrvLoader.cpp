
#include "stdafx.h"
#include "MiniDrvLoader.h"
//��װMiniFilter����
BOOL InstallMiniDriver(const WCHAR* lpszDriverName,const WCHAR* lpszDriverPath,const WCHAR* lpszAltitude)
{
	WCHAR    szTempStr[MAX_PATH * 2];
	HKEY    hKey;
	DWORD    dwData;
	WCHAR    szDriverImagePath[MAX_PATH];    

	if( NULL == lpszDriverName || NULL == lpszDriverPath )
	{
		return FALSE;
	}
	//�õ�����������·��
	GetFullPathName(lpszDriverPath,MAX_PATH,szDriverImagePath,NULL);

	SC_HANDLE hServiceMgr = NULL;// SCM�������ľ��
	SC_HANDLE hService = NULL;// NT��������ķ�����

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hServiceMgr == NULL ) 
	{
		// OpenSCManagerʧ��
		CloseServiceHandle(hServiceMgr);
		return FALSE;        
	}

	// OpenSCManager�ɹ�  

	//������������Ӧ�ķ���
	hService = CreateService( hServiceMgr,
		lpszDriverName,             // �����������ע����е�����
		lpszDriverName,             // ע������������DisplayName ֵ
		SERVICE_ALL_ACCESS,         // ������������ķ���Ȩ��
		SERVICE_FILE_SYSTEM_DRIVER, // ��ʾ���صķ������ļ�ϵͳ��������
		SERVICE_DEMAND_START,       // ע������������Start ֵ
		SERVICE_ERROR_IGNORE,       // ע������������ErrorControl ֵ
		szDriverImagePath,          // ע������������ImagePath ֵ
		TEXT("FSFilter Activity Monitor"),// ע������������Group ֵ
		NULL, 
		TEXT("FltMgr"),                   // ע������������DependOnService ֵ
		NULL, 
		NULL);

	if( hService == NULL ) 
	{        
		if( GetLastError() == ERROR_SERVICE_EXISTS ) 
		{
			//���񴴽�ʧ�ܣ������ڷ����Ѿ�������
			CloseServiceHandle(hService);       // ������
			CloseServiceHandle(hServiceMgr);    // SCM���
			return TRUE; 
		}
		else 
		{
			CloseServiceHandle(hService);       // ������
			CloseServiceHandle(hServiceMgr);    // SCM���
			return FALSE;
		}
	}
	CloseServiceHandle(hService);       // ������
	CloseServiceHandle(hServiceMgr);    // SCM���

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances�ӽ��µļ�ֵ�� 
	//-------------------------------------------------------------------------------------------------------
	StringCchCopy(szTempStr,MAX_PATH * 2,TEXT("SYSTEM\\CurrentControlSet\\Services\\"));
	StringCchCat(szTempStr,MAX_PATH * 2,lpszDriverName);
	StringCchCat(szTempStr,MAX_PATH * 2,TEXT("\\Instances"));
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,szTempStr,0,TEXT(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,(LPDWORD)&dwData)!=ERROR_SUCCESS)
	{
		return FALSE;
	}
	// ע������������DefaultInstance ֵ 

	StringCchCopy(szTempStr,MAX_PATH * 2,lpszDriverName);
	StringCchCat(szTempStr,MAX_PATH * 2,TEXT(" Instance"));
	if(RegSetValueEx(hKey,TEXT("DefaultInstance"),0,REG_SZ,(CONST BYTE*)szTempStr,(DWORD)wcslen(szTempStr) * sizeof(WCHAR))!=ERROR_SUCCESS)
	{
		return FALSE;
	}
	RegFlushKey(hKey);//ˢ��ע���
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances\\DriverName Instance�ӽ��µļ�ֵ�� 
	//-------------------------------------------------------------------------------------------------------
	StringCchCopy(szTempStr,MAX_PATH * 2,TEXT("SYSTEM\\CurrentControlSet\\Services\\"));
	StringCchCat(szTempStr,MAX_PATH * 2,lpszDriverName);
	StringCchCat(szTempStr,MAX_PATH * 2,TEXT("\\Instances\\"));
	StringCchCat(szTempStr,MAX_PATH * 2,lpszDriverName);
	StringCchCat(szTempStr,MAX_PATH * 2,TEXT(" Instance"));

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,szTempStr,0,TEXT(""),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,(LPDWORD)&dwData)!=ERROR_SUCCESS)
	{
		return FALSE;
	}
	// ע������������Altitude ֵ
	StringCchCopy(szTempStr,MAX_PATH * 2,lpszAltitude);
	
	if(RegSetValueEx(hKey,TEXT("Altitude"),0,REG_SZ,(CONST BYTE*)szTempStr,(DWORD)wcslen(szTempStr) * sizeof(WCHAR))!=ERROR_SUCCESS)
	{
		return FALSE;
	}
	// ע������������Flags ֵ
	dwData=0x0;
	if(RegSetValueEx(hKey,TEXT("Flags"),0,REG_DWORD,(CONST BYTE*)&dwData,sizeof(DWORD))!=ERROR_SUCCESS)
	{
		return FALSE;
	}
	RegFlushKey(hKey);//ˢ��ע���
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	return TRUE;
}

//����Mini����
BOOL StartMiniDriver(const WCHAR* lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;

	if(NULL==lpszDriverName)
	{
		return FALSE;
	}

	schManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(NULL==schManager)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	schService=OpenService(schManager,lpszDriverName,SERVICE_ALL_ACCESS);
	if(NULL==schService)
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if(!StartService(schService,0,NULL))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		if( GetLastError() == ERROR_SERVICE_ALREADY_RUNNING ) 
		{             
			// �����Ѿ�����
			return TRUE;
		} 
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

//ֹͣMini����
BOOL StopMiniDriver(const WCHAR* lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(NULL == schManager)
	{
		return FALSE;
	}
	schService = OpenService(schManager,lpszDriverName,SERVICE_ALL_ACCESS);
	if(NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}    
	if(!ControlService(schService,SERVICE_CONTROL_STOP,&svcStatus) && (svcStatus.dwCurrentState!=SERVICE_STOPPED))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

//ж��Mini����
BOOL DeleteMiniDriver(const WCHAR* lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(NULL==schManager)
	{
		return FALSE;
	}
	schService=OpenService(schManager,lpszDriverName,SERVICE_ALL_ACCESS);
	if(NULL==schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	ControlService(schService,SERVICE_CONTROL_STOP,&svcStatus);
	if(!DeleteService(schService))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}
HANDLE OpenDevice(WCHAR* szDeviceName)
{
	//������������  
	HANDLE hDevice = CreateFile(szDeviceName,  
		GENERIC_WRITE | GENERIC_READ,  
		0,  
		NULL,  
		OPEN_EXISTING,  
		0,  
		NULL);  
	if( hDevice != INVALID_HANDLE_VALUE )  
	{
		OutputDebugString(TEXT("Create Device ok ! \n"));  
	}
	else  
	{
		OutputDebugString(TEXT("Create Device faild! \n")); 
		return NULL;
	}

	return hDevice;
}