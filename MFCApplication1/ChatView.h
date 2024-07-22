#pragma once
#include "afxdialogex.h"


// ChatView 대화 상자

class ChatView : public CDialogEx
{
	DECLARE_DYNAMIC(ChatView)

private:
	// 채팅 창 정보 변수
	CString targetName;

public:
	ChatView(wchar_t* targetName, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~ChatView();

	LPCWSTR GetChatTargetInfo();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_VIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
};
