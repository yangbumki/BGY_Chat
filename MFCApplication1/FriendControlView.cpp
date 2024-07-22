// FriendControlView.cpp: 구현 파일
//

#include "pch.h"
#include "afxdialogex.h"
#include "FriendControlView.h"


// FriendControlView 대화 상자

IMPLEMENT_DYNAMIC(FriendControlView, CDialogEx)



FriendControlView::FriendControlView(BGY_CLIENT* client,AccountInfo* accountInfo, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FRIEND_CONTROL_VIEW, pParent), bClient(client), myAccountInfo(accountInfo)
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

bool FriendControlView::UpdateListView() {
	//벡터 사이즈 구하기
	int size = reqFriendList.size();
	//리스트 사이즈 구하기
	int listMaxSize = friendRequestBox->GetCount();

	// 1번 전체 초기화 후 벡터 값 다시 로드
	// 2번 부분 초기화 후 변경 된 값만 다시 로드
	// 1번의 경우가 구현이 편하여 1번으로 일단 구현
	//변경된 값이 없을 경우 바로 리턴
	if (size == listMaxSize) {
		WarningMessage("[FRIEND_ADD_VIEW] : Nothings change");
		return true;
		
	}
	////친구 요청이 늘어 났을 경우
	//else if (size > listMaxSize) {
	//}

	//리스트 항목 초기화
	for (int cnt = 0; cnt < listMaxSize; cnt++) {
		friendRequestBox->DeleteString(cnt);
	}

	//리스트 항목 벡터 값 복사
	for (int cnt = 0; cnt < size; cnt++) {
		friendRequestBox->AddString(reqFriendList[cnt]->GetString());
	}

	return true;
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
	ON_BN_CLICKED(REQUST_REJECT_BTN, &FriendControlView::ReqRejectBtnClicekd)
	ON_BN_CLICKED(FRIEND_ADD_BTN, &FriendControlView::AddFriendBtnClicked)
END_MESSAGE_MAP()

void FriendControlView::ReqAcceptBtnClicked() {
	int currentItem = friendRequestBox->GetCurSel();
	if (currentItem == ERROR) {
		WarningMessage("[CLIENT] : No items selected");
		return;
	}

	wchar_t currentStirng[MAX__STRING_LEN] = { 0, };

	friendRequestBox->GetText(currentItem, currentStirng);

	//통신 변수
	//데이터 헤더	: RESPOND_FRIEND_INFO
	//실 데이터		: AccountInfo + FriendInfo 구성
	
	DataHeaders header;

	//사이즈 문제로 인해 totData를 삭제해야 되나, 일단 진행
	char totData[sizeof(AccountInfo) + sizeof(FriendInfo)] = { 0, };
	AccountInfo* accountInfo;
	FriendInfo* friendInfo;

	//헤더 설정
	header.dataCnt = 2;
	header.dataSize = sizeof(AccountInfo) + sizeof(FriendInfo);
	header.dataType = RESPOND_FRIEND_INFO;

	//데이터 설정
	//내정보 복사
	accountInfo = (AccountInfo*)totData;
	memcpy(accountInfo, myAccountInfo, sizeof(AccountInfo));

	//친구 정보 설정
	//해당 구조체 따로 생성 후 totData에 복사 진행
	//데이터 초기화
	friendInfo = new FriendInfo;
	//friendInfo = (FriendInfo*)(totData + sizeof(AccountInfo));

	friendInfo->request = false;
	friendInfo->friending = true;
	friendInfo->userID.append(ConvertWCtoC(currentStirng));

	//필요 없는 코드 긴 하나 totData를 나중에 사용 할 가능성이 있기에 일단 적음
	memcpy((totData + sizeof(AccountInfo)), friendInfo, sizeof(FriendInfo));

	//사이즈 문제로 두개로 나눠서 전달
	if (!bClient->SendData(&header, accountInfo)) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send update-frined-info");
	}

	//데이터 동기화 함수
	if (bClient->RespondData() != SUCCESS) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send update-friend-info : respond");
		return;
	}

	//데이터 만 전송
	//헤더 생성
	DataHeaders generalHeader;

	generalHeader.dataSize = sizeof(FriendInfo);
	generalHeader.dataCnt = 1;
	generalHeader.dataType = COMMON_DATA;
	
	//데이터 전송
	if (!bClient->SendData(&generalHeader, friendInfo, sizeof(FriendInfo))) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send update-frined-info");
	}

	//데이터 정리
	//친구 추가를 받았으므로, 친구 추가 목록 업데이트
	//친구 추가 목록 업데이트 됨으로 리스트 뷰 또한 변경 되어야한다.
	//배열 업데이트
	reqFriendList.erase(reqFriendList.begin() + currentItem);
	//리스트 목록 업데이트
	UpdateListView();

	//메모리 정리
	delete(friendInfo);
}

void FriendControlView::ReqRejectBtnClicekd() {
	int currentItem = friendRequestBox->GetCurSel();

	//아이템 선택이 안됬을 경우
	if (currentItem == ERROR) {
		WarningMessage("[FRIEND_ADD_VIEW] : Failed to ReqRejectBtnClicked");
		return;
	}

	wchar_t currentStirng[MAX__STRING_LEN] = { 0, };

	friendRequestBox->GetText(currentItem, currentStirng);

	//통신 변수
	//데이터 헤더	: RESPOND_FRIEND_INFO
	//실 데이터		: AccountInfo + FriendInfo 구성

	DataHeaders header;

	//사이즈 문제로 totData를 없애긴 해야되나, 일단 진행
	char totData[sizeof(AccountInfo) + sizeof(FriendInfo)] = { 0, };
	AccountInfo* accountInfo;
	FriendInfo* friendInfo;

	//헤더 설정
	header.dataCnt = 1;
	header.dataSize = sizeof(AccountInfo);
	header.dataType = RESPOND_FRIEND_INFO;

	//데이터 설정
	//내정보 복사
	accountInfo = (AccountInfo*)totData;
	memcpy(accountInfo, myAccountInfo, sizeof(AccountInfo));

	
	friendInfo = new FriendInfo;
	//friendInfo = (FriendInfo*)(totData + sizeof(AccountInfo));

	friendInfo->request = false;
	friendInfo->friending = false;
	friendInfo->userID.append(ConvertWCtoC(currentStirng));

	//필요 없는 코드 긴 하나 totData를 나중에 사용 할 가능성이 있기에 일단 적음
	memcpy((totData + sizeof(AccountInfo)), friendInfo, sizeof(FriendInfo));

	//사이즈 문제로 두개로 나눠서 전달
	if (!bClient->SendData(&header, accountInfo)) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send update-frined-info");
	}

	//데이터 동기화 함수
	if (bClient->RespondData() != SUCCESS) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send update-friend-info : respond");
		return;
	}

	//데이터 만 전송
	//헤더 생성
	DataHeaders generalHeader;

	generalHeader.dataSize = sizeof(FriendInfo);
	generalHeader.dataCnt = 1;
	generalHeader.dataType = COMMON_DATA;

	//데이터 전송
	if (!bClient->SendData(&generalHeader, friendInfo, sizeof(FriendInfo))) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send update-frined-info");
	}

	//데이터 정리
	//친구 추가를 받았으므로, 친구 추가 목록 업데이트
	//친구 추가 목록 업데이트 됨으로 리스트 뷰 또한 변경 되어야한다.
	//배열 업데이트
	reqFriendList.erase(reqFriendList.begin() + currentItem);
	//리스트 목록 업데이트
	UpdateListView();

	//메모리 정리
	delete(friendInfo);
}

void FriendControlView::AddFriendBtnClicked() {
	CString friendAddID;

	//친구 추가 될 ID 가져오기
	friendAddEdit->GetWindowTextW(friendAddID);

	//ID 오류 확인
	if (friendAddID.GetLength() <= 0) {
		WarningMessage("[FRIEND_ADD_VIEW] : Failed to get friend-add-edit");
		return;
	}

	//통신 변수
	//데이터 헤더	: RESPOND_FRIEND_INFO
	//실 데이터		: AccountInfo + FriendInfo 구성
	DataHeaders header;

	char totData[sizeof(AccountInfo) + sizeof(FriendInfo)] = { 0, };
	AccountInfo* accountInfo;
	FriendInfo* friendInfo;
	
	//totData 가 BUFSIZE를 넘겨서 헤더 조정
	/*header.dataCnt = 2;
	header.dataSize = sizeof(AccountInfo) + sizeof(FriendInfo);
	header.dataType = ADD_FRIEND;*/

	header.dataCnt = 1;
	header.dataSize = sizeof(AccountInfo);
	header.dataType = ADD_FRIEND;

	//데이터 설정
	//내정보 복사
	accountInfo = (AccountInfo*)totData;
	memcpy(accountInfo, this->myAccountInfo, sizeof(AccountInfo));

	//친구 정보 설정
	friendInfo = (FriendInfo*)(totData + sizeof(AccountInfo));
	//친구 이름 복사3 설정으로 인해 비활성
	/*friendInfo->friending = false;
	friendInfo->request = true;*/

	//친구 이름 복사 -> 잘못된 값이 들어감
	//friendInfo->userID.append(ConvertWCtoC(friendAddID.GetString()));

	//친구 이름 복사2 : 동작은 되나 코드 편리성이 안좋음
	/*char* tmpID = ConvertWCtoC(friendAddID.GetString());
	int len = strlen(tmpID);

	memcpy((char*)friendInfo->userID.c_str(), tmpID, len);*/

	//친구 이름 복사3
	//임시 데이터 생성 : 위에 선언 하면 햇갈려 아래에 일부로 변수 선언함
	FriendInfo* tmpFriendInfo;
	tmpFriendInfo = new FriendInfo;
	
	//친구 정보 설정
	tmpFriendInfo->userID.append(ConvertWCtoC(friendAddID.GetString()));
	tmpFriendInfo->friending = true;
	tmpFriendInfo->request = false;

	//복사
	memcpy(friendInfo, tmpFriendInfo, sizeof(FriendInfo));

	//데이터 보내기
	//SendData 함수 size 값 추가 
	//if (!bClient->SendData(&header, totData)) {
	//사이즈 문제로 두개로 나눠서 전달
	// SendData에서 totData 값을 그대로 쓸꺼면 size 값을 같이 전달해 줘야된다.
	//if (!bClient->SendData(&header, accountInfo)) {
	if (!bClient->SendData(&header, accountInfo)) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send add-friend-info");
	}
	//데이터 동기화를 위해 Respond Data 확인
	if (bClient->RespondData() != SUCCESS) {
		 WarningMessage("[FRIND_ADD_VIEW] : Failed to send add-friend-info : respond");
		return;
	}

	//데이터 만 전송
	//헤더 생성
	DataHeaders generalHeader;

	generalHeader.dataSize = sizeof(FriendInfo);
	generalHeader.dataCnt = 1;
	generalHeader.dataType = COMMON_DATA;

	if (!bClient->SendData(&generalHeader, friendInfo, sizeof(FriendInfo))) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send add-friend-info");
	}

	MessageBoxW(L"친구 요청 보냈습니다.", L"친구 요청");

	//메모리 정리
	delete(tmpFriendInfo);
}
