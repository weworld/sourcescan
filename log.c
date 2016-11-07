#include "log.h"

#include <stdio.h>

FILE* g_logf = NULL;
const char* g_prog = NULL;
int g_logLevel = kLogInfo;

void log_init(const char* prog, FILE* logf, LogLevel logLevel)
{
	if (logf) {
		g_logf = logf;
	} else {
		// default error ouput: stderr
		g_logf = stderr;
	}
	g_prog = prog;
	g_logLevel = logLevel;
}
