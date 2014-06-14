
#include "archive.h"
#include "ziplib/zip.h"

#include <vector>
#include <cstring>
#include <coreutils/log.h>
#define _UNIX
#include  "unrar/dll.hpp"

using namespace std;
using namespace utils;

/*
class ExtArchive : public Archive {
	File extract(const string &name) {
		system("lha x " + name);
	}
};*/

class ZipFile : public Archive {
public:
	ZipFile(const string &fileName, const string &workDir = ".") : workDir(workDir) {
		zipFile = zip_open(fileName.c_str(), 0, NULL);
	}

	~ZipFile() {
		if(zipFile)
			close();
	}

	void close() {
		zip_close(zipFile);
		zipFile = nullptr;
	}

	File extract(const string &name) {
		int i = zip_name_locate(zipFile, name.c_str(), ZIP_FL_NOCASE);
		if(i >= 0) {
			struct zip_file *zf = zip_fopen_index(zipFile, i, 0);
			File file(workDir + "/" + name);
			vector<uint8_t> buffer(2048);
			while(true) {
				int bytes = zip_fread(zf, &buffer[0], buffer.size());
				if(bytes > 0)
					file.write(&buffer[0], bytes);
				else
					break;
			}
			file.close();
			zip_fclose(zf);
			return file;
		}
		return File();
	}

	virtual string nameFromPosition(int pos) const {
		struct zip_stat sz;
		zip_stat_index(zipFile, pos, 0, &sz);
		return string(sz.name);
	}

	virtual int totalFiles() const {
		return zip_get_num_files(zipFile);
	}

private:
	struct zip *zipFile;
	string workDir;
};


class RarFile : public Archive {
public:
	RarFile(const string &fileName, const string &workDir = ".") : workDir(workDir) {

		memset(&archiveInfo, 0, sizeof(archiveInfo));
		archiveInfo.CmtBuf = NULL;
		archiveInfo.OpenMode = RAR_OM_EXTRACT;
		archiveInfo.ArcName = (char*)fileName.c_str();
		rarFile = RAROpenArchiveEx(&archiveInfo);
		if(archiveInfo.OpenResult != 0) {
			throw archive_exception("Bad RAR");
		};
		currentPos = -1;

	}

	~RarFile() {
		RARCloseArchive(rarFile);
	}

	File extract(const string &name) {
		//RARHeaderDataEx fileInfo;
		//int RHCode = RARReadHeaderEx(rarFile, &fileInfo);
		int PFCode = RARProcessFile(rarFile, RAR_EXTRACT, (char*)workDir.c_str(), NULL);

		LOGD("extract %d", PFCode);

		currentPos++;

		File f { workDir + "/" + fileInfo.FileName };

		int RHCode = RARReadHeaderEx(rarFile, &fileInfo);
		LOGD("RHCode %d %s", RHCode, fileInfo.FileName);
		if(RHCode !=0)
			currentPos--;

		return f;
	}

	virtual string nameFromPosition(int pos) const {
		
		LOGD("POS %d", pos);
		while(currentPos < pos) {
			int PFCode = RARProcessFile(rarFile, RAR_SKIP, NULL, NULL);
			LOGD("PFCode %d", PFCode);

			int RHCode = RARReadHeaderEx(rarFile, &fileInfo);
			LOGD("RHCode %d %s", RHCode, fileInfo.FileName);
			if(RHCode !=0)
				return "";
			currentPos++;
		}
		//int RHCode = RARReadHeaderEx(rarFile, &fileInfo);
		LOGD("pos %d %s", currentPos, fileInfo.FileName);
		//if(RHCode !=0)
		//	return "";
		return fileInfo.FileName;
	}

	virtual int totalFiles() const {
		return -1;
	}

private:
	RAROpenArchiveDataEx archiveInfo;
	
	HANDLE rarFile;
	mutable int currentPos;
	//struct zip *zipFile;
	mutable RARHeaderDataEx fileInfo;
	string workDir;
};


Archive *Archive::open(const std::string &fileName, const std::string &targetDir) {
	if(utils::endsWith(fileName, ".zip"))
		return new ZipFile(fileName, targetDir);
	else if(utils::endsWith(fileName, ".rar"))
		return new RarFile(fileName, targetDir);
}

bool Archive::canHandle(const std::string &name) {
	return utils::endsWith(name, ".zip");
}

