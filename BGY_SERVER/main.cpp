#include "headers/IOCPServer.h"
#include "headers/BGYSqlite.h"

#include <iostream>
#include <time.h>
#include <conio.h>

#define DEBUGGING		1

#if DEBUGGING
#define TICK_TIME		1000
#define ESC				27
#endif

#define PORT_NUM		8986
const char* DB_PATH = "test.db";


int main(int argc, char* argv[]) {

	BGYSqlite sql(DB_PATH);
	IOCPServer* server = new IOCPServer(PORT_NUM, &sql);
	server->ServerOpen();

	//function test
	/*Sleep(3000);
	if (server->ServerStop()) {
		SetColorMessage("[MAIN] : Success to stop", ccolor::LIGHTGREEN, ccolor::BLACK);
	}
	if (server->ServerRestart()) {
		SetColorMessage("[MAIN] : Success to restart", ccolor::LIGHTGREEN, ccolor::BLACK);
	}*/
	
#if DEBUGGING
	unsigned int cTime = 0, lTime = 0, tick = TICK_TIME;
	while (1) {
		cTime = clock();

		if (cTime - lTime >= tick) {
			lTime = cTime;

			if (_kbhit()) {
				if (_getch() == ESC) {
					break;
				}
			}
		}
	}
#endif

	return 0;
}