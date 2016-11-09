#ifndef IPS_COMMON_H
#define IPS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define CC_RES "\033[0m" // reset
#define CC_RED "\033[1;31m" // red
#define CC_GRE "\033[1;32m" // green
#define CC_YEL "\033[1;33m" // yellow
#define CC_BLU "\033[1;34m" // blue
#define CC_035 "\033[1;35m" // blue
#define CC_036 "\033[1;36m" // blue

typedef enum LogLevel {
	kLogFailed = 0, kLogError, kLogInfo, kLogVerbose, kLogDebug,
} LogLevel;

#define LOG_F(fmt, ...)			LOG(kLogFailed,		CC_RED "F: " fmt "\n" CC_RES, ##__VA_ARGS__)
#define LOG_E(fmt, ...)			LOG(kLogError,		CC_RED "E: " fmt "\n" CC_RES, ##__VA_ARGS__)
#define LOG_I(fmt, ...)			LOG(kLogInfo,		CC_035 "I: " fmt "\n" CC_RES, ##__VA_ARGS__)
#define LOG_V(fmt, ...)			LOG(kLogVerbose,	CC_036 "V: " fmt "\n" CC_RES, ##__VA_ARGS__)
#define LOG_D(fmt, ...)			LOGD_(kLogDebug,	CC_YEL "D: " fmt "\n" CC_RES, ##__VA_ARGS__)
#define LOG_F_SE(fmt, ...)		perror("FailException"); \
								LOG(kLogFailed,		CC_RED "F: %s:%d:" fmt "\n" CC_RES, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_E_SE(fmt, ...)		perror("Exception"); \
								LOG(kLogError,		CC_RED "E: %s:%d:" fmt "\n" CC_RES, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG(level, fmt, ...)						\
	do {											\
		if (g_logLevel >= level)					\
			fprintf(g_logf, fmt, ##__VA_ARGS__);	\
	} while (0)

#ifndef NDEBUG
#define LOGD_(level, fmt, ...)						\
	do {											\
		fprintf(g_logf, fmt, ##__VA_ARGS__);		\
	} while (0)
#else
#define LOGD_(...) ((void)0)
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define ASSERT_PTR(p)	assert(p && "Pointer " #p " can't be null");

#define println()	printf("\n")

// global log vars
extern FILE* g_logf;
extern const char* g_prog;
extern int g_logLevel;

void log_init(const char* prog, FILE* logf, LogLevel logLevel);

#endif

