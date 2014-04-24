#ifndef FUSEFS_H
#define FUSEFS_H

#include <coreutils/log.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <thread>

class FuseFS {
public:

	struct file {
		typedef std::function<std::vector<uint8_t>()> get_func;
		typedef std::function<void(const std::vector<uint8_t>&)> put_func;
		file() {}
		file(const std::string &name, const std::vector<uint8_t> &contents) : name(name), contents(contents) {}
		file(const std::string &name, get_func gc) : name(name), get_contents(gc) {}
		file(const std::string &name, get_func gc, put_func pc) : name(name), get_contents(gc), put_contents(pc) {}

		void update_contents() {
			if(dirty && !written) {
				if(get_contents)
					contents = get_contents();
				dirty = false;
			}
		}

		bool dirty = true;
		bool written = false;
		std::string name;
		get_func get_contents;
		put_func put_contents;
		std::vector<uint8_t> contents;
		std::map<std::string, file> files;
	};

	struct open_file {


	};

	static FuseFS& getInstance();

	FuseFS();

	static int mknod(const char *p, mode_t m, dev_t d) {
		LOGD("mknod");
		return 0;
	}

	static int truncate(const char *path, off_t o) {
		auto &fs = filesys[0]->root;
		LOGD("truncate %d", o);
		if(fs.files.count(path+1) == 1) {
			LOGD("Found");
			auto &ff = fs.files[path+1];
			ff.contents.resize(o);
			ff.written = true;
		}
		return 0;
	}

	~FuseFS() {
		fprintf(stderr, "Destroying fuse\n");
		//if(fuseThread.joinable()) {
		quitThread = true;
		fuse_exit(fs);
			//fuseThread.join();
		//}
		if(chan)
			fuse_unmount(mount_point.c_str(), chan);
		fuse_destroy(fs);
	}
#if 0
	void add(int pos, std::vector<std::string> parts, file &f) {
		if(f.files.count(parts[pos]) < 1) {
			auto &f = f.files[parts[pos]] = file(parts[pos], true);
			add(pos+1, parts, f);
		}

	}

	file add(const std::string &name) {
		auto parts = utils::split(name, "/");
		int pos = 0;
		while(parts[pos] == "") pos++;
		add(pos, parts);
	}
#endif
	file addFile(const std::string &name, const std::string &contents = "") {
		auto c = std::vector<uint8_t>(contents.begin(), contents.end());
		LOGD("Vector len %d", c.size());
		root.files[name] = file(name, c);
		return root.files[name];
	};

	const file& addFile(const std::string &name, file::get_func get_contents) {
		root.files[name] = file(name, get_contents);
		return root.files[name];
	};

	const file& addFile(const std::string &name, file::get_func get_contents, file::put_func put_contents) {
		root.files[name] = file(name, get_contents, put_contents);
		return root.files[name];
	};

	void mount(const std::string &mount_point) {
		struct fuse_args args;
		this->mount_point = mount_point;
		args.argc = 0;
		args.argv = { nullptr };
		args.allocated = 0;
		LOGD("FUSE MAIN");
		chan = fuse_mount(mount_point.c_str(), &args);
		fs = fuse_new(chan, nullptr, &fuseops, sizeof(fuseops), &args);
		LOGD("%p", fs);
		//char *argv[] = { "cmd", "/home/sasq/fuse" };
		//fuse_main(2, argv, &fuseops, NULL);
		LOGD("DONE");
		quitThread = false;
		fuseThread = std::thread([=]() {
			while(!quitThread ) {
				fuse_loop(fs);
			}
		});
		fuseThread.detach();
	}



	// void update() {
	// 	int rc = fuse_loop(fs);
	// 	if(rc != 0)
	// 		LOGD("Update failed");
	// }

private:

	static int getattr(const char *path, struct stat *s) {
		LOGD("Getattr %s", path);
		auto &fs = filesys[0]->root;
		memset(s, 0, sizeof(struct stat));

		if(strcmp(path, "/") == 0) {
			s->st_mode = S_IFDIR | 0755;
			s->st_nlink = 2;
			return 0;
		}

		if(fs.files.count(path+1) == 1) {
			LOGD("Found");
			auto &ff = fs.files[path+1];
			ff.dirty = true;
			ff.update_contents();
			s->st_size = ff.contents.size();
			s->st_nlink = 1;
			s->st_blocks = (s->st_size + 511) / 512;
			s->st_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ;
			return 0;
		}
		return -ENOENT;
	}

	static int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
		auto &fs = filesys[0]->root;
		LOGD("Readdir %s", path);
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		for(const auto &f : fs.files) {
			filler(buf, f.second.name.c_str(), NULL, 0);
		}
		return 0;
	}

	static int flush(const char *path, struct fuse_file_info *fi) {
		LOGD("Flush %s", path);
		auto &fs = filesys[0]->root;
		if(fs.files.count(path+1) == 1) {
			LOGD("Found");
			auto &ff = fs.files[path+1];
			ff.dirty = true;
			if(ff.written) {
				if(ff.put_contents)
					ff.put_contents(ff.contents);
				ff.written = false;
			}
			return 0;
		}
		return -ENOENT;
	}

	static int open(const char *path, struct fuse_file_info *fi) {
		LOGD("Open %s", path);
		auto &fs = filesys[0]->root;
		if(fs.files.count(path+1) == 1) {
			LOGD("Found");
			auto &ff = fs.files[path+1];
			ff.update_contents();
			return 0;

		}
		return -ENOENT;
	}

	static int write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
		auto &fs = filesys[0]->root;
		LOGD("Write %s (%d+%d)", path, offset, size);
		if(fs.files.count(path+1) == 1) {
			LOGD("Found");
			auto &ff = fs.files[path+1];
			if(offset + size > ff.contents.size()) {
				LOGD("Growing to %d", offset+size);
				ff.contents.resize(offset+size);
			}
			memcpy(&ff.contents[offset], buf, size);
			ff.written = true;
			return size;
		}
		return -ENOENT;
	}

	static int read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
		auto &fs = filesys[0]->root;
		LOGD("Read %s (%d)", path, size);
		if(fs.files.count(path+1) == 1) {
			LOGD("Found");
			auto &ff = fs.files[path+1];
			if(offset + size > ff.contents.size())
				size = ff.contents.size() - offset;
			memcpy(buf, &ff.contents[offset], size);
			return size;
		}
		return -ENOENT;
	}
/*
	static int create(const char *path, mode_t mode, struct fuse_file_info *fi) {
		LOGD("Create %s (%d)", path, mode);
		return 0;
	}

	static int chmod(const char *path, mode_t mode) {
		LOGD("chmod");
		return 0;
	}

	static int chown(const char *path, uid_t u, gid_t g) {
		LOGD("chown");
		return 0;
	}

	static int unlink(const char *path) {
		LOGD("unlink");
		return 0;
	}

	static int rename(const char *path, const char *p2) {
		LOGD("rename");
		return 0;
	}
*/
	std::atomic<bool> quitThread;
	std::thread fuseThread;

	struct fuse_operations fuseops;
	struct fuse *fs;
	struct fuse_chan *chan;
	std::string mount_point;

	//std::map<std::string, file> files;
	file root;

	static std::vector<FuseFS*> filesys;
};

#endif // FUSEFS_H
