#include "file.h"

#include <sys/stat.h>

#include "format.h"
//#include "utils.h"
#include "log.h"


#ifdef WIN32
#include <windows.h>
#endif
#include <unistd.h>
//#include <cstring>
//#include <iostream>
//#include <sstream>
#include <iomanip>
#include <dirent.h>

#ifdef APPLE
#include <mach-o/dyld.h>
#endif

namespace utils {

using namespace std;

File File::NO_FILE;

string File::appDir = "/usr/share/" APP_NAME_STR;
string File::userDir;


File::File() : size(-1), writeFP(nullptr), readFP(nullptr) {}

File::File(const string &name, const Mode mode) : fileName(rstrip(name, '/')), size(-1), writeFP(nullptr), readFP(nullptr) {

	fileName = resolvePath(fileName);

	if(mode != NONE)
		open(mode);
};

File::File(const File &parent, const string &name, const Mode mode) : File(parent.getName() + "/" + name, mode) {}

File::File(const string &parent, const string &name, const Mode mode) : File(rstrip(parent) + "/" + name, mode) {}

vector<File> File::listFiles() {
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
			makedirs(fileName);
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

void File::seek(int where) {
	open(READ);
	if(!readFP)
		throw file_not_found_exception(fileName);
	fseek(readFP, where, SEEK_SET);
}


vector<string> File::getLines() {
	vector<string> lines;
	auto data = readAll();
	string source { reinterpret_cast<char*>(&data[0]), (unsigned int)data.size() };
	stringstream ss(source);
	string to;

	while(getline(ss, to)) {
		auto l = to.length()-1;
		while(to[l] == 10 || to[l] == 13)
			to = to.substr(0, l--);
		lines.push_back(to);
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

File File::findFile(const string &path, const string &name) {
	if(name == "")
		return NO_FILE;
	auto parts = split(path, ":");
	for(string p : parts) {
		if(p.length() > 0) {
			if(p[p.length()-1] != '/')
			p += "/";
			File f { p + name };
			if(f.exists())
				return f;
		}
	}
	return NO_FILE;
}

#ifdef APP_NAME

const std::string File::getCacheDir() {
    const char *home = getenv("HOME");
    auto d = format("%s/.cache/" APP_NAME_STR, home);
    if(!exists(d))
        utils::makedirs(d);
    return d + "/";
}

const std::string File::getConfigDir() {
    const char *home = getenv("HOME");
    auto d = format("%s/.config/" APP_NAME_STR, home);
    if(!exists(d))
        utils::makedirs(d);
    return d + "/";
}

const std::string File::getUserDir() {
    std::string home = getenv("HOME");
#ifdef WIN32
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
	if(_fullpath(temp, fileName.c_str(), PATH_MAX))
#else
	if(::realpath(fileName.c_str(), temp))
#endif
		return std::string(temp);
	return fileName;
}

File File::cwd() {
	char temp[PATH_MAX];
	if(::getcwd(temp, sizeof(temp))) {
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

const std::string File::getExeDir() {

	static char buf[1024];
#if defined LINUX
	int rc = readlink("/proc/self/exe", buf, sizeof(buf)-1);
	if(rc >= 0) {
		buf[rc] = 0;
		return path_directory(buf);
	}
#elif defined APPLE
	uint32_t size = sizeof(buf);
	if(_NSGetExecutablePath(buf, &size) == 0) {
		return path_directory(buf);
	}
#endif
	return "";
}


void File::setAppDir(const std::string &a) {
	LOGD("Setting appdir to %s", a);
	appDir = a;
}

const std::string File::getAppDir() {
	if(appDir == "") {
		if(APP_NAME_STR != "")
			appDir = "/usr/share/" APP_NAME_STR;
		else
			throw io_exception("No appDir specified");
	}
	return appDir;
}

void File::writeln(const std::string &line) {
	write(line + "\n");
}

}

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