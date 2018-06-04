#ifndef LOGGING_H
#define LOGGING_H

#include "format.h"
#include <string>
#include <cstring>

namespace logging {

enum Level {
	Verbose = 0,
	Debug = 1,
	Info = 2,
	Warning = 3,
	Error = 4,
	Off = 100
};

#ifdef SIMPLE_LOG

inline void log2(const char *fn, int line, const Level level, const std::string &text) {
	if(level >= Info)
		printf("[%s:%d] %s\n", fn, line, text.c_str());
}


#else

void log(const std::string &text);
void log(const Level level, const std::string &text);
void log2(const char *fn, int line, const Level level, const std::string &text);

#endif

template <class... A>
void log(const std::string &fmt, const A& ... args) {
	log(utils::format(fmt, args...));
};

template <class... A>
void log(Level level, const std::string &fmt, const A& ... args) {
	log(level, utils::format(fmt, args...));
};

template <class... A>
void log2(const char *fn, int line, Level level, const std::string &fmt, const A& ... args) {
	log2(fn, line, level, utils::format(fmt, args...));
};

inline void LogVL(int line, const char* fileName, const char* text, va_list vl)
{
	char temp[2048];
    vsnprintf(temp, sizeof(temp), text, vl);
	log2(fileName, line, Debug, temp);
}


void setLevel(Level level);
void setOutputFile(const std::string &fileName);
//void setLogSpace(const std::string &sourceFile, const std::string &function, const std::string &spaceName);

void useLogSpace(const std::string &spaceName, bool on = true);

inline const char *xbasename(const char *x) {
	const char *slash = x;
	while(*x) {
		if(*x++ == '/') slash = x;
	}
	return slash;
}

#define LOGV(...) logging::log2(logging::xbasename(__FILE__), __LINE__, logging::Verbose, __VA_ARGS__)
#define LOGD(...) logging::log2(logging::xbasename(__FILE__), __LINE__, logging::Debug, __VA_ARGS__)
#define LOGI(...) logging::log2(logging::xbasename(__FILE__), __LINE__, logging::Info, __VA_ARGS__)
#define LOGW(...) logging::log2(logging::xbasename(__FILE__), __LINE__, logging::Warning, __VA_ARGS__)
#define LOGE(...) logging::log2(logging::xbasename(__FILE__), __LINE__, logging::Error, __VA_ARGS__)

class LogSpace {
public:
	LogSpace(const std::string &sourceFile, const std::string &function, const std::string &spaceName, bool on = true) {
		//spaces[sourceFile] = std::make_pair(spaceName, on);
		//LOGD("LogSpace %s for %s '%s'", spaceName, sourceFile, function);
	}
	//static std::unordered_map<std::string, std::pair<std::string, bool>> spaces;

};

#define LOGSPACE(x) static logging::LogSpace lsp(__FILE__, "", x);

}

#endif // LOGGING_H
