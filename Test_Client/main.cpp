#include "../Common/common.h"

#include <iostream>
#include <conio.h>
#include <vector>

#define PORT_NUM		8986
#define BUFSIZE			1024

int main() {
	WSADATA wsaData;
	SOCKET* sock;
	SOCKADDR_IN addr;

	DataHeaders* dh = nullptr;

	auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result < 0) exit(1);

	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(PORT_NUM);

	sock = new SOCKET;
	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	result = connect(*sock, (sockaddr*)&addr, sizeof(SOCKADDR_IN));

	ClientModel* cm = new ClientModel;
	memset(cm, 0, sizeof(ClientModel));
	
	int i = 0;

	while (1) {
		
		if (_kbhit()) {
			memset(cm, 0, sizeof(ClientModel));

			dh = (DataHeaders*)cm->clientData.data;
			dh->dataCnt = i;
			dh->dataSize = i + 1;
			dh->dataType = i + 2;

			i++;

			cm->clientData.wsaBuf.buf = cm->clientData.data;
			cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

			printf("Clicked \n");
			result = WSASend(*sock, &cm->clientData.wsaBuf, 1, &cm->clientData.sendBytes, cm->clientData.flag, &cm->clientData.overlapped, NULL);
			printf("send Byte 1 : %d \n", cm->clientData.sendBytes);

			if (_getch() == 27)	break;
		}
	}
		
	return 0;
}