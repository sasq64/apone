#ifndef FILE_H
#define FILE_H

#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>

//#ifdef LINUX
//#include <linux/limits.h>
//#endif

#include "utils.h"

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

//#define THROW(e, args...) throw e(args, __FILE__, __LINE__)

#define XSF(x) SF(x)
#define SF(x) #x
#define APP_NAME_STR XSF(APP_NAME)

class File {
public:

	enum Mode {
		NONE = 0,
		READ = 1,
		WRITE = 2
	};

	static bool exists(const std::string &fileName);
	static void copy(const std::string &from, const std::string &to);
	static std::string resolvePath(const std::string &fileName);
	static File cwd();

	static void remove(const std::string &fileName);

	static const std::string getCacheDir();
	static const std::string getConfigDir();
	static const std::string getAppDir();
	static void setAppDir(const std::string &a);

	static const std::string getUserDir();

	static File findFile(const std::string &path, const std::string &name);

	File();
	File(const std::string &name, Mode mode  = NONE);
	File(const File &parent, const std::string &name, Mode mode  = NONE);
	File(const std::string &parent, const std::string &name, Mode mode = NONE);
	~File();

	void write(const uint8_t *data, int size); // throw(io_exception);
	void write(const std::string &text);
	void close();

	std::vector<File> listFiles();

	bool exists();

	uint8_t *getPtr();
	const std::vector<uint8_t>& getData();

	const std::string &getName() const { return fileName; }

	const std::string getDirectory() const { return path_directory(resolvePath(fileName)); }


	int64_t getSize();

	bool isDir() const;

	std::vector<std::string> getLines();

	void remove();

	void copyFrom(File &otherFile);
	void copyFrom(const std::string &other);

	int read(uint8_t *target, int len);

	bool isChildOf(const File &f);

	void seek(int where);

	template <typename T> int read(T *target, int count) {
		open(READ);
		return (int)fread(target, sizeof(T), (size_t)count, readFP);
	}

	std::string read();

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

	void readAll();

	void clear();

	void writeln(const std::string &line);

	static File NO_FILE;

private:

	void open(Mode mode);

	static std::string appDir;
	static std::string userDir;

	std::string fileName;
	std::vector<uint8_t> data;
	mutable int64_t size;
	bool loaded;
	FILE *writeFP;
	FILE *readFP;
};

}

#endif // FILE_H