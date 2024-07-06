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
	for (int cnt = 0; cnt < reqFriendList.size(); cnt++) {
		delete(reqFriendList[cnt]);
	}

	reqFriendList.clear();
}

BOOL FriendControlView::OnInitDialog() {
	CDialogEx::OnInitDialog();
	if (!InitListView()) return false;

	return true;
}

bool FriendControlView::InitListView() {
	auto listTotCnt= reqFriendList.size();
	if (listTotCnt <= 0) {
		WarningMessage("[FRIEND_ADD_VIEW] : There are no listings to add.");
		return false;
	}

	for (int cnt = 0; cnt < listTotCnt; cnt++) {
		#if DEBUGGING
		auto tmp = reqFriendList[cnt]->GetString();
#endif
		friendRequestBox->AddString(reqFriendList[cnt]->GetString());
	}
	
}

bool FriendControlView::UpdateRequestFriendList(wchar_t* name) {
	if (this->friendRequestBox == nullptr) {
		reqFriendList.push_back(new CString(name));

		WarningMessage("[FRIEND_ADD_VIEW] Failed to update request-friend-list");
		return false;
	}

	friendRequestBox->AddString(name);
	return true;
}

template <typename T>
void FriendControlView::DynamicMemoryAllocation(T** val) {
	if (*val != nullptr) {
		ErrorMessage("[FRIEND_ADD_VIEW] : Aleady exist variable");
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
	

	if (!InitializeVariable()) ErrorMessage("[FRIEND_ADD_VIEW] : Failed to initialize variable");

	DDX_Control(pDX, REQUST_ADD_EDIT, *friendAddEdit);
	DDX_Control(pDX, FRIEND_REQUST_LISBOX, *friendRequestBox);
	DDX_Control(pDX, FRIEND_ADD_BTN, *friendAddBtn);
	DDX_Control(pDX, REQUST_ACCEPT_BTN, *acceptBtn);
	DDX_Control(pDX, REQUST_REJECT_BTN, *rejectBtn);
}


BEGIN_MESSAGE_MAP(FriendControlView, CDialogEx)
	ON_BN_CLICKED(REQUST_ACCEPT_BTN, &FriendControlView::ReqAcceptBtnClicked)
END_MESSAGE_MAP()

void FriendControlView::ReqAcceptBtnClicked() {
	int currentItem = friendRequestBox->GetCurSel();

	wchar_t currentStirng[MAX__STRING_LEN] = { 0, };

	friendRequestBox->GetText(currentItem, currentStirng);
	
}

void FriendControlView::ReqRejectBtnClicekd() {
}

// FriendControlView 메시지 처리기
