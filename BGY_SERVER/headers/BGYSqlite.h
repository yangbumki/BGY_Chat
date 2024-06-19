#pragma once

#include "../../Common/common.h"

#include <winsqlite/winsqlite3.h>

#pragma comment(lib, "winsqlite3.lib")

class BGYSqlite
{
private:
	sqlite3* db			= nullptr;
	sqlite3_stmt* stmt	= nullptr;

public:
	BGYSqlite(const char* dbName);
	~BGYSqlite();

	bool GetDuplicatedAccount(DB_ACCOUNT_INFO ai);
	bool CreateAccount(DB_ACCOUNT_INFO ai);
};

