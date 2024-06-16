#pragma once
#include "afxdialogex.h"


// SignupDlg 대화 상자

class SignupDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SignupDlg)

public:
	SignupDlg(BgyClient* client, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~SignupDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SINGUP };
#endif

private:
	BgyClient* client = nullptr;

	CEdit* editID = nullptr;
	CEdit* editPasswd = nullptr;
	CEdit* editName = nullptr;
	
	CDateTimeCtrl* dateTime = nullptr;
	CTime* dateMin = nullptr;
	CTime* dateMax = nullptr;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSignUp();
	afx_msg void OnBnClickedCancel();
};
