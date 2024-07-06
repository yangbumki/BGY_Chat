#pragma once

#include "../../Common/common.h"

#include <winsqlite/winsqlite3.h>

#pragma comment(lib, "winsqlite3.lib")

#define STR_SAME		0

class BGYSqlite
{
private:
	sqlite3* db			= nullptr;
	sqlite3_stmt* stmt;

public:
	BGYSqlite(const char* dbName);
	~BGYSqlite();

	bool GetDuplicatedAccount(const DB_ACCOUNT_INFO ai);
	bool CreateAccount(const DB_ACCOUNT_INFO ai);
	bool CheckAccount(const AccountInfo* ai);
	bool GetAccountInfo(AccountInfo*);
	std::vector<FRIENDINFO> GetFriendInfo(const DB_ACCOUNT_INFO ai);
	
	bool UpdateFriendInfo(const AccountInfo* ai, const FRIENDINFO* fi);
};

