
#include "file.h"

#include <sys/stat.h>

#include "utils.h"
#include "log.h"


#ifdef WIN32
#include <windows.h>
#endif
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <dirent.h>
namespace utils {

using namespace std;

File::File() : size(-1), loaded(false), writeFP(nullptr), readFP(nullptr) {}

File::File(const string &name, Mode mode) : fileName(rstrip(name, '/')), size(-1), loaded(false), writeFP(nullptr), readFP(nullptr) {

	fileName = resolvePath(fileName);

	if(mode != NONE)
		open(mode);
};

File::File(const File &parent, const std::string &name, Mode mode) : File(parent.getName() + "/" + name, mode) {}

File::File(const std::string &parent, const std::string &name, Mode mode) : File(rstrip(parent) + "/" + name, mode) {}

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

void File::readAll()  {		
	if(!loaded) {
		FILE *fp = fopen(fileName.c_str(), "rb");
		if(!fp)
			throw file_not_found_exception(fileName);
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		data.resize(size);
		int rc = fread(&data[0], 1, size, fp);
		if(rc != size)
			throw io_exception{};
		fclose(fp);
		loaded = true;
	}
}

void File::open(Mode mode) {
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

bool File::isChildOf(const File &f) {
	string myPath = resolvePath(getName());
	string parentPath = resolvePath(f.getName());
	return (myPath.find(parentPath) == 0);
}

int File::read(uint8_t *target, int len) {
	open(READ);
	return fread(target, 1, len, readFP);
}

void File::seek(int where) {
	open(READ);
	if(!readFP)
		throw file_not_found_exception(fileName);
	fseek(readFP, where, SEEK_SET);
}

vector<string> File::getLines() {
	vector<string> lines;
	close();
	if(!loaded)
		readAll();
	string source { reinterpret_cast<char*>(&data[0]), (unsigned int)size };
	stringstream ss(source);
	string to;

	while(getline(ss, to)) {
		lines.push_back(to);
    }
    return lines;
}

void File::write(const uint8_t *data, int size) {
	open(WRITE);
	fwrite(data, 1, size, writeFP);
}

void File::write(const string &data) {
	open(WRITE);
	fwrite(data.c_str(), 1, data.length(), writeFP);
}


void File::copyFrom(File &otherFile) {
	open(WRITE);
	uint8_t *ptr = otherFile.getPtr();
	int size = otherFile.getSize();
	fwrite(ptr, 1, size, writeFP);
}

void File::close() {
	if(writeFP)
		fclose(writeFP);
	writeFP = nullptr;
}

bool File::exists() {
	struct stat ss;
	return (stat(fileName.c_str(), &ss) == 0);
}

bool File::exists(const string &fileName) {
	struct stat ss;
	return (stat(fileName.c_str(), &ss) == 0);
}

uint8_t *File::getPtr() {
	close();
	if(!loaded)
		readAll();
	return &data[0];
}

const std::vector<uint8_t>& File::getData() {
	close();
	if(!loaded)
		readAll();
	return data;
}

}