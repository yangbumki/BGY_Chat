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
	friendInfo = (FriendInfo*)(totData + sizeof(AccountInfo));

	friendInfo->request = false;
	friendInfo->friending = true;
	friendInfo->userID.append(ConvertWCtoC(currentStirng));


	//사이즈 문제로 두개로 나눠서 전달
	if (!bClient->SendData(&header, accountInfo)) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send update-frined-info");
	}
	//데이터 만 전송 : 헤더 부분 nullptr
	if (!bClient->GeneralSendData(nullptr, friendInfo)) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send update-frined-info");
	} 
}

void FriendControlView::ReqRejectBtnClicekd() {
	int currentItem = friendRequestBox->GetCurSel();

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
	header.dataCnt = 2;
	header.dataSize = sizeof(AccountInfo) + sizeof(FriendInfo);
	header.dataType = RESPOND_FRIEND_INFO;

	//데이터 설정
	//내정보 복사
	accountInfo = (AccountInfo*)totData;
	memcpy(accountInfo, myAccountInfo, sizeof(AccountInfo));
	//친구 정보 설정
	friendInfo = (FriendInfo*)(totData + sizeof(AccountInfo));

	friendInfo->request = false;
	friendInfo->friending = false;
	friendInfo->userID.append(ConvertWCtoC(currentStirng));

	//데이터 보내기
	//사이즈 문제로 두개로 나눠서 전달
	if (!bClient->SendData(&header, accountInfo)) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to reject friend-info");
	}
	//데이터 만 전송 : 헤더 부분 nullptr
	if (!bClient->GeneralSendData(nullptr, friendInfo)) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to reject friend-info");
	}
}

void FriendControlView::AddFriendBtnClicked() {
	CString friendAddID;

	//친구 추가 될 ID 가져오기
	friendAddEdit->GetWindowTextW(friendAddID);

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
	tmpFriendInfo->friending = false;
	tmpFriendInfo->request = true;

	//복사
	memcpy(friendInfo, tmpFriendInfo, sizeof(FriendInfo));

	//데이터 보내기
	//SendData 함수 size 값 추가 
	//if (!bClient->SendData(&header, totData)) {
	//사이즈 문제로 두개로 나눠서 전달
	// SendData에서 totData 값을 그대로 쓸꺼면 size 값을 같이 전달해 줘야된다.
	//if (!bClient->SendData(&header, accountInfo)) {
	if (!bClient->SendData(&header, accountInfo, sizeof(AccountInfo))) {
		WarningMessage("[FRIND_ADD_VIEW] : Failed to send add-friend-info");
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

	delete(tmpFriendInfo);
}
