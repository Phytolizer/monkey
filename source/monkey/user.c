#include "monkey/user.h"

#include <assert.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#define SECURITY_WIN32
#include <security.h>
#else
#include "monkey/string.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

char* CurrentUser(void) {
#ifdef _WIN32
	ULONG size = 0;
	GetUserNameEx(NameDisplay, NULL, &size);
	char* result = malloc(size);
	assert(result != NULL && "malloc failure");
	GetUserNameEx(NameDisplay, result, &size);
	return result;
#else
#define DEFAULT_MAX_NAME_LENGTH 16384
	uid_t uid = getuid();
	long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	assert(bufsize != -1 && "weird OS doesn't know max username length");
	char* buf = malloc((size_t)bufsize);
	struct passwd pw;
	struct passwd* result;
	getpwuid_r(uid, &pw, buf, (size_t)bufsize, &result);
	assert(result != NULL && "getpwuid_r failure");
	char* name = MonkeyStrdup(pw.pw_name);
	free(buf);
	return name;
#endif
}
