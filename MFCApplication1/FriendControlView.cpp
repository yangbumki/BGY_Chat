// FriendControlView.cpp: 구현 파일
//

#include "pch.h"
#include "afxdialogex.h"
#include "FriendControlView.h"


// FriendControlView 대화 상자

IMPLEMENT_DYNAMIC(FriendControlView, CDialogEx)



FriendControlView::FriendControlView(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FRIEND_CONTROL_VIEW, pParent)
{
}

FriendControlView::~FriendControlView()
{
}

bool FriendControlView::UpdateRequestFriendList(wchar_t* name) {
	if (this->friendRequestBox == nullptr) {
		WarningMessage("[FRIEND-VIEW] Failed to update request-friend-list");
		return false;
	}
	friendRequestBox->AddString(name);
	return true;
}

template <typename T>
void FriendControlView::DynamicMemoryAllocation(T** val) {
	if (*val != nullptr) {
		ErrorMessage("[MAIN] : Aleady exist variable");
	}

	*val = new T;
}

bool FriendControlView::InitializeVariable() {
	DynamicMemoryAllocation(&friendAddEdit);
	DynamicMemoryAllocation(&friendRequestBox);
	DynamicMemoryAllocation(&friendAddBtn);
	DynamicMemoryAllocation(&acceptBtn);
	DynamicMemoryAllocation(&rejectBtn);

	return true;
}

void FriendControlView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	//친구 추가 변수 불러오기

	if (!InitializeVariable()) {
		ErrorMessage("[MAIN] : Failed to InitFriendCtlDlg");
	}
	
	DDX_Control(pDX, REQUST_ADD_EDIT, *friendAddEdit);
	DDX_Control(pDX, FRIEND_REQUST_LISBOX, *friendRequestBox);
	DDX_Control(pDX, FRIEND_ADD_BTN, *friendAddBtn);
	DDX_Control(pDX, REQUST_ACCEPT_BTN, *acceptBtn);
	DDX_Control(pDX, REQUST_REJECT_BTN, *rejectBtn);	
}


BEGIN_MESSAGE_MAP(FriendControlView, CDialogEx)
END_MESSAGE_MAP()


// FriendControlView 메시지 처리기
