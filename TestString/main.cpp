#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

#define MAX_ADD_STRING_LEN		128

static const char* CONVERT_STR = "%s";

static std::string AddString(std::string args, ...) {
	va_list ap;

	va_start(ap, args);

	unsigned char pos[MAX_ADD_STRING_LEN] = { 0, };
	unsigned int cnt = 0;
	unsigned int padding = strlen(CONVERT_STR);

	do {
		if (cnt == 0)
			pos[cnt] = args.find(CONVERT_STR);
		else
			pos[cnt] = args.find(CONVERT_STR, pos[cnt-1] + padding);

		if (pos[cnt++] + padding >= args.length()-1) break;
	} while (pos[cnt] != -1);

	for (int i = 0; i < cnt; i++) {
		auto tmpStr = va_arg(ap, char*);
		args.replace(pos[i], strlen(CONVERT_STR), "");
		args.insert(pos[i], tmpStr);
	}

	return args;
}

int main() {
	std::string str = "insert into users values(%s, %s, %s, %s)";

	str = AddString(str, "'hello'", "'tester'", "'heoo'", "'1999.99.99'");

	std::cout << str << std::endl;
	return 0;
}