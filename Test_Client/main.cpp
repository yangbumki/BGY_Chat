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

	ClientIOData* cd = nullptr;
	DataHeaders* dh = nullptr;

	auto result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result < 0) exit(1);

	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(PORT_NUM);

	sock = new SOCKET;
	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	result = connect(*sock, (sockaddr*)&addr, sizeof(SOCKADDR_IN));

	
	
	while (1) {
		
		if (_kbhit()) {
			cd = new ClientIOData;


			dh = new DataHeaders;
			dh->dataType = 1;
			dh->dataSize = 2;
			dh->dataCnt = 3;

			cd->wsaBuf.buf = (CHAR*)dh;
			cd->wsaBuf.len = sizeof(DataHeaders);

			memset(cd, 0, sizeof(ClientIOData));
			cd->wsaBuf.buf = (CHAR*)dh;
			cd->wsaBuf.len = sizeof(DataHeaders);

			printf("Clicked \n");
			result = WSASend(*sock, &cd->wsaBuf, 1, &cd->sendBytes, cd->flag, &cd->overlapped, NULL);
			printf("send Byte 1 : %d \n", cd->sendBytes);
			result = send(*sock, (char*)dh, sizeof(DataHeaders), 0);
			printf("send bytes : %d \n", result);

			if (_getch() == 27)	break;
		}
	}
		
	return 0;
}