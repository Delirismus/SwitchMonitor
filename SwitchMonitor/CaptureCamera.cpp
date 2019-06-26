#include "stdafx.h"

#include "KeyboardHook.h"
#include "CaptureCamera.h"

#include <Vfw.h>
#pragma comment(lib,"vfw32.lib")

CAPDRIVERCAPS m_CapDrvCap;
CAPSTATUS m_CapStatus;
CAPTUREPARMS m_Parms;

void GetCurTime(WCHAR* szCatPath,WCHAR* szOutTime,int SizeOutTime,WCHAR* szSuffix)
{
	BOOL bRet = FALSE;
	SYSTEMTIME SystemTime;
	WCHAR szFormat[MAX_PATH];

	RtlZeroMemory(szFormat,sizeof(WCHAR)*MAX_PATH);
	GetSystemTime(&SystemTime);
	if (szCatPath)
	{
		StringCchPrintf(szFormat,MAX_PATH,TEXT("%s\\%d-%d-%d-%d-%d-%d-%d.%s"), \
			szCatPath, \
			SystemTime.wYear, \
			SystemTime.wMonth, \
			SystemTime.wDay, \
			SystemTime.wHour, \
			SystemTime.wMinute, \
			SystemTime.wSecond, \
			SystemTime.wMilliseconds, \
			szSuffix);
	}
	else
	{
		StringCchPrintf(szFormat,MAX_PATH,TEXT("%d-%d-%d-%d-%d-%d-%d.%s"), \
			SystemTime.wYear, \
			SystemTime.wMonth, \
			SystemTime.wDay, \
			SystemTime.wHour, \
			SystemTime.wMinute, \
			SystemTime.wSecond, \
			SystemTime.wMilliseconds, \
			szSuffix);
	}
	StringCchCopy(szOutTime,SizeOutTime,szFormat);
}
void InitializeCaptureCamera(PMONITOR_STATUS pMonitorStatus)
{
	pMonitorStatus->hCapWnd = capCreateCaptureWindow((LPTSTR)TEXT("��Ƶ��׽���Գ���"),NULL,0,0,0,0,NULL,0); // ����Ԥʾ����
	ASSERT(pMonitorStatus->hCapWnd);
	::ShowWindow(pMonitorStatus->hCapWnd,SW_HIDE);
	if(capDriverConnect(pMonitorStatus->hCapWnd,0))
	{
		//���ӵ�0 ��������
		//�õ�������������
		capDriverGetCaps(pMonitorStatus->hCapWnd,sizeof(CAPDRIVERCAPS),&m_CapDrvCap);

		if(m_CapDrvCap.fCaptureInitialized)
		{
			//�����ʼ���ɹ�
			capGetStatus(pMonitorStatus->hCapWnd, &m_CapStatus,sizeof(m_CapStatus)); // �õ�������״̬
			capPreviewRate(pMonitorStatus->hCapWnd,30); // ����Ԥʾ֡Ƶ
			capPreview(pMonitorStatus->hCapWnd,TRUE); // ����Ԥʾ��ʽ
			pMonitorStatus->bIsCamera = TRUE;
		}
		else
		{
			//��ʼ��δ�ɹ�
			pMonitorStatus->bIsCamera = FALSE;
			OutputDebugString(TEXT("��Ƶ��׽����ʼ��ʧ��!\n"));
			PostMessage(pMonitorStatus->hCapWnd,WM_CLOSE,0,0);
		}
	}
	else
	{
		//δ�����ӵ�������
		OutputDebugString(TEXT("����Ƶ��׽������ʧ��!\n"));
		pMonitorStatus->bIsCamera = FALSE;
	}
}
void Photo(PMONITOR_STATUS pMonitor,WCHAR* pSzSavePath)
{
	if (pMonitor->hCapWnd)
	{
		capCaptureSingleFrame(pMonitor->hCapWnd);
		capFileSaveDIB(pMonitor->hCapWnd,pSzSavePath);
	}
	return;
}
void CloseCapture(PMONITOR_STATUS pMonitor)
{
	if (pMonitor->hCapWnd)
	{
		capDriverDisconnect(pMonitor->hCapWnd);
		pMonitor->hCapWnd = NULL;
	}
}