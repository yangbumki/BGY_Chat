// MainView.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BCHAT_APPLICATION.h"
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
}

bool MainView::InitFriendCtlDlg() {
	friendControlDlg = new FriendControlView(bClient, this->myAccountInfo, this);

	return true;
}
BOOL MainView::OnInitDialog() {
	CDialog::OnInitDialog();

	statusNameLabel.SetWindowTextW(myAccountInfo->name);
	InitTabView();
	InitFriendCtlDlg();
	InitFriendList();

	return true;
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
		if (friendListBox == nullptr) {
			return false;
		}
		if (friendInfos[cFriendInfo]->friending == TRUE) {
			friendListBox->AddString(ConvertCtoWC(friendInfos[cFriendInfo]->userID.c_str()));
		}
		else {
			//친구 요청 처리 만들어야함
			if (friendControlDlg == nullptr) {
				WarningMessage("[MAIN] : Failed to Initialize friendCtlDlg");
				return false;
			}

			friendControlDlg->UpdateRequestFriendList(ConvertCtoWC(friendInfos[cFriendInfo]->userID.c_str()));
		}
	}

	return true;
}
bool MainView::SetFirendGridView(bool set) {
	int status = -1;

	if (!set) status = SW_HIDE;
	else status = SW_SHOW;

	friendArea.ShowWindow(status);
	friendListBox->ShowWindow(status);
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
MainView::MainView(BgyClient* client, AccountInfo* ac, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_MAIN_VIEW, pParent), bClient(client)
{
	DynamicMemoryAllocation(&myAccountInfo);
	memcpy(this->myAccountInfo, ac, sizeof(AccountInfo));

	DynamicMemoryAllocation(&friendListBox);

	if (!RequestFriendsInfo()) {
		ErrorMessage("Failed to Initialize main-view");
	}
}
MainView::~MainView()
{
	//친구 정보 값 삭제
	for (int cnt = 0; cnt < friendInfos.size(); cnt++) {
		delete(friendInfos[cnt]);
		friendInfos.clear();
	}
	//채팅 다이어로그 값 삭제
	for (int cnt = 0; cnt < chatViewDlgs.size(); cnt++) {
		delete(chatViewDlgs[cnt]);
		chatViewDlgs.clear();
	}

	//내정보 삭제
	delete(myAccountInfo);
}
void MainView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, TAB_VIEW, tabView);
	DDX_Control(pDX, FRIEND_LIST, *friendListBox);
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
}

BEGIN_MESSAGE_MAP(MainView, CDialog)
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, TAB_VIEW, &MainView::OnSelchangeTabView)
	ON_BN_CLICKED(ADD_FRIEND_BTN, &MainView::FriendCtlBtnClicked)
	ON_BN_CLICKED(TALK_BTN, &MainView::TalkBtnClicked)
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
	//재생성
	if (friendControlDlg == nullptr) {
		InitFriendCtlDlg();
		//친구 목록 불러오기
		InitFriendList();
	}
	friendControlDlg->DoModal();

	//정리 
	delete(friendControlDlg);
	friendControlDlg = nullptr;
}
void MainView::TalkBtnClicked() {
	//현재 선택된 리스트 번호
	int curSel = friendListBox->GetCurSel();
	if (curSel == ERROR) {
		WarningMessage("[MAIN_VIEW] : Failed to talk : nothing selected");
		return;
	}

	//선택된 친구 정보
	//정보 넣을 변수
	CString curlStr;

	//정보 대입
	friendListBox->GetText(curSel, curlStr);


	curlStr.GetString();

	//동일 대상 다이어로그 판단 변수
	//기본 true 값으로 설정하여 데이터가 존재하지 않을 경우 다이어로그 생성 조건 만족
	byte aleadyExistDlg = false;
	//동일한 대상인지 비교하는 비교 메소드
	//모든 대상을 비교하는 형태로 제작
	//24-07-13 윈도우 생성은 성공하였으나, 다른 윈도우 생성할려니까 에러 발생
	//chatViewDlgs[curCel] -> chatViewDlgs[cnt] 로 변경 에러 수정
	int maxCnt = chatViewDlgs.size();
	if (maxCnt > 0) {
		for (int cnt = 0; cnt < maxCnt; cnt++) {
			auto result = wcscmp(curlStr.GetString(), chatViewDlgs[cnt]->GetChatTargetInfo());
			//동일한 대상이 존재할 경우 
			if (result == 0) {
				aleadyExistDlg = !result;
				break;
			}
		}
	}
	//기존 데이터가 존재 하지 않을 경우 dlg true 값 입력
	//else aleadyExistDlg = true;
	//존재 하지 않을 경우
	if (!aleadyExistDlg) {
		//다이어로그 동적 생성
		ChatView* tmpCv = new ChatView((wchar_t*)curlStr.GetString());
		//동적생성이 실패했을 경우
		if (tmpCv == nullptr) {
			WarningMessage("[MAIN_VIEW] : Failed to allocate chat-dlg");
			return;
		}

		chatViewDlgs.push_back(tmpCv);
		// 윈도우 생성
		// tmpCv->DoModal();
		tmpCv->Create(IDD_CHAT_VIEW);
		tmpCv->ShowWindow(SW_SHOW);
	}
	//존재 할 경우
}