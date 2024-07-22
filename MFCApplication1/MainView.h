#pragma once
#include "afxdialogex.h"
#include "resource.h"

//#include "LoginView.h"
#include "FriendControlView.h"
#include "ChatView.h"
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
	BgyClient* bClient			= nullptr;
	CListBox* friendListBox		= nullptr;
	CStatic friendArea;
	CStatic statusNameLabel;
	CButton addFriendBtn;
	CButton talkBtn;
	CButton logoutBtn;

	//친구추가 다이어로그 변수
	FriendControlView* friendControlDlg	= nullptr;
	//대화 다이어로그 변수
	//대화 창을 여러개 생성 할 수 있으니 해당 다이어로그 벡터로 관리
	std::vector<ChatView*> chatViewDlgs = {};

	//CEdit* friendAddEdit		= nullptr;
	//CListBox* friendRequstBox	= nullptr;
	//CButton* friendAddBtn		= nullptr;
	//CButton* acceptBtn			= nullptr;
	//CButton* rejectBtn			= nullptr;

	template <typename T>
	void DynamicMemoryAllocation(T** val);
	bool InitFriendCtlDlg();
	

	void InitTabView();
	bool InitFriendList();

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
	 BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSelchangeTabView(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnClose();
	afx_msg void OnDestroy();

	//친구 추가 버튼 함수
	afx_msg void FriendCtlBtnClicked();
	//대화하기 버튼 함수
	afx_msg void TalkBtnClicked();
	CStatic logoImg;
};
