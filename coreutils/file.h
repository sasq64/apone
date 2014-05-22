#ifndef FILE_H
#define FILE_H

#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <typeinfo>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

//#include "utils/format.h"

namespace utils {

class io_exception : public std::exception {
public:
	io_exception(const char *ptr = "IO Exception") : msg(ptr) {}
	virtual const char *what() const throw() { return msg; }
private:
	const char *msg;
};

class file_not_found_exception : public std::exception {
public:
	file_not_found_exception(const std::string &fileName = "") : msg(std::string("File not found: ") + fileName) {}
	file_not_found_exception(const char *fileName = "") : msg(std::string("File not found: ") + fileName) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

#define THROW(e, args...) throw e(args, __FILE__, __LINE__)

class File {
public:

	enum Mode {
		NONE = 0,
		READ = 1,
		WRITE = 2
	};

	File();
	File(const std::string &name, Mode mode  = NONE);
	File(const File &parent, const std::string &name, Mode mode  = NONE);
	File(const std::string &parent, const std::string &name, Mode mode  = NONE);
	~File() {
		if(readFP)
			fclose(readFP);
		if(writeFP)
			fclose(writeFP);
	}
	void write(const uint8_t *data, int size); // throw(io_exception);
	void write(const std::string &text);
	void close();

	std::vector<File> listFiles();

	bool exists();
	static bool exists(const std::string &fileName);

	static std::string resolvePath(const std::string &fileName) {
		char temp[PATH_MAX];
		if(::realpath(fileName.c_str(), temp))
			return temp;
		return fileName;
	}

	static File cwd() {
		char temp[PATH_MAX];
		if(::getcwd(temp, sizeof(temp))) {
			return File(temp);
		}
		throw io_exception {"Could not get current directory"};
	}

	uint8_t *getPtr();
	const std::vector<uint8_t>& getData();

	const std::string &getName() const { return fileName; }
	int getSize() const { 
		if(size < 0) {
			struct stat ss;
			if(stat(fileName.c_str(), &ss) != 0)
				throw io_exception {"Could not stat file"};
			size = ss.st_size;
		}
		return size;
	}

	bool isDir() const {
		struct stat ss;
		if(stat(fileName.c_str(), &ss) != 0)
			throw io_exception {"Could not stat file"};
		return S_ISDIR(ss.st_mode);
	}

	std::vector<std::string> getLines();
	void remove() {
		if(std::remove(fileName.c_str()) != 0)
			throw io_exception {"Could not delete file"};
	}

	static void remove(const std::string &fileName) {
		if(std::remove(fileName.c_str()) != 0)
			throw io_exception {"Could not delete file"};
	}

	void copyFrom(File &otherFile);

	int read(uint8_t *target, int len);

	bool isChildOf(const File &f);

	void seek(int where);
	template <typename T> int read(T *target, int count) {
		open(READ);
		return fread(target, sizeof(T), count, readFP);
	}

	std::string read() {
		open(READ);
		if(!loaded)
			readAll();
		return std::string((const char *)&data[0], size);
	}

	template <typename T> T read() {
		open(READ);
		T temp;
		if(fread(&temp, sizeof(T), 1, readFP) > 0)
			return temp;
		throw io_exception{};
	}

	template <typename T> void write(const T &t) {
		open(WRITE);
		if(fwrite(&t, sizeof(T), 1, writeFP) > 0)
			return;
		throw io_exception{};
	}

	void readAll(); // throw(file_not_found_exception, io_exception);

private:
	void open(Mode mode);

	std::string fileName;
	std::vector<uint8_t> data;
	mutable int size;
	bool loaded;
	FILE *writeFP;
	FILE *readFP;
};

}

#endif // FILE_H