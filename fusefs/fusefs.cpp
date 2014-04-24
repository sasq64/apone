#include "fusefs.h"

std::vector<FuseFS*> FuseFS::filesys;


FuseFS::FuseFS() {
	fuseops.getattr = FuseFS::getattr;
	fuseops.readdir = FuseFS::readdir;
	fuseops.open = FuseFS::open;
	fuseops.read = FuseFS::read;
	fuseops.write = FuseFS::write;
	fuseops.flush = FuseFS::flush;
	//fuseops.create = FuseFS::create;
	//fuseops.chown = FuseFS::chown;
	//fuseops.chmod = FuseFS::chmod;
	//fuseops.unlink = FuseFS::unlink;
	//fuseops.rename = FuseFS::rename;
	//fuseops.mknod = FuseFS::mknod;
	fuseops.truncate = FuseFS::truncate;
	filesys.push_back(this);		
	chan = nullptr;
	fs = nullptr;
};

FuseFS& FuseFS::getInstance() {
	static FuseFS fusefs;
	return fusefs;
}
