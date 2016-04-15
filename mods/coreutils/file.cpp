#include "file.h"

#include <sys/stat.h>

#include "format.h"
#include "log.h"
#ifdef _WIN32
#include <windows.h>
#endif

#include <unistd.h>
#include <iomanip>
#include <dirent.h>
#include <stdlib.h>

#ifdef APPLE
#include <mach-o/dyld.h>
#endif

#include <mutex>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

namespace utils {

using namespace std;

File File::NO_FILE;

File File::appDir ("/usr/share/" APP_NAME_STR);
File File::userDir;
File File::cacheDir;
File File::configDir;
File File::exeDir;
File File::homeDir;

static mutex fm;

File::File() : size(-1), writeFP(nullptr), readFP(nullptr) {}

File::File(const string &name, const Mode mode) : fileName(rstrip(name, '/')), size(-1), writeFP(nullptr), readFP(nullptr) {
	if(mode != NONE)
		open(mode);
}

File::File(const string &parent, const string &name, const Mode mode) : size(-1), writeFP(nullptr), readFP(nullptr) {
	if(parent == "")
		fileName = rstrip(name, '/');
	else
		fileName = rstrip(parent, '/') + "/" + rstrip(name, '/');
	if(mode != NONE)
		open(mode);
}

vector<File> File::listFiles() const {
	DIR *dir;
	struct dirent *ent;
	vector<File> rc;
	if( (dir = opendir(fileName.c_str())) != nullptr) {
		while ((ent = readdir (dir)) != nullptr) {
			char *p = ent->d_name;
			if(p[0] == '.' && (p[1] == 0 || (p[1] == '.' && p[2] == 0)))
				continue;
			rc.emplace_back(fileName + "/" + ent->d_name);
		}
	}
	closedir(dir);
	return rc;
}

vector<uint8_t> File::readAll() {
	vector<uint8_t> data;
	seek(0);
	data.resize(getSize());
	int rc = read(&data[0], data.size());
	if(rc != data.size())
		throw io_exception{};
	return data;
}

void File::open(const Mode mode) {
	if(mode == READ) {
		if(!readFP) {
			readFP = fopen(fileName.c_str(), "rb");
			if(!readFP)
				throw file_not_found_exception(fileName);
		}
	} else if(mode == WRITE) {
		if(!writeFP) {
            //makedirs(fileName);
			writeFP = fopen(fileName.c_str(), "wb");
			if(!writeFP)
				throw io_exception { "Could not open file for writing" };
		}
	} else
		throw io_exception { "Can't open file with no mode" };

}

bool File::isChildOf(const File &f) const {
	string myPath = resolvePath(getName());
	string parentPath = resolvePath(f.getName());
	return (myPath.find(parentPath) == 0);
}

void File::seek(int64_t where) {
	open(READ);
	if(!readFP)
		throw file_not_found_exception(fileName);
	fseek(readFP, where, SEEK_SET);
}
int64_t File::tell() {
	open(READ);
	if(!readFP)
		throw file_not_found_exception(fileName);
	return ftell(readFP);
}



vector<string> File::getLines() {
	vector<string> lines;
	auto data = readAll();
	string source { reinterpret_cast<char*>(&data[0]), (unsigned int)data.size() };
	stringstream ss(source);
	string to;

	while(getline(ss, to)) {
		auto l = to.length();
		while(l > 0 && (to[l-1] == 10 || to[l-1] == 13)) l--;
		lines.push_back(to.substr(0,l));
	}
	return lines;
}

void File::write(const uint8_t *data, const int size) {
	open(WRITE);
	fwrite(data, 1, size, writeFP);
}

void File::write(const string &data) {
	open(WRITE);
	fwrite(data.c_str(), 1, data.length(), writeFP);
}


void File::copyFrom(File &otherFile) {
	open(WRITE);
	const auto data = otherFile.readAll();
	fwrite(&data[0], 1, data.size(), writeFP);
}

void File::copyFrom(const string &other) {
	File f { other };
	copyFrom(f);
	f.close();
}

void File::close() {
	if(writeFP)
		fclose(writeFP);
	else if(readFP)
		fclose(readFP);
	writeFP = readFP = nullptr;
}

bool File::exists() const {
	struct stat ss;
	return (stat(fileName.c_str(), &ss) == 0);
}

bool File::exists(const string &fileName) {
	struct stat ss;
	return (stat(fileName.c_str(), &ss) == 0);
}

string File::makePath(vector<File> files) {
	string path = "";
	string sep = "";
	for(const File& f : files) {
		path = path + sep + f.getName();
		sep = PATH_SEPARATOR_STR;
	}
	return path;

}


File File::findFile(const string &path, const string &name) {
	LOGD("Find '%s'", name);
	if(name == "")
		return NO_FILE;
	auto parts = split(path, PATH_SEPARATOR_STR);
	for(string p : parts) {
		if(p.length() > 0) {
			if(p[p.length()-1] != '/')
				p += "/";
			LOGD("...in path %s", p);
			File f { p + name };
			if(f.exists())
				return f;
		}
	}
	return NO_FILE;
}

const File& File::getHomeDir() {
	if(!homeDir) {
	#ifdef _WIN32
		char path[MAX_PATH];
		string h = getenv("HOMEPATH");
		if(h[0] == '\\') {
			h = string("C:") + h;
			replace_char(h, '\\', '/');
		}
		homeDir = File(h);
	#else
		homeDir = File(getenv("HOME"));
	#endif
	}
	return homeDir;
}

static std::string getHome() { return File::getHomeDir().getName(); }

#ifdef APP_NAME

const File& File::getCacheDir() {
	lock_guard<mutex> lock(fm);
	if(!cacheDir) {
		string home = getHome();
#ifdef _WIN32
		replace_char(home, '\\', '/');
#endif
	auto d = format("%s/.cache/" APP_NAME_STR, home);
		LOGD("CACHE: %s", d);
		if(!exists(d))
			utils::makedirs(d);
		cacheDir = File(d);
	}
	return cacheDir;
}

const File& File::getConfigDir() {
	lock_guard<mutex> lock(fm);
	if(!configDir) {
		std::string home = getHome();
#ifdef _WIN32
		replace_char(home, '\\', '/');
#endif
	auto d = format("%s/.config/" APP_NAME_STR, home);
	LOGD("CACHE: %s", d);
	if(!exists(d))
	utils::makedirs(d);
	configDir = File(d);
	}
	return configDir;
}

const std::string File::getUserDir() {
	std::string home = getHome();
#ifdef _WIN32
	replace_char(home, '\\', '/');
#endif
	auto d = format("%s/" APP_NAME_STR, home);
	if(!exists(d))
	utils::makedirs(d);
	return d + "/";
}

#endif

uint64_t File::getModified() const {
	struct stat ss;
	if(stat(fileName.c_str(), &ss) != 0)
		throw io_exception {"Could not stat file"};
	return (uint64_t)ss.st_mtime;
}

uint64_t File::getModified(const std::string &fileName) {
	struct stat ss;
	if(stat(fileName.c_str(), &ss) != 0)
		throw io_exception {"Could not stat file"};
	return (uint64_t)ss.st_mtime;
}

int64_t File::getSize() const {
	if(size < 0) {
		struct stat ss;
		if(stat(fileName.c_str(), &ss) != 0)
			throw io_exception {"Could not stat file"};
		size = (uint64_t)ss.st_size;
	}
	return size;
}

bool File::isDir() const {
	struct stat ss;
	if(stat(fileName.c_str(), &ss) != 0)
		throw io_exception {"Could not stat file"};
	return S_ISDIR(ss.st_mode);
}

void File::remove() {
	close();
	if(std::remove(fileName.c_str()) != 0)
		throw io_exception {"Could not delete file"};
}

void File::rename(const std::string &newName) {
	if(std::rename(fileName.c_str(), newName.c_str()) != 0)
		throw io_exception {"Could not rename file"};
	fileName = newName;
}

void File::remove(const std::string &fileName) {
	if(std::remove(fileName.c_str()) != 0)
		throw io_exception {"Could not delete file"};
}

std::string File::read() {
	open(READ);
	auto data = readAll();
	return std::string(reinterpret_cast<const char *>(&data[0]), (unsigned long)data.size());
}

void File::copy(const std::string &from, const std::string &to) {
	File f0 { from };
	File f1 { to };
	f1.copyFrom(f0);
}

std::string File::resolvePath(const std::string &fileName) {
	char temp[PATH_MAX];
#ifdef _WIN32
	//if(GetFullPathNameA(fileName.c_str(), PATH_MAX, temp, NULL) > 0)
	if(_fullpath(temp, fileName.c_str(), PATH_MAX)) {
		replace_char(temp, '\\', '/');
		return std::string(temp);
	}
#else
	if(::realpath(fileName.c_str(), temp))
		return std::string(temp);
#endif
	return fileName;
}


File& File::resolve() {
	char temp[PATH_MAX];
#ifdef _WIN32
	if(_fullpath(temp, fileName.c_str(), PATH_MAX)) {
		replace_char(temp, '\\', '/');
		fileName = temp;
	}
#else
	if(::realpath(fileName.c_str(), temp)) {
		fileName = temp;
	}
#endif
	else
		fileName = "";
	return *this;
}

File File::cwd() {
	char temp[PATH_MAX];
	if(::getcwd(temp, sizeof(temp))) {
#ifdef _WIN32
		replace_char(temp, '\\', '/');
#endif
		return File(temp);
	}
	throw io_exception {"Could not get current directory"};
}

File::~File() {
	if(readFP)
		fclose(readFP);
	if(writeFP)
		fclose(writeFP);
}

const File& File::getExeDir() {
	lock_guard<mutex> lock(fm);

	if(!exeDir) {
		static char buf[1024];
	#if defined _WIN32
		GetModuleFileName(nullptr, buf, sizeof(buf)-1);
		replace_char(buf, '\\', '/');
		char *ptr = &buf[strlen(buf)-1];
		while(ptr > buf && *ptr != '/')
			*ptr-- = 0;
		*ptr = 0;
		exeDir = File(buf);
	#elif defined APPLE
		uint32_t size = sizeof(buf);
		if(_NSGetExecutablePath(buf, &size) == 0) {
			exeDir = File(path_directory(buf));
		}
	#elif defined UNIX
		int rc = readlink("/proc/self/exe", buf, sizeof(buf)-1);
		if(rc >= 0) {
			buf[rc] = 0;
			exeDir = File(path_directory(buf));
		}
	#endif
	}
	LOGD("EXEDIR:%s", exeDir.getName());
	exeDir.resolve();
	return exeDir;
}


void File::setAppDir(const std::string &a) {
	lock_guard<mutex> lock(fm);
	LOGD("Setting appdir to %s", a);
	appDir = a;
}

const File& File::getAppDir() {
	lock_guard<mutex> lock(fm);
	if(!appDir) {
		if(APP_NAME_STR != "")
			appDir = File("/usr/share/" APP_NAME_STR);
		else
			throw io_exception("No appDir specified");
	}
	return appDir;
}

void File::writeln(const std::string &line) {
	write(line + "\n");
}

File File::changeSuffix(const std::string &ext) {
	int dot = fileName.find_last_of('.');
	if(dot != string::npos)
		return File(fileName.substr(0, dot) + ext);
	return File(fileName + ext);
}

std::string File::suffix() const {

	int dot = fileName.find_last_of('.');
	if(dot != string::npos)
		return fileName.substr(dot);
	return "";
}

} // namespace

#ifdef UNIT_TEST

#include "catch.hpp"

TEST_CASE("utils::File", "File operations") {

	using namespace utils;
	using namespace std;

	// Delete to be safe
	std::remove("temp.text");

	// File
	File file { "temp.text" };

	REQUIRE(file.getName() == "temp.text");

	file.write("First line\nSecond line");
	file.close();
	REQUIRE(file.exists());
	REQUIRE(file.getSize() > 5);
	REQUIRE(file.getSize() < 50);

	file = File { "temp.text" };

	auto data = file.readAll();
	REQUIRE(data.size() > 0);

	vector<string> lines = file.getLines();

	REQUIRE(lines.size() == 2);

	file.remove();

	REQUIRE(!file.exists());

}

#endif
