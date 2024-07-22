// ChatView.cpp: 구현 파일
//

#include "pch.h"
#include "BChat_Application.h"
#include "afxdialogex.h"
#include "ChatView.h"


// ChatView 대화 상자

IMPLEMENT_DYNAMIC(ChatView, CDialogEx)

ChatView::ChatView(wchar_t* _target,CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_VIEW, pParent)
{
	this->targetName.Append(_target);

}

ChatView::~ChatView()
{
}

LPCWSTR ChatView::GetChatTargetInfo() {
	return targetName.GetString();
}

void ChatView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);	
}


BEGIN_MESSAGE_MAP(ChatView, CDialogEx)
END_MESSAGE_MAP()