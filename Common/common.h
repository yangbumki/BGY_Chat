#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <vector>
#include <string>
#include <thread>

//전처리
#define ERROR						-1

#define BUFSIZE						1024

#define MAX_ACCOUNT_LEN				128

#define ERROR_TEXT_COLOR			4
#define ERROR_BACKGROUND_COLOR		0
#define WARNING_TEXT_COLOR			14
#define WARNING_BACKGROUND_COLOR	0

//color상수 지정
//	#define BLACK 0 
//	#define BLUE 1
//	#define GREEN 2
//	#define CYAN 3
//	#define RED 4 
//	#define MAGENTA 5 
//	#define BROWN 6 
//	#define LIGHTGRAY 7 
//	#define DARKGRAY 8
//	#define LIGHTBLUE 9 
//	#define LIGHTGREEN 10 
//	#define LIGHTCYAN 11
//	#define LIGHTRED 12
//	#define LIGHTMAGENTA 13
//	#define YELLOW 14
//	#define WHITE 15 

typedef enum CMD_COLOR {
	BLACK=0,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	LIGHTGRAY,
	DARKGRAY,
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	YELLOW,
	WHITE,
}ccolor;

typedef enum DATA_TYPE {
	CREATE_ACCOUNT = 0,
	LOGIN_ACCOUNT,
	RESPOND,
}DataType;

typedef enum RESPOND_DATA_TYPE {
	SUCCESS = 0,
	FAIL
}RespondDataType;

//공통구조체
typedef struct DB_ACCOUNT_INFO {
	wchar_t id[MAX_ACCOUNT_LEN];
	wchar_t passwd[MAX_ACCOUNT_LEN];
	wchar_t  name[MAX_ACCOUNT_LEN];
	wchar_t birth[MAX_ACCOUNT_LEN];
}AccountInfo;

typedef struct DATA_HEADERS {
	BYTE dataType;
	unsigned char dataSize;
	unsigned char dataCnt;
}DataHeaders;

typedef struct CLIENT_IO_DATA {
	WSABUF wsaBuf;
	char data[BUFSIZE];
	OVERLAPPED overlapped;
	DWORD recvBytes;
	DWORD sendBytes;
	DWORD flag;
}ClientIOData;

typedef struct CLEINT_MODEL {
	SOCKET socket;
	SOCKADDR_IN addr;
	int addrLen;
	CLIENT_IO_DATA clientData;
}ClientModel;

//공통함수
static void SetColorMessage(const char* msg, int textColor, int backgroundColor) {
	int color = textColor + backgroundColor * 16;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);

	std::cout << msg << std::endl;

	color = ccolor::WHITE + ccolor::BLACK * 16;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

static void ErrorMessage(const char* msg) {
	SetColorMessage("---ERROR---", ERROR_TEXT_COLOR, ERROR_BACKGROUND_COLOR);
	SetColorMessage(msg, ERROR_TEXT_COLOR, ERROR_BACKGROUND_COLOR);
	SetColorMessage("-----------", ERROR_TEXT_COLOR, ERROR_BACKGROUND_COLOR);

	exit(1);
}

static void WarningMessage(const char* msg) {
	SetColorMessage("---WARNING---", WARNING_TEXT_COLOR, WARNING_BACKGROUND_COLOR);
	SetColorMessage(msg, WARNING_TEXT_COLOR, WARNING_BACKGROUND_COLOR);
	SetColorMessage("-----------", WARNING_TEXT_COLOR, WARNING_BACKGROUND_COLOR);
}

static wchar_t* ConvertCtoWC(const char* str) {
	wchar_t* wStr;

	int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);
	wStr = new wchar_t(strSize);
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, wStr, strSize);

	return wStr;
}

static char* ConvertWCtoC(const wchar_t* wstr) {
	char* str;

	int strSize = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, NULL, NULL, NULL);
	str = new char(strSize);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, strSize, 0, 0);

	return str;
}

static std::string AddString(int num, ...) {
	va_list ap;
	std::string addString;

	char addStr[BUFSIZE] = "";

	va_start(ap, num);
	for (int i = 0; i<num; i++) {
		strcat_s(addStr,sizeof(addStr),va_arg(ap, char*));
	}
	va_end(ap);

	addString.append(addStr);

	return addString;
}


