#include <Windows.h>
#include <winsqlite/winsqlite3.h>

#pragma comment(lib, "winsqlite3.lib")

#include <iostream>

int callback(void* data, int argc, char** argv, char** azColName) {
	int i;
	std::cout << (const char*)data << ":" << std::endl;

	for (i = 0; i < argc; i++) {
		std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
	}
	printf("\n");

	return 0;

}

int main() {
	sqlite3* db = nullptr;
	sqlite3_stmt* sqlStat;

	auto result = sqlite3_open("test.sqlite", &db);
	if (result != SQLITE_OK) {
		printf("Open\n");
		return -1;
	}

	char* errMsg = NULL;
	sqlite3_exec(db, "select * from test", callback, (void*)"Called", &errMsg);

	result = sqlite3_prepare(db, "select two from test order by two", -1, &sqlStat, 0);
	if (result != SQLITE_OK) {
		printf("Prepare\n");
		return -1;
	}


	int temp = 0;
	while (1) {
		temp = sqlite3_step(sqlStat);
		if (temp == SQLITE_ROW) {
			printf("%d \n", temp);
		}
		else break;
	}

	sqlite3_finalize(sqlStat);
	sqlite3_close(db);

	return 0;
}