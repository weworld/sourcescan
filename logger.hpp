#pragma once

#include <iostream>
#include <string>

typedef std::string String;

using std::endl;

//typedef enum LogLevel {
//	FATAL_LOG = 0, ERROR_LOG, INFO_LOG, DEBUG_LOG
//} LogLevel;

typedef std::ostream& (*ostream_manipulator)(std::ostream&);

struct LogLocation {
	LogLocation(const String& file, int line)
		: file(file), line(line){
	}
	String file;
	int line;
};

class Logger {
public:
	static Logger& getErrorLogger() {
		static Logger log(kLogError, std::cout, CC_RED "E> ");
		return log;
	}

	static Logger& getInfoLogger() {
		static Logger log(kLogInfo, std::cout, "I> ");
		return log;
	}

	static Logger& getVerboseLogger() {
		static Logger log(kLogVerbose, std::cout, CC_BLU "V> ");
		return log;
	}

	static Logger& getDebugLogger() {
		static Logger log(kLogDebug, std::cout, CC_YEL "D> ");
		return log;
	}

	static Logger& getDebug2Logger() {
		static Logger log(kLogDebug, std::cerr, "");
		return log;
	}

protected:
	Logger(LogLevel logLevel, std::ostream &outStream, const String& prefix)
		: logLevel_(logLevel), outStream_(outStream), printedPrefix_(false), prefix_(prefix) {
	}
private:
	template <typename T>
	friend Logger& operator<<(Logger &logger, const T &data);
	friend Logger& operator<<(Logger &logger, ostream_manipulator manip);

	LogLevel logLevel_;
	std::ostream &outStream_;
	bool printedPrefix_;
	const String prefix_;
};

//LogLevel gLogLevel = ERROR_LOG;

class NoLogger {
public:
	static NoLogger& getLogger() {
		static NoLogger nolog;
		return nolog;
	}
	
	template <typename T>
	NoLogger& operator<<(const T &data) {
		return *this;
	}
};

template <typename T>
inline Logger& operator<<(Logger &logger, const T &data) {
	if (!logger.printedPrefix_) {
		logger.printedPrefix_ = true;
		logger.outStream_ << logger.prefix_;
	}
	if (logger.logLevel_ <= g_logLevel) {
		logger.outStream_ << data;
	}
	return logger;
}

template <>
inline Logger& operator<< <LogLocation>(Logger &logger, const LogLocation &data) {
	if (!logger.printedPrefix_) {
		logger.printedPrefix_ = true;
		logger.outStream_ << logger.prefix_;
	}
	logger.outStream_ << data.file << ":" << data.line << ": ";
	return logger;
}

//extern std::ostream& std::endl(std::ostream&);

Logger& operator<<(Logger &logger, ostream_manipulator manip)
{
	//operator<< <ostream_manipulator> (logger.outStream_, manip);
	if (logger.logLevel_ <= g_logLevel) {
		logger.printedPrefix_ = false;
		manip(logger.outStream_);
		if (manip == &std::endl<char, std::char_traits<char> >) {
			logger.outStream_ << CC_RES;
		}
	}
	return logger;
}

template <typename T>
inline NoLogger& operator<<(NoLogger &logger, const T &/*data*/) {
	return logger;
}

NoLogger& operator<<(NoLogger &logger, ostream_manipulator /*manip*/)
{
	return logger;
}

#define NO_LOG NoLogger::getLog()
#define LOC LogLocation(__FILE__, __LINE__)
#define LOGE Logger::getErrorLogger() << LOC
#define LOGI Logger::getInfoLogger() << LOC
#define LOGD Logger::getDebugLogger() << LOC
#define LOGD2 Logger::getDebug2Logger()


// isTokStr

enum { LL_ERROR = 0, LL_INFO, LL_VERBOSE, LL_DEBUG, LL_DEBUG_RAW };
const uint8_t logLevel = LL_DEBUG_RAW;
const bool showLevelFlag = true;
const char* levelFlag[] = { "E: ", "I: ", "V: ", "D: ", "" };
#define LOGE(fmt, ...) LOG(LL_ERROR, fmt "\n", ##__VA_ARGS__)
#define LOGI(fmt, ...) LOG(LL_INFO, fmt "\n", ##__VA_ARGS__)
#define LOGV(fmt, ...) LOG(LL_VERBOSE, fmt "\n", ##__VA_ARGS__)
#define LOGD(fmt, ...) LOG(LL_DEBUG, fmt "\n", ##__VA_ARGS__)
#define DDD(fmt, ...) LOG(LL_DEBUG_RAW, fmt, ##__VA_ARGS__)
#define LOG(level, fmt, ...) \
    do { \
        if (level >= 0 && level <= LL_DEBUG_RAW) { \
            if (level <= logLevel) \
                fprintf(stderr, "%s" fmt, (showLevelFlag? levelFlag[level] : ""), ##__VA_ARGS__); \
        } else \
            fprintf(stderr, "ERROR: Log level error. (%d). %s:%d\n", level, __FILE__, __LINE__); \
    } while (0)

enum { V_LOW = 0, V_MEDIUM, V_HIGH, V_CRITICAL };

const bool showReport = true;
const bool showReportPrefix = false;
const char* reportPrefix = "VULN: ";
const char* vulnLevelStr[] = { "LOW", "MDM", "HGH", "CRT" };

#define PRINT_VULN(level, fmt, ...) \
    do { \
        if (showReport) { \
            if (level >= V_LOW && level <= V_CRITICAL) { \
                fprintf(stderr, "%s[%s] " fmt, (showReportPrefix? reportPrefix : ""), vulnLevelStr[level], ##__VA_ARGS__); \
            } else {\
                LOGD("Vulnerable level error. (%d). \t%s:%d", level, __FILE__, __LINE__); \
            } \
        } \
    } while (0)

// sUnkownTok
