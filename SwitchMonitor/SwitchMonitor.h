
// SwitchMonitor.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSwitchMonitorApp:
// �йش����ʵ�֣������ SwitchMonitor.cpp
//

class CSwitchMonitorApp : public CWinApp
{
public:
	CSwitchMonitorApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSwitchMonitorApp theApp;