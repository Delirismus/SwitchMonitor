
// SwitchMonitorDlg.h : ͷ�ļ�
//

#pragma once


// CSwitchMonitorDlg �Ի���
class CSwitchMonitorDlg : public CDialogEx
{
// ����
public:
	CSwitchMonitorDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SWITCHMONITOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	sqlite3* Global_KeyDb;
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedSelect();
};
