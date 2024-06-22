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
	AccountInfo* ai = nullptr;

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

			switch (_getch()) {
			case 0x3B:
				printf("---F1---\n\n");
				ZeroMemory(cm, sizeof(ClientModel));

				dh = (DataHeaders*)cm->clientData.data;
				dh->dataCnt = 1;
				dh->dataSize = sizeof(AccountInfo);
				dh->dataType = LOGIN_ACCOUNT;

				ai = (AccountInfo*)(cm->clientData.data + sizeof(DataHeaders));
				wcscpy_s(ai->id, sizeof("tester"), L"tester");
				wcscpy_s(ai->passwd, sizeof("tester"), L"tester");
				wcscpy_s(ai->name, sizeof("tester"), L"tester");
				wcscpy_s(ai->birth, sizeof("0000.00.00"), L"0000.00.00");

				cm->clientData.wsaBuf.buf = cm->clientData.data;
				cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

				printf("---SEND---\n");
				result = WSASend(*sock, &cm->clientData.wsaBuf, 1, &cm->clientData.sendBytes, cm->clientData.flag, &cm->clientData.overlapped, NULL);
				printf("send Byte : %d \n", cm->clientData.sendBytes);
				ZeroMemory(cm, sizeof(ClientModel));

				cm->clientData.wsaBuf.buf = cm->clientData.data;
				cm->clientData.wsaBuf.len = sizeof(cm->clientData.data);

				printf("---RECV---\n");
				result = WSARecv(*sock, &cm->clientData.wsaBuf, 1, &cm->clientData.recvBytes, &cm->clientData.flag, NULL, NULL);
				if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
					std::cerr << "Failed to recv-data" << std::endl;
					goto END;
				}

				dh = (DataHeaders*)cm->clientData.data;
				printf("---HEADER---\n");
				printf("TYPE : %d \n", dh->dataType);
				printf("SIZE : %d \n", dh->dataSize);
				printf("COUNT : %d \n", dh->dataCnt);

				if (dh->dataType == DATA_TYPE::RESPOND) {
					char* respondData = (cm->clientData.data + sizeof(DataHeaders));
					printf("---DATA---\n");
					if (*respondData == RespondDataType::SUCCESS)
						printf("Respond : SUCCESS \n");
					else
						printf("Respond : FAIL	\n");
				}
				break;

			case 0x1B:
				goto END;
				break;

			default:
				break;
			}
		}
	}
END:

	return 0;
}