#pragma once
#include "afxdialogex.h"
#include "resource.h"

//#include "LoginView.h"
#include <vector>

// MainView 대화 상자

class MainView : public CDialog
{
	DECLARE_DYNAMIC(MainView)

private:	
	typedef enum TAB_VIEW_TYPE {
		FRIEND = 0,
		SETTING
	}TabViewType, TABVIEWTYPE;

	AccountInfo* myAccountInfo = nullptr;
	std::vector<FriendInfo*> friendInfos = {};

	CTabCtrl tabView;
	BgyClient* bClient = nullptr;
	CListBox friendListBox;
	CStatic friendArea;
	CStatic statusNameLabel;
	CButton addFriendBtn;
	CButton talkBtn;
	CButton logoutBtn;

	void InitDialog();
	void InitTabView();

	bool SetFirendGridView(bool set);
	bool SetSettingGridView(bool set);

	bool RequestFriendsInfo();
	

public:
	MainView(BgyClient* client,AccountInfo* ac, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~MainView();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_VIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSelchangeTabView(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnClose();
	afx_msg void OnDestroy();
	CStatic logoImg;
};
