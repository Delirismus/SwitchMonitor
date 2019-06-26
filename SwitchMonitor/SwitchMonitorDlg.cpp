
// SwitchMonitorDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SwitchMonitor.h"
#include "SwitchMonitorDlg.h"
#include "afxdialogex.h"
#include "DataBase.h"
#include "CheckExamples.h"

#include "KeyboardHook.h"

#include <FltUser.h>
#include "ServiceFun.h"
#include "MiniDrvLoader.h"
#include <shlobj.h>

#pragma comment(lib,"fltLib.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "CaptureCamera.h"
#include "UserCmdMsg.h"
// CSwitchMonitorDlg �Ի���


PQUICK_KEY_LIST pQuickKeyList;

HANDLE g_hEvent = NULL;
HANDLE g_hPort = NULL;
PMONITOR_STATUS pMonitorStatus = NULL;

DWORD WINAPI DoWorker(PMONITOR_STATUS pTmp)
{
	WCHAR OutTime[MAX_PATH];

	PMONITOR_STATUS pMonitorStatus = NULL;
	RtlZeroMemory(OutTime,sizeof(WCHAR) * MAX_PATH);
	pMonitorStatus = pTmp;
	
	InitializeCaptureCamera(pMonitorStatus);
	if (pMonitorStatus->bIsCamera == TRUE && \
		pMonitorStatus->hCapWnd != NULL && \
		pMonitorStatus->bIsMonitor == TRUE && \
		pMonitorStatus->g_bIsShowWindow == FALSE)
	{
		while (WaitForSingleObject(pMonitorStatus->g_hAttached,INFINITE))
		{
			//GetCurTime(pMonitorStatus->SavePicPath,OutTime,MAX_PATH,TEXT("bmp"));
			//Photo(pMonitorStatus,OutTime);
			OutputDebugString(TEXT("Capture Photo\r\n"));
		}
	}
	CloseCapture(pMonitorStatus);
	return 0;
}
DWORD WINAPI ControlSwitchDrv(PMSG_THREAD_CONTEXT pMsgContext)
{
	PCOMMAND_MESSAGE message;
	USERCOMMAND_MESSAGE_REPLAY UserMsgReply;
	PUSER_COMMAND_MESSAGE pUserCmdMsg;
	LPOVERLAPPED pOvlp;
	BOOL bResult;
	DWORD dwOutSize;
	HRESULT hr;
	ULONG_PTR key;

	message = NULL;
	pUserCmdMsg = NULL;
	RtlZeroMemory(&UserMsgReply,sizeof(USERCOMMAND_MESSAGE_REPLAY));
	while (TRUE)
	{
		bResult = GetQueuedCompletionStatus(pMsgContext->hCompletion,&dwOutSize,&key,&pOvlp,INFINITE);
		pUserCmdMsg = CONTAINING_RECORD(pOvlp,USER_COMMAND_MESSAGE,Ovlp);
		if (!bResult)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}
		message = &pUserCmdMsg->CommandMsg;
		//Start
		//
		OutputDebugString((WCHAR*)message->ShowMsg);
		//MessageBox(NULL,(WCHAR*)message->ShowMsg,TEXT("��ʾ"),MB_OK);
		//
		//Stop
		UserMsgReply.ReplayHeader.Status = 0;
		UserMsgReply.ReplayHeader.MessageId = pUserCmdMsg->MsgHeader.MessageId;

		UserMsgReply.ReplyMsg.bIsSuccess = TRUE;

		hr = FilterReplyMessage(pMsgContext->hPort,(PFILTER_REPLY_HEADER)&UserMsgReply,sizeof(FILTER_REPLY_HEADER) + sizeof(REPLY_MSG));
		if (SUCCEEDED(hr))
		{
			OutputDebugString(TEXT("Replied message\r\n"));
		}
		else
		{
			OutputDebugString(TEXT("Replying message error\r\n"));
			break;
		}
		RtlZeroMemory(&pUserCmdMsg->Ovlp,sizeof(OVERLAPPED));
		hr = FilterGetMessage(pMsgContext->hPort,&pUserCmdMsg->MsgHeader,FIELD_OFFSET(USER_COMMAND_MESSAGE,Ovlp),&pUserCmdMsg->Ovlp);
		if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
		{
			break;
		}
	}

	if (!SUCCEEDED(hr))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE))
		{
			printf("Scanner: Port is disconnected, probably due to scanner filter unloading.\n");

		} 
		else 
		{

			printf("Scanner: Unknown error occured. Error = 0x%X\n",hr);
		}
	}
	free(pUserCmdMsg);
	return hr;
}
int InitializeDrv()
{
	HRESULT hResult = 0;
	DWORD dwThreadId = 0;
	WCHAR ShowMsg[MAX_PATH];
	WCHAR MsgBuf[MAX_PATH];
	DWORD dwRetByte = 0;
	BOOL bRet = FALSE;
	SYSTEM_INFO SystemInfo;
	DWORD dwProcessors = 0;
	HANDLE hCompletePort = NULL;
	PUSER_COMMAND_MESSAGE pUserMsg;
	MSG_THREAD_CONTEXT MsgThreadContext;
	int i = 0;;

	HANDLE hThreads[MAX_THREAD_COUNT];
	hResult = 0;
	pUserMsg = NULL;

	RtlZeroMemory(ShowMsg,sizeof(WCHAR)*MAX_PATH);
	RtlZeroMemory(MsgBuf,sizeof(WCHAR)*MAX_PATH);

	//��������Ƿ����
	if (ChkHasServc(SERVICE_NAME))
	{
		//MessageBox(NULL,TEXT("�������Ѿ���װ!"),TEXT("��ʾ"),MB_OK);
		//		return 0;
	}

	//��װ����
	BOOL bInst = InstallMiniDriver(SERVICE_NAME,DRIVER_NAME,ALTITUDE);
	if (bInst)
	{
		//MessageBox(NULL,TEXT("��װ�ɹ�!"),TEXT("��ʾ"),MB_OK);
	}
	else
	{
		//MessageBox(NULL,TEXT("��װʧ��!"),TEXT("��ʾ"),MB_OK);
		return 0;
	}

	if (ChkServcRun(SERVICE_NAME))
	{
		//MessageBox(NULL,TEXT("����������!"),TEXT("��ʾ"),MB_OK);
		return 0;
	}

	//��������
	BOOL bStart = FALSE;
	while (!bStart)
	{
		bStart = StartMiniDriver(SERVICE_NAME);
		if (bStart)
		{
			//MessageBox(NULL,TEXT("��������ɹ�!"),TEXT("��ʾ"),MB_OK);
		}
		else
		{
			MessageBox(NULL,TEXT("��������ʧ��!"),TEXT("��ʾ"),MB_OK);
			return 0;
		}
	}
	pMonitorStatus->hDrvDevice = OpenDevice(DEVICE_NAME);
	if (pMonitorStatus->hDrvDevice == NULL)
	{
		return 0;
	}
	bRet = DeviceIoControl(pMonitorStatus->hDrvDevice,SWITCHDRV_INITIAL_GLOBAL,NULL,0,NULL,0,&dwRetByte,NULL);
	if (bRet == FALSE)
	{
		return 0;
	}
	//MessageBox(NULL,TEXT("������ʼ���ɹ�!"),TEXT("��ʾ"),MB_OK);

	bRet = DeviceIoControl(pMonitorStatus->hDrvDevice,SWITCHDRV_GET_EXPLORER,NULL,0,NULL,0,&dwRetByte,NULL);
	if (bRet == FALSE)
	{
		MessageBox(NULL,TEXT("��ȡ \"Explorer.exe\"��������ʧ��!"),TEXT("��ʾ"),MB_OK);
		return 0;
	}
	StringCchCopy(MsgBuf,MAX_PATH,TEXT("Test connect!"));
	hResult = FilterConnectCommunicationPort(SWITCHDRV_PORT_NAME,0,MsgBuf,(WORD)wcslen(MsgBuf),NULL,&g_hPort);
	if (hResult == S_OK)
	{
		//MessageBox(NULL,TEXT("���Ӷ˿ڳɹ�!"),TEXT("��ʾ"),MB_OK);

		GetSystemInfo(&SystemInfo);
		dwProcessors = SystemInfo.dwNumberOfProcessors;
		DWORD dwThreadCount = dwProcessors * 2;
		if (dwThreadCount >= MAX_THREAD_COUNT)
		{
			dwThreadCount = MAX_THREAD_COUNT;
		}
		hCompletePort = CreateIoCompletionPort(g_hPort,NULL,0,dwThreadCount);
		if (hCompletePort == NULL)
		{
			//MessageBox(NULL,TEXT("������ɶ˿�ʧ��!"),TEXT("��ʾ"),MB_OK);
			return 0;
		}

		MsgThreadContext.hPort = g_hPort;
		MsgThreadContext.hCompletion = hCompletePort;
		for (i = 0;i < dwThreadCount;i++)
		{
			hThreads[i] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ControlSwitchDrv,&MsgThreadContext,0,&dwThreadId);
			if (hThreads[i] == NULL)
			{
				goto main_cleanup;
			}
		}
		for (int j = 0;j < dwThreadCount;j++)
		{
			pUserMsg = (PUSER_COMMAND_MESSAGE)malloc(sizeof(USER_COMMAND_MESSAGE));
			if (pUserMsg == NULL)
			{
				goto main_cleanup;
			}
			RtlZeroMemory(&pUserMsg->Ovlp,sizeof(OVERLAPPED));
			hResult = FilterGetMessage(g_hPort, \
				&pUserMsg->MsgHeader, \
				FIELD_OFFSET(USER_COMMAND_MESSAGE,Ovlp), \
				&pUserMsg->Ovlp);
			if (hResult != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
			{
				free(pUserMsg);
				goto main_cleanup;
			}
		}
	}
	hResult = S_OK;
	WaitForMultipleObjectsEx(i,hThreads,TRUE,INFINITE,FALSE);
main_cleanup:

	if (g_hPort)
	{
		CloseHandle(g_hPort);
		g_hPort = NULL;
	}
	if (g_hEvent)
	{
		CloseHandle(g_hEvent);
		g_hEvent = NULL;
	}
	return -1;
}
CSwitchMonitorDlg::CSwitchMonitorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSwitchMonitorDlg::IDD, pParent)
	, Global_KeyDb(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSwitchMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSwitchMonitorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HOTKEY()
	ON_WM_DESTROY()
	ON_BN_CLICKED(BTN_SELECT, &CSwitchMonitorDlg::OnBnClickedSelect)
END_MESSAGE_MAP()


// CSwitchMonitorDlg ��Ϣ�������

BOOL CSwitchMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	DWORD dwRet = 0;
	BOOL bRet = FALSE;
	int nMaxCount = 0;
	Global_KeyDb = NULL;
	pQuickKeyList = NULL;
	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	dwRet = CheckExamples();
	if (dwRet == 0)
	{
		return FALSE;
	}
	else if (dwRet == 1)
	{
		OutputDebugString(TEXT("First Run!\n"));

		do 
		{
			pMonitorStatus = (PMONITOR_STATUS)malloc(sizeof(MONITOR_STATUS));
		} while (pMonitorStatus == NULL);
		RtlZeroMemory(pMonitorStatus,sizeof(MONITOR_STATUS));

		pMonitorStatus->bIsMonitor = FALSE;
		pMonitorStatus->g_bIsShowWindow = TRUE;

		do 
		{
			pMonitorStatus->g_hAttached = CreateEvent(NULL,FALSE,FALSE,TEXT("��ʼ����"));
		} while (pMonitorStatus->g_hAttached == NULL);

		do 
		{
			pMonitorStatus->hThread = CreateThread(NULL, \
				0, \
				(LPTHREAD_START_ROUTINE)DoWorker, \
				pMonitorStatus, \
				CREATE_SUSPENDED, \
				&pMonitorStatus->dwThreadId);

		} while (pMonitorStatus->hThread == NULL);

		bRet = InitializeHookData(TEXT("InjectKeyBoard.dll"),"SetKeyBoard",TEXT("InjectMouse.dll"),"SetMouse");
		if (bRet == TRUE)
		{
			//MessageBox(TEXT("��װ���ӳɹ�"),TEXT("��ʾ"),MB_OK);
		}
		InitializeDrv();
		Global_KeyDb = OpenDataBase();
		if (Global_KeyDb != NULL)
		{
			bRet = GetMaxCountDataBase(Global_KeyDb,&nMaxCount);
			if (bRet == TRUE && nMaxCount > 0)
			{
				pQuickKeyList = (PQUICK_KEY_LIST)malloc(sizeof(QUICK_KEY_LIST));
				if (pQuickKeyList == NULL)
				{
					OutputDebugString(TEXT("pQuickKeyList malloc() failed!\n"));
					return FALSE;
				}
				RtlZeroMemory(pQuickKeyList,sizeof(QUICK_KEY_LIST));
				pQuickKeyList->QuickKey = (PQUICK_KEY)malloc(sizeof(QUICK_KEY) * nMaxCount + 10);
				if (pQuickKeyList->QuickKey == NULL)
				{
					OutputDebugString(TEXT("pQuickKeyList->QuickKey malloc() failed!\n"));
					return FALSE;
				}
				RtlZeroMemory(pQuickKeyList->QuickKey,sizeof(QUICK_KEY) * nMaxCount + 10);

				pQuickKeyList->nMaxCount = nMaxCount + 10;
				pQuickKeyList->nCurCount = 0;
				bRet = GetKeyToDataBase(Global_KeyDb,pQuickKeyList);
				if (bRet == TRUE)
				{
					for (int i = 0;i < pQuickKeyList->nCurCount;i++)
					{
						RegisterHotKey(m_hWnd,pQuickKeyList->QuickKey[i].nUserDefine,pQuickKeyList->QuickKey[i].nKeyValue,pQuickKeyList->QuickKey[i].SubKeyValue);
					}
					CloseDataBase(Global_KeyDb);
				}
			}
			else
			{
				//default call
				//register Show Hide dialog hotkey
				RegisterHotKey(m_hWnd,ID_ACTIVATING,MOD_ALT,'A');
				SetKeyToDataBase(Global_KeyDb,ID_ACTIVATING,"Activating",MOD_ALT,'A');

				RegisterHotKey(m_hWnd,ID_STARTMONITOR,MOD_ALT,'S');
				SetKeyToDataBase(Global_KeyDb,ID_STARTMONITOR,"StartAttach",MOD_ALT,'S');

				CloseDataBase(Global_KeyDb);
			}
		}
		return TRUE;
	}else if (dwRet == 2)
	{
		OutputDebugString(TEXT("Mutex Run"));
		MessageBox(TEXT("�����Ѿ�����."),TEXT("Caption"),MB_OK);
		ExitProcess(0);
		return FALSE;
	}else
	{
		OutputDebugString(TEXT("Unknow"));
		MessageBox(TEXT("�����Ѿ����л������쳣�������ظ�����"),TEXT("Caption"),MB_OK);
		ExitProcess(0);
		return FALSE;
	}
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSwitchMonitorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSwitchMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSwitchMonitorDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	// TODO: Add your message handler code here and/or call default
	BOOL bShow = FALSE;
	DWORD dwRetByte = 0;
	BOOLEAN bRet = FALSE;

	if (nHotKeyId == ID_ACTIVATING)
	{
		if (nKey1 == MOD_ALT)
		{
			if (nKey2 == 'A' || nKey2 == 'a')
			{
				OutputDebugString(TEXT("Activating!\n"));
				bShow = ::IsWindowVisible(m_hWnd);
				if(bShow == TRUE)
				{
					this->ShowWindow(SW_HIDE);
					bRet = DeviceIoControl(pMonitorStatus->hDrvDevice,SWIRCHDRV_SET_WINDOW_HIDE,NULL,0,NULL,0,&dwRetByte,NULL);
					if (bRet == TRUE)
					{
						OutputDebugString(TEXT("Set window Hide\r\n"));
					}
					pMonitorStatus->g_bIsShowWindow = FALSE;
				}
				else
				{
					bRet = DeviceIoControl(pMonitorStatus->hDrvDevice,SWITCHDRV_SET_WINDOW_SHOW,NULL,0,NULL,0,&dwRetByte,NULL);
					if (bRet == TRUE)
					{
						OutputDebugString(TEXT("Set window Show\r\n"));
					}
					pMonitorStatus->g_bIsShowWindow = TRUE;
					this->ShowWindow(SW_SHOW);
				}
			}
		}
	}
	if (nHotKeyId == ID_STARTMONITOR)
	{
		if (nKey1 == MOD_ALT)
		{
			if (nKey2 == 'S' || nKey2 == 's')
			{
				OutputDebugString(TEXT("Start Attached!\n"));
				if (pMonitorStatus->bIsMonitor == FALSE)
				{
					if (StartKeyBoardMonitor(pMonitorStatus) && StartMouseMonitor(pMonitorStatus) && \
						DeviceIoControl(pMonitorStatus->hDrvDevice,SWITCHDRV_START_MONITOR,NULL,0,NULL,0,&dwRetByte,NULL))
					{
						pMonitorStatus->bIsMonitor = TRUE;
						ResumeThread(pMonitorStatus->hThread);
						MessageBox(TEXT("������!"),TEXT("��ȫ��ʾ"),MB_OK);
					}
				}
				else
				{
					if (StopKeyBoardMonitor() && StopMouseMonitor() && \
						DeviceIoControl(pMonitorStatus->hDrvDevice,SWITCHDRV_STOP_MONITOR,NULL,0,NULL,0,&dwRetByte,NULL))
					{
						pMonitorStatus->bIsMonitor = FALSE;
						SuspendThread(pMonitorStatus->hThread);
						MessageBox(TEXT("�رռ��!"),TEXT("��ȫ��ʾ"),MB_OK);
					}
				}
			}
		}
	}
	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}


void CSwitchMonitorDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	DWORD dwRetByte = 0;
	ReleaseExamples();
	UnInitializeHookData();
	DeviceIoControl(pMonitorStatus->hDrvDevice,SWITCHDRV_UNINIT_GLOBAL,NULL,0,NULL,0,&dwRetByte,NULL);

	for (int i = 0;i < pQuickKeyList->nCurCount;i++)
	{
		UnregisterHotKey(m_hWnd,pQuickKeyList->QuickKey[i].nUserDefine);
	}
	//	UnregisterHotKey(m_hWnd,ID_ACTIVATING);
	if (pQuickKeyList!= NULL)
	{
		if (pQuickKeyList->QuickKey != NULL)
		{
			free(pQuickKeyList->QuickKey);
			pQuickKeyList->QuickKey = NULL;
		}
		free(pQuickKeyList);
		pQuickKeyList = NULL;
	}
	if (pMonitorStatus)
	{
		free(pMonitorStatus);
		pMonitorStatus = NULL;
	}
	// TODO: Add your message handler code here
}


void CSwitchMonitorDlg::OnBnClickedSelect()
{
	// TODO: Add your control notification handler code here
	WCHAR SelectPath[MAX_PATH];
	BROWSEINFO BrowseInfo;
	LPITEMIDLIST lpItemIdList;

	RtlZeroMemory(SelectPath,sizeof(WCHAR) * MAX_PATH);
	InitCommonControls();

	BrowseInfo.hwndOwner = NULL;
	BrowseInfo.pidlRoot = NULL;
	BrowseInfo.pszDisplayName = TEXT("Ŀ¼ѡ��");
	BrowseInfo.lpszTitle = TEXT("��ѡ��Ŀ¼");

	BrowseInfo.ulFlags = BIF_STATUSTEXT | BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
	BrowseInfo.lpfn = NULL;
	BrowseInfo.lParam = 0;
	BrowseInfo.iImage = 0;
	lpItemIdList = SHBrowseForFolder(&BrowseInfo);
	if (lpItemIdList == NULL)
	{
		return;
	}
	if (!SHGetPathFromIDList(lpItemIdList,SelectPath))
	{
		return;
	}
	SetDlgItemText(EDT_SELECT,SelectPath);
	OutputDebugString(SelectPath);
}
