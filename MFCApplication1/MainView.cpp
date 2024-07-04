﻿// MainView.cpp: 구현 파일
//

#include "pch.h"
#include "afxdialogex.h"
#include "MainView.h"


// MainView 대화 상자

IMPLEMENT_DYNAMIC(MainView, CDialog)

//private function

template <typename T>
void MainView::DynamicMemoryAllocation(T** val) {
	if (*val != nullptr) {
		ErrorMessage("[MAIN] : Aleady exist variable");
	}

	*val = new T;
	//ZeroMemory(*val, sizeof(T));
}

bool MainView::InitFriendCtlDlg() {
	friendControlDlg = new FriendControlView(this);

	return true;
}

void MainView::InitDialog() {
	statusNameLabel.SetWindowTextW(myAccountInfo->name);
	InitTabView();
	if (!InitFriendCtlDlg()) {
		ErrorMessage("[MAIN] Somethings wrong check your code");
	}
	
}
void MainView::InitTabView() {
	tabView.InsertItem(0, L"친구");
	tabView.InsertItem(1, L"설정");
}
bool MainView::InitFriendList() {
	int friendInfoCnt = friendInfos.size();
	if (friendInfoCnt <= 0) {
		WarningMessage("[MAIN] : Failed to Initialize Friend-List");
		return false;
	}

	for (int cFriendInfo = 0; cFriendInfo < friendInfoCnt; cFriendInfo++) {
		if (friendInfos[cFriendInfo]->friending == TRUE) {
			friendListBox.AddString(ConvertCtoWC(friendInfos[cFriendInfo]->userName.c_str()));
		}
			//friendListBox.AddString(L"friendInfos[cFriendInfo]->userName.c_str())");
		else {
			//친구 요청 처리 만들어야함
			//friendControlDlg->UpdateRequestFriendList(ConvertCtoWC(friendInfos[cFriendInfo]->userName.c_str()));
		}
	}

	return true;
}
bool MainView::SetFirendGridView(bool set) {
	int status = -1;
	
	if (!set) status = SW_HIDE;
	else status = SW_SHOW;
	
	friendArea.ShowWindow(status);
	friendListBox.ShowWindow(status);
	statusNameLabel.ShowWindow(status);
	addFriendBtn.ShowWindow(status);
	talkBtn.ShowWindow(status);
	logoutBtn.ShowWindow(status);
	logoImg.ShowWindow(status);

	return true;
}

bool MainView::RequestFriendsInfo() {
	DataHeaders dataHeaders = { NULL, };
	/*AccountInfo* myAccountInfo = nullptr;*/
	
	dataHeaders.dataType = REQUEST_FRIEND_INFO;
	dataHeaders.dataSize = sizeof(FriendInfo);
	dataHeaders.dataCnt = 1;

	/*if (myAccountInfo != nullptr) {
		WarningMessage("[MAIN] : Failed to request friend infos");
		return false;
	}*/

	/*myAccountInfo = new AccountInfo;
	ZeroMemory(myAccountInfo, sizeof(AccountInfo));*/

	if (!bClient->SendData(&dataHeaders, this->myAccountInfo)) {
		WarningMessage("[MAIN] : Failed to request friend infos");
		return false;
	}

	if (bClient->RespondData() == ERROR) {
		WarningMessage("[MAIN] : Failed to respond friend infos");
		return false;
	}

	auto result = bClient->RecvData(&friendInfos);
	if (result == ERROR) {
		WarningMessage("[MAIN] : Failed to recv friend infos");
		return false;
	}

	return true;
}
bool MainView::SetSettingGridView(bool set) {

	return true;
}
MainView::MainView(BgyClient* client,AccountInfo* ac, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAIN_VIEW, pParent), bClient(client)
{
	this->myAccountInfo = new AccountInfo;
	memcpy(this->myAccountInfo, ac, sizeof(AccountInfo));

	if (!RequestFriendsInfo()) {
		ErrorMessage("Failed to Initialize main-view");
	}
}
MainView::~MainView()
{
	for (int cnt = 0; cnt < friendInfos.size(); cnt++) {
		delete(friendInfos[cnt]);

		friendInfos.clear();
		delete(myAccountInfo);
	}
}
void MainView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, TAB_VIEW, tabView);
	DDX_Control(pDX, FRIEND_LIST, friendListBox);
	DDX_Control(pDX, FRIEND_AREA, friendArea);

	DDX_Control(pDX, STATUS_NAME, statusNameLabel);
	DDX_Control(pDX, ADD_FRIEND_BTN, addFriendBtn);
	DDX_Control(pDX, TALK_BTN, talkBtn);
	DDX_Control(pDX, LOGOUT_BTN, logoutBtn);
	DDX_Control(pDX, IDC_LOGIN_IMAGE, logoImg);

	//친구 추가 변수 불러오기
	/*DDX_Control(pDX, REQUST_ADD_EDIT, *friendAddEdit);
	DDX_Control(pDX, FRIEND_REQUST_LISBOX, *friendRequstBox);
	DDX_Control(pDX, FRIEND_ADD_BTN, *friendAddBtn);
	DDX_Control(pDX, REQUST_ACCEPT_BTN, *acceptBtn);
	DDX_Control(pDX, REQUST_REJECT_BTN, *rejectBtn);*/

	InitDialog();
	if (!InitFriendList()) {
		ErrorMessage("[MAIN] Somethings wrong check your code");
	}
}


BEGIN_MESSAGE_MAP(MainView, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, TAB_VIEW, &MainView::OnSelchangeTabView)
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(ADD_FRIEND_BTN, &MainView::FriendCtlBtnClicked)
END_MESSAGE_MAP()


// MainView 메시지 처리기


void MainView::OnSelchangeTabView(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	int currentView = 0;

	currentView = tabView.GetCurSel();

	switch (currentView) {
	case TABVIEWTYPE::FRIEND:
		SetFirendGridView(true);
		break;
	case TABVIEWTYPE::SETTING:
		SetFirendGridView(false);
		break;
	}

}

void MainView::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CDialog::OnSysCommand(nID, lParam);
}
void MainView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.
}

void MainView::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CDialog::OnClose();
	this->EndDialog(0);
	auto pDlg = (CDialog*)this->GetParent();
	pDlg->EndDialog(0);
}
void MainView::OnDestroy()
{
	CDialog::OnDestroy();
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	this->EndDialog(0);
	auto pDlg = (CDialog*)this->GetParent();
	pDlg->EndDialog(0);
}
void MainView::FriendCtlBtnClicked() {
	friendControlDlg->DoModal();
}