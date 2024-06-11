#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <conio.h>
#include <vector>

#define PORT_NUM		8986
#define BUFSIZE			1024

typedef struct CLIENT_MODEL {
	SOCKET sock;
	sockaddr_in addr;
}ClientModel;

typedef struct CLIENT_IO_DATA {
	WSABUF wsaBuf;
	DWORD recvBytes;
	DWORD sendBytes;
	WSAOVERLAPPED overlap;
	char buffer[BUFSIZE];
}ClientIOData;

DWORD WINAPI WorkerFunction(void* args) {
	HANDLE iocpHandle = (HANDLE)args;
	DWORD transBytes = 0;
	ULONG_PTR ptr;
	ClientModel* cm = new CLIENT_MODEL;
	memset(cm, 0, sizeof(ClientModel));
	ClientIOData* cd = new ClientIOData;
	memset(cd, 0, sizeof(ClientIOData));

	while (true) {
		if (GetQueuedCompletionStatus(iocpHandle, &transBytes, &ptr, (LPOVERLAPPED*)cd, INFINITE)) {
			//printf("Success to worker-function\n");

			cm = (ClientModel*)ptr;

			char* tempAddr = new char[BUFSIZE];
			inet_ntop(AF_INET, &cm->addr.sin_addr, tempAddr, BUFSIZE);
			printf("[WORKER] Client-addresss : %s \n", tempAddr);
			delete(tempAddr);

			DWORD flag = 0;
			cd->wsaBuf.buf = cd->buffer;
			cd->wsaBuf.len = sizeof(cd->buffer);
			cd->recvBytes = 0;

			/*auto result = recv(cm->sock, cd->buffer, strlen(cd->buffer), 0);
			if (result != 0) {
				printf("[WORKER] : %s \n", cd->buffer);
			}*/

			auto result = WSARecv(cm->sock, &cd->wsaBuf, 1, &cd->recvBytes, &flag, &cd->overlap, NULL);
			printf("Recv-data : %s \n", cd->wsaBuf.buf);
			if(result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
				printf("Failed to recv \n");
				printf("[Worker] ERROR : %d \n", WSAGetLastError());
			}
		}
	}

	return 0;
}

DWORD WINAPI IOCPFunction(void* args) {
	SOCKET* sock = (SOCKET*)args;
	HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (iocpHandle == NULL) {
		printf("CreateIOCP \n");
		return -1;
	}

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	std::vector<HANDLE> threads;
	HANDLE tempThreadHandle = NULL;

	for (int i = 0; i < si.dwNumberOfProcessors; i++) {
		tempThreadHandle = CreateThread(NULL, 0, WorkerFunction, iocpHandle, NULL, 0);
		if (tempThreadHandle == INVALID_HANDLE_VALUE) {
			printf("Failed to create threads");
			return -1;
		}

		threads.push_back(tempThreadHandle);
		tempThreadHandle = NULL;
	}

	printf("[MAIN] : Create Threads \n");


	ClientModel cm;
	ClientIOData cd;
	int len = sizeof(SOCKADDR_IN);

	memset(&cm, 0, sizeof(ClientModel));
	memset(&cd, 0, sizeof(ClientIOData));

	cd.wsaBuf.buf = cd.buffer;
	cd.wsaBuf.len = sizeof(cd.buffer);

	while (1) {
		cm.sock = accept(*sock, (sockaddr*)&cm.addr, &len);
		if (cm.sock == NULL) {
			printf("Somethings wrong\n");
			printf("Check your code - accept\n");
			return -1;
		}

		char* tempAddr = new char[BUFSIZE];
		inet_ntop(AF_INET, &cm.addr.sin_addr, tempAddr, BUFSIZE);
		printf("Client-addresss : %s \n", tempAddr);
		delete(tempAddr);

		iocpHandle = CreateIoCompletionPort((HANDLE)cm.sock, iocpHandle, (ULONG_PTR)&cm, 0);
		printf("Success to Create new iocp\n");

		DWORD flag = 0;

		cd.wsaBuf.len = BUFSIZE;
		cd.recvBytes = 0;
		ZeroMemory(&cd.overlap, sizeof(WSAOVERLAPPED));
		flag = 0;

		auto result = WSARecv(cm.sock, &cd.wsaBuf, 1, &cd.recvBytes, &flag, &cd.overlap, NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			printf("Failed to recv \n");
			printf("Error : %d\n", WSAGetLastError());
		}
	}
}

int main() {
	WSADATA wsaData;

	auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result < 0) {
		printf("WSAStartup \n");
		return -1;
	}

	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		printf("SOCKET \n");
		return -1;
	}

	SOCKADDR_IN sockAddr;
	inet_pton(AF_INET, "127.0.0.1", &sockAddr.sin_addr);
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(PORT_NUM);

	result = bind(sock, (sockaddr*)&sockAddr, sizeof(SOCKADDR_IN));
	if (result < 0) {
		printf("bind \n");
		return -1;
	}

	result = listen(sock, 0);
	if (result < 0) {
		printf("Listen \n");
		return -1;
	}

	HANDLE openThread = CreateThread(NULL, 0, IOCPFunction, &sock, NULL, NULL);

	while (1) {
		Sleep(1000);
		if (_kbhit()) break;
	}

	
	return 0;
}