
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

namespace utils {

using namespace std;

File::File() : size(-1), loaded(false), writeFP(nullptr), readFP(nullptr) {}

File::File(const string &name) : fileName(name), size(-1), loaded(false), writeFP(nullptr), readFP(nullptr) {
};

void File::readAll()  {		
	if(!loaded) {
		FILE *fp = fopen(fileName.c_str(), "rb");
		if(!fp)
			throw file_not_found_exception{};
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		data.reserve(size);
		int rc = fread(&data[0], 1, size, fp);
		if(rc != size)
			throw io_exception{};
		fclose(fp);
		loaded = true;
	}
}

void File::open() {
	if(readFP == nullptr) {
		readFP = fopen(fileName.c_str(), "rb");
		if(!readFP)
			throw file_not_found_exception{};
	}
}

int File::read(uint8_t *target, int len) {
	open();
	return fread(target, 1, len, readFP);
}

void File::seek(int where) {
	open();
	if(!readFP)
		throw file_not_found_exception{};
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
	if(!writeFP) {
		makedirs(fileName);
		writeFP = fopen(fileName.c_str(), "wb");
		if(!writeFP)
			throw io_exception{"Could not open file for writing"};
	}
	fwrite(data, 1, size, writeFP);
}

void File::write(const string &data) {
	if(!writeFP) {
		makedirs(fileName);
		writeFP = fopen(fileName.c_str(), "wb");
		if(!writeFP)
			throw io_exception{"Could not open file for writing"};
	}
	fwrite(data.c_str(), 1, data.length(), writeFP);
}


void File::copyFrom(File &otherFile) {
	if(!writeFP) {
		makedirs(fileName);
		writeFP = fopen(fileName.c_str(), "wb");
		if(!writeFP)
			throw io_exception{"Could not open file for writing"};
	}
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

}