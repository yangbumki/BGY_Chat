#include <iostream>
#include <conio.h>
#include <vector>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT_NUM		8986
#define BUFSIZE			1024

typedef struct TESTDATA {
	char buffer[BUFSIZE];
	int len;

}TestData;

DWORD WINAPI CopyFunction(void* args) {
	WSADATA wsaData;
	TestData* td = new TestData;
	auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result < 0) exit(1);

	std::vector<SOCKET*> socks;
	SOCKADDR_IN addr;

	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(PORT_NUM);

	SOCKET* sock;
	sock = new SOCKET;

	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	result = connect(*sock, (sockaddr*)&addr, sizeof(SOCKADDR_IN));

	memset(td, 0, sizeof(TestData));
	memcpy(td->buffer, (char*)args, strlen((char*)args));
	td->len = strlen(td->buffer);

	WSABUF buf;
	buf.buf = td->buffer;
	buf.len = td->len;

	DWORD flag = 0;
	OVERLAPPED overlap;
	DWORD sendBytes;

	result = WSASend(*sock, &buf, 1, &sendBytes, flag, &overlap, NULL);

	return 0;
}

int main() {
	WSADATA wsaData;
	auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result < 0) exit(1);

	std::vector<SOCKET*> socks;
	SOCKADDR_IN addr;

	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(PORT_NUM);

	SOCKET* sock;
	sock = new SOCKET;

	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	result = connect(*sock, (sockaddr*)&addr, sizeof(SOCKADDR_IN));

	TestData td;
	memset(&td, 0, sizeof(TestData));
	memcpy(td.buffer, "Test1 \n", strlen("Test1 \n"));
	td.len = strlen(td.buffer);

	WSABUF buf;
	buf.buf = td.buffer;
	buf.len = strlen("Test1 \n");

	DWORD flag = 0;
	OVERLAPPED overlap;
	memset(&overlap, 0, sizeof(OVERLAPPED));
	DWORD sendBytes ;

	/*HANDLE one = CreateThread(NULL, 0, CopyFunction, (void*)"TEST2", 0, NULL);
	WaitForSingleObject(one, INFINITE);
	HANDLE two = CreateThread(NULL, 0, CopyFunction, (void*)"TEST3", 0, NULL);
	WaitForSingleObject(two, INFINITE);
	HANDLE three = CreateThread(NULL, 0, CopyFunction, (void*)"TEST4", 0, NULL);
	WaitForSingleObject(three, INFINITE);*/

	int i = 0;

	while (1) {
		Sleep(1000);

		result = WSASend(*sock, &buf, 1, &sendBytes, flag, &overlap, NULL);
		printf("%d", i++);
		if (_kbhit()) break;
	}
		
	return 0;
}