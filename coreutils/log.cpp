
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <ctime>
#include <unordered_map>

static std::string logSource = "Grappix";

#ifdef ANDROID

#include <android/log.h>
#define LOG_PUTS(x) ((void)__android_log_print(ANDROID_LOG_INFO, logSource.c_str(), "%s", x))

#else

#ifdef LOG_INCLUDE
#include LOG_INCLUDE
#endif

#ifndef LOG_PUTS // const char *
#define LOG_PUTS(x) (fwrite(x, 1, strlen(x), stdout), putchar(10))
#endif

#endif
namespace logging {

using namespace std;

LogLevel defaultLevel = DEBUG;
static FILE *logFile = nullptr;
unordered_map<string, pair<string, bool>> LogSpace::spaces;

const char *xbasename(const char *x) {
	const char *slash = x;
	while(*x) {
		if(*x++ == '/') slash = x;
	}
	return slash;
}


void log(const std::string &text) {
	log(defaultLevel, text);
}

void log(LogLevel level, const std::string &text) {
	if(level >= defaultLevel) {
		const char *cptr = text.c_str();
		LOG_PUTS(cptr);
	}
	if(logFile) {
		
		//std::time_t t = chrono::system_clock::to_time_t(system_clock::now());
		//const std::string s = put_time(t, "%H:%M:%S - ");
		time_t now  = time(nullptr);
		struct tm *t = localtime(&now);
		string ts = utils::format("%02d:%02d.%02d - ", t->tm_hour, t->tm_min, t->tm_sec);

		fwrite(ts.c_str(), 1, ts.length(), logFile);
		fwrite(text.c_str(), 1, text.length(), logFile);
		char c = text[text.length()-1];
		if(c != '\n' && c != '\r')
			putc('\n', logFile);
		fflush(logFile);
	}
}

void log2(const char *fn, int line, LogLevel level, const std::string &text) {

	const auto &space = LogSpace::spaces[fn];
	if(space.second || space.first == "") {
		char temp[2048];
		sprintf(temp, "[%s:%d] ", fn, line);
		log(level, std::string(temp).append(text));
	}
}

void setLevel(LogLevel level) {
	defaultLevel = level;
}
void setOutputFile(const std::string &fileName) {
	if(logFile)
		fclose(logFile);
	logFile = fopen(fileName.c_str(), "wb");
}

//void setLogSpace(const std::string &sourceFile, const std::string &function, const std::string &spaceName) {
//	LogSpace::spaces[sourceFile] = spaceName;
//}

void useLogSpace(const string &spaceName, bool on) {
	for(auto &s : LogSpace::spaces) {
		if(s.second.first == spaceName) {
			s.second.second = on;
		}
	}
}

}