// SignupDlg.cpp: 구현 파일
//

#include "pch.h"
#include "MFCApplication1.h"
#include "afxdialogex.h"
#include "SignupDlg.h"


// SignupDlg 대화 상자

IMPLEMENT_DYNAMIC(SignupDlg, CDialogEx)

SignupDlg::SignupDlg(BgyClient* pClient, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SINGUP, pParent), client(pClient)
{
	editID = new CEdit;
	editPasswd = new CEdit;
	editName = new CEdit;
	dateTime = new CDateTimeCtrl;

	SYSTEMTIME* st = new SYSTEMTIME;
	memset(st, 0, sizeof(SYSTEMTIME));
	GetSystemTime(st);

	dateMin = new CTime(*st);
	dateMax = new CTime(GetCurrentTime());

	delete(st);
}

SignupDlg::~SignupDlg()
{
	delete(editID);
	delete(editPasswd);
	delete(editName);
	delete(dateTime);
	delete(dateMin);
	delete(dateMax);
}

void SignupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	

	DDX_Control(pDX, EDITBOX_CREATE_NAME, *editName);
	DDX_Control(pDX, DATETIME_BIRTH, *dateTime);
	DDX_Control(pDX, EDITBOX_CREATE_ID, *editID);
	DDX_Control(pDX, EDITBOX_CREATE_PASSWORD, *editPasswd);

	dateTime->SetRange(dateMin, dateMax);
}


BEGIN_MESSAGE_MAP(SignupDlg, CDialogEx)
	ON_BN_CLICKED(BTN_CREATE_ACCOUNT, &SignupDlg::OnBnClickedSignUp)
	ON_BN_CLICKED(BTN_CANCEL, &SignupDlg::OnBnClickedSignUp)
	ON_NOTIFY(DTN_DATETIMECHANGE, DATETIME_BIRTH, &SignupDlg::OnDtnDatetimechangeDateTimeBirth)
END_MESSAGE_MAP()


// SignupDlg 메시지 처리기


void SignupDlg::OnBnClickedSignUp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
	CString tmpStr;
	AccountInfo* ai = new AccountInfo;;
	memset(ai, 0, sizeof(AccountInfo));

	editID->GetWindowTextW(tmpStr);
	//최소 최대 길이 체크 및 특수문자 검출 코드가 들어가야 할 자리
	wcscpy_s(ai->id, tmpStr.GetString());

	editPasswd->GetWindowTextW(tmpStr);
	//최소 최대 길이 체크 및 특수문자 검출 코드가 들어가야 할 자리
	wcscpy_s(ai->passwd, tmpStr.GetString());

	dateTime->GetWindowTextW(tmpStr);
	//최소 최대 길이 체크 및 특수문자 검출 코드가 들어가야 할 자리
	wcscpy_s(ai->birth, tmpStr.GetString());

	editName->GetWindowTextW(tmpStr);
	//최소 최대 길이 체크 및 특수문자 검출 코드가 들어가야 할 자리
	wcscpy_s(ai->name, tmpStr.GetString());

	if (client->GetServerStat() != CONNECT) {
		AfxMessageBox(L"네트워크 연결이 이상합니다.");
		exit(-1);
	}

	DataHeaders* dh = new DataHeaders;
	memset(dh, 0, sizeof(DataHeaders));

	dh->dataSize = sizeof(ai);
	dh->dataType = DataType::CREATE_ACCOUNT;

	client->SendData(dh, ai);

	delete(ai);
	delete(dh);

	CDialogEx::OnOK();
}

void SignupDlg::OnBnClickedCancel() {
	exit(1);
}