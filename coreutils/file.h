#ifndef FILE_H
#define FILE_H

#include <sys/stat.h>
#include <stdint.h>
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
	virtual const char *what() const throw() { return "File not found"; }
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
	~File() {
		if(readFP)
			fclose(readFP);
		if(writeFP)
			fclose(writeFP);
	}
	void write(const uint8_t *data, int size); // throw(io_exception);
	void write(const std::string &text);
	void close();

	bool exists();
	static bool exists(const std::string &fileName);
	uint8_t *getPtr();
	const std::string &getName() const { return fileName; }
	int getSize() const { 
		if(size < 0) {
			struct stat ss;
			if(stat(fileName.c_str(), &ss) != 0)
				throw io_exception("Could not stat file");
			size = ss.st_size;
		}
		return size;
	}
	std::vector<std::string> getLines();
	void remove() {
		if(std::remove(fileName.c_str()) != 0)
			throw io_exception("Could not delete file");
	}

	void copyFrom(File &otherFile);

	int read(uint8_t *target, int len);

	void seek(int where);
	template <typename T> int read(T *target, int count) {
		open(READ);
		return fread(target, sizeof(T), count, readFP);
	}

	template <typename T> T read() {
		open(READ);
		T temp;
		if(fread(&temp, sizeof(T), 1, readFP) > 0)
			return temp;
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