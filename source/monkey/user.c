#include "monkey/user.h"
#ifdef _WIN32
#include <Windows.h>
#include <security.h>
#else
#include "monkey/string.h"

#include <pwd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#endif

char* CurrentUser(void) {
#ifdef _WIN32
	ULONG size = 0;
	GetUserNameEx(NameDisplay, NULL, &size);
	char* result = malloc(size);
	GetUserNameEx(NameDisplay, result, &size);
	return result;
#else
#define DEFAULT_MAX_NAME_LENGTH 16384
	uid_t uid = getuid();
	long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufsize == -1) {
		bufsize = DEFAULT_MAX_NAME_LENGTH;
	}
	char* buf = malloc((size_t)bufsize);
	struct passwd pw;
	struct passwd* result;
	getpwuid_r(uid, &pw, buf, (size_t)bufsize, &result);
	if (result == NULL) {
		free(buf);
		return NULL;
	}
	char* name = MonkeyStrdup(pw.pw_name);
	free(buf);
	return name;
#endif
}
