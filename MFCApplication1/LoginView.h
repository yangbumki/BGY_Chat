
// MFCApplication1Dlg.h: 헤더 파일
//

#pragma once

#include <atlimage.h>
#include "SignupDlg.h"


// CMFCApplication1Dlg 대화 상자
typedef class LOGIN_VIEW : public CDialogEx
{
// 생성입니다.
public:
	LOGIN_VIEW(BgyClient* pClient, CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif
private:
	BgyClient* client			= nullptr;
	CStatic* mainImageControl	= nullptr;
	CEdit* loginIDEditBox		= nullptr;
	CEdit* loginPasswdEditBox	= nullptr;
	
	bool InitMainLogo(const wchar_t* path);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	CString dlgTitle;
	

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedLogin();
	afx_msg void OnBnClickedSignUp();
}LoginView;
