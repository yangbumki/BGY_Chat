#pragma once
#include "afxdialogex.h"
#include "resource.h"


// FriendControlView 대화 상자

class FriendControlView : public CDialogEx
{
	DECLARE_DYNAMIC(FriendControlView)

private:
	//친구 추가 변수
	CEdit* friendAddEdit = nullptr;
	CListBox* friendRequestBox = nullptr;
	CButton* friendAddBtn = nullptr;
	CButton* acceptBtn = nullptr;
	CButton* rejectBtn = nullptr;

	template <typename T>
	void DynamicMemoryAllocation(T** val);
	bool InitializeVariable();

public:
	FriendControlView(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~FriendControlView();

	bool UpdateRequestFriendList(wchar_t* name);

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRIEND_CONTROL_VIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
};
