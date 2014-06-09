
extern "C" {
#include "archdep.h"
#include "drive.h"
#include "gfxoutput.h"
#include "init.h"
#include "initcmdline.h"
#include "lib.h"
#include "machine.h"
#include "maincpu.h"
#include "psid.h"
#include "resources.h"
#include "sound.h"
#include "sysfile.h"


void psid_play(short *buf, int size);
const char *psid_get_name();
const char *psid_get_author();
const char *psid_get_copyright();
int psid_tunes(int* default_tune);

}

#include "VicePlugin.h"
#include "../../chipplayer.h"
#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <coreutils/file.h>
#include <crypto/md5.h>

#include <set>
#include <algorithm>

int console_mode = 1;
int vsid_mode = 1;
int video_disabled_mode = 1;

namespace chipmachine {

using namespace std;
using namespace utils;


static bool videomode_is_ntsc;
static bool videomode_is_forced;
static int sid;
static bool sid_is_forced;

 template <typename T> const T get(const vector<uint8_t> &v, int offset) {}

template <> const uint16_t get(const vector<uint8_t> &v, int offset) {
	return (v[offset] <<8) | v[offset+1];
}

template <> const uint32_t get(const vector<uint8_t> &v, int offset) {
	return (v[offset] <<24) | (v[offset+1]<<16) | (v[offset+2] <<8) | v[offset+3];
}

enum {
	MAGICID = 0,
	PSID_VERSION = 4,
	DATA_OFFSET = 6,
	LOAD_ADDRESS  = 8,
	INIT_ADDRESS = 0xA,
	PLAY_ADDRESS = 0xC,
	SONGS = 0xE,
	START_SONG = 0x10,
	SPEED = 0x12,
	FLAGS = 0x76
};

class VicePlayer : public ChipPlayer {
public:

	vector<uint8_t> calculateMD5(vector<uint8_t> data) {
		
		uint8_t speed = (data[0] == 'R') ? 60 : 0;
		uint16_t version = get<uint16_t>(data, PSID_VERSION);

		uint16_t initAdr = get<uint16_t>(data, INIT_ADDRESS);
		uint16_t playAdr = get<uint16_t>(data, PLAY_ADDRESS);
		uint16_t songs = get<uint16_t>(data, SONGS);

		uint32_t speedBits = get<uint32_t>(data, SPEED);
		uint16_t flags = get<uint16_t>(data, FLAGS);

		MD5 md5;

		auto offset = (version == 2) ? 126 : 120;

		md5.add(data, offset);

		md5.add(initAdr);
		md5.add(playAdr);
		md5.add(songs);	

		for(int i=0; i<songs; i++) {
			if((speedBits & (1 << i)) != 0) {
				md5.add((uint8_t)60);
			} else {
				md5.add(speed);
			}
		}

		if((flags & 0x8) != 0) {
			md5.add((uint8_t)2);
		}

		return md5.get();
	}

#if 0
	private byte[] calculateMD5(vector<uint8_t> data) {
		ByteBuffer src = ByteBuffer.wrap(module, 0, size);		
		src.order(ByteOrder.BIG_ENDIAN);
		
		uint8_t speed =(data[0] == 'R' ? 60 : 0);

		uint16_t version = get<uint16_t>(data, 4);

		byte[] id = new byte[4];
		src.get(id);
		int version = src.getShort();
		src.position(8);
		/*short loadAdress =*/ src.getShort();
		short initAdress = src.getShort();
		short playAdress = src.getShort();
		short songs = src.getShort();
		/*short startSong =*/ src.getShort();
		int speedBits = src.getInt();
		src.position(0x76);
		int flags = src.getShort();
		
		int offset = 120;
		if (version == 2) {
			offset = 126;
		}
		
		int speed = 0;
		if (id[0] == 'R') {
			speed = 60;
		}
		
		Log.d(TAG, "speed %08x, flags %04x left %d songs %d init %x", speed, flags, size - offset, songs, initAdress);
		
		try {
			MessageDigest md = MessageDigest.getInstance("MD5");
			
			md.update(module, offset, size - offset);
			
			ByteBuffer dest = ByteBuffer.allocate(128);
			dest.order(ByteOrder.LITTLE_ENDIAN);
			
			dest.putShort(initAdress);
			dest.putShort(playAdress);
			dest.putShort(songs);
			
			for (int i = 0; i < songs; i ++) {
				if ((speedBits & (1 << i)) != 0) {
					dest.put((byte) 60);
				} else {
					dest.put((byte) speed);
				}
			}
			
			if ((flags & 0x8) != 0) {
				dest.put((byte) 2);
			}

			byte[] d = dest.array();
			md.update(d, 0, dest.position());
			
			byte[] md5 = md.digest();
			Log.d(TAG, "%d %02x %02x DIGEST %02x %02x %02x %02x", d.length, d[0], d[1], md5[0], md5[1], md5[2], md5[3]);
			return md5;
		} catch (NoSuchAlgorithmException e) {
			throw new RuntimeException(e);
		}
	}
#endif


	static bool init(const string &c64Dir) {
		maincpu_early_init();
		machine_setup_context();
		drive_setup_context();
		machine_early_init();
		sysfile_init("C64");
		gfxoutput_early_init();
		if(init_resources() < 0) {
			archdep_startup_log_error("Failed to init resources");
			return false;
		}

		if(resources_set_defaults() < 0) {
			archdep_startup_log_error("Cannot set defaults.\n");
			return false;
		}

		resources_set_int("SidResidSampling", 0);
		resources_set_int("VICIIVideoCache", 0);
		resources_set_string("Directory", c64Dir.c_str());
		if(init_main() < 0) {
			archdep_startup_log_error("Failed to init main");
			return false;
		}

		return true;
	}

	static void c64_song_init()
	{
		/* Set default, potentially overridden by reset. */
		resources_set_int("MachineVideoStandard", videomode_is_ntsc ? MACHINE_SYNC_NTSC : MACHINE_SYNC_PAL);
		
		/* Default to 6581 in case tune doesn't specify. */
		resources_set_int("SidModel", sid);

		/* Reset C64, which also initializes PSID for us. */
		machine_trigger_reset(MACHINE_RESET_MODE_SOFT);

		/* Now force video mode if we are asked to. */
		if (videomode_is_forced)
		{
			resources_set_int("MachineVideoStandard", videomode_is_ntsc ? MACHINE_SYNC_NTSC : MACHINE_SYNC_PAL);
		}
		
		/* Force the SID model if told to in the settings */
		if (sid_is_forced)
		{
			resources_set_int("SidModel", sid);
		}

	}

	VicePlayer(const string &sidFile) {
		int ret = psid_load_file(sidFile.c_str());
		LOGD("Loaded %s -> %d", sidFile, ret);
		if (ret == 0) {

			File f { sidFile };
			auto data = f.getData();
			auto md5 = calculateMD5(data);
			uint32_t key = get<uint32_t>(md5, 0);
			LOGD("MD5: [%02x] %08x", md5, key);
			songLengths = VicePlugin::findLengths(key);

			int defaultSong;
			int songs = psid_tunes(&defaultSong);
			defaultSong--;
			LOGD("DEFSONG: %d", defaultSong);
			currentLength = 0;
			currentPos = 0;
			if((int)songLengths.size() > defaultSong) {
				currentLength = songLengths[defaultSong];
			}
			LOGD("Length:%d", currentLength);

			setMeta(
				"title", psid_get_name(),
				"composer", psid_get_author(),
				"copyright", psid_get_copyright(),
				"songs", songs,
				"length", currentLength,
				"startSong", defaultSong
			);

			c64_song_init();
		}
	}

	~VicePlayer() {
		psid_set_tune(-1);
	}

	virtual void seekTo(int song, int seconds = -1) {
		if(song >= 0) {
			psid_set_tune(song+1);
			c64_song_init();
			currentLength = 0;
			currentPos = 0;
			if((int)songLengths.size() > song) {
				currentLength = songLengths[song];
			}
			LOGD("Length:%d", currentLength);
			setMeta("length", currentLength);
		}
	}

	virtual int getSamples(int16_t *target, int size) {
		currentPos += (size/2);
		//LOGD("%d vs %d", currentPos, currentLength*44100);
		//if(currentLength > 0 && currentPos > currentLength*44100)
		//	return -1;
		psid_play(target, size);
		return size;
	}
	uint32_t currentLength;
	uint32_t currentPos;
	std::vector<uint16_t> songLengths;
};

VicePlugin::VicePlugin(const string &dataDir) {
	VicePlayer::init(dataDir.c_str());
	readLengths();
}

VicePlugin::VicePlugin(const unsigned char *data) {
	mkdir("c64", 0777);

	FILE *fp;
	fp = fopen("c64/basic", "wb");
	fwrite(&data[0], 1, 8192, fp);
	fclose(fp);

	fp = fopen("c64/chargen", "wb");
	fwrite(&data[8192], 1, 4096, fp);
	fclose(fp);

	fp = fopen("c64/kernal", "wb");
	fwrite(&data[8192+4096], 1, 8192, fp);
	fclose(fp);
	VicePlayer::init("c64");

	readLengths();
}

void VicePlugin::readLengths() {

	File f { "data/songlengths.dat" };

	if(f.exists()) {
		auto data = f.getData();

		auto len = get<uint32_t>(data, 0);
		LOGD("Found %d songs in songlengths.dat", len);

		mainHash.resize(6*len);
		memcpy(&mainHash[0], &data[4], 6*len);

		auto offs = 4 + 6*len;
		auto elen = (data.size() - offs) / 2;

		extraLengths.resize(elen*2);
		for(int i=0; i<(int)elen; i++)
			extraLengths[i] = get<uint16_t>(data, offs+i*2);
	}
}

VicePlugin::~VicePlugin() {
	LOGD("VicePlugin destroy\n");
	machine_shutdown();
}

static const set<string> ext { ".sid", ".psid", ".rsid" , ".2sid", ".mus" };

bool VicePlugin::canHandle(const std::string &name) {
	for(string x : ext) {
		if(utils::endsWith(name, x))
			return true;
	}
	return false;
}

ChipPlayer *VicePlugin::fromFile(const std::string &fileName) {
	return new VicePlayer { fileName };
}
vector<uint8_t> VicePlugin::mainHash;
vector<uint16_t> VicePlugin::extraLengths;

vector<uint16_t> VicePlugin::findLengths(uint32_t key) {

	vector<uint16_t> songLengths;
	//long key = ((md5[0]&0xff) << 24) | ((md5[1]&0xff) << 16) | ((md5[2]&0xff) << 8) | (md5[3] & 0xff);
	//key &= 0xffffffffL;

	int first = 0;
	int upto = mainHash.size() / 6;
	//int found = -1;
	

	//short lens [] = new short [128];
	
	//Log.d(TAG, "MD5 %08x", key);
	while (first < upto) {
		int mid = (first + upto) / 2;  // Compute mid point.

		uint32_t hash = get<uint32_t>(mainHash, mid*6);
		//long hash = ((mainHash[mid*6]&0xff) << 24) | ((mainHash[mid*6+1]&0xff) << 16) | ((mainHash[mid*6+2]&0xff) << 8) | (mainHash[mid*6+3] & 0xff);
		//hash &= 0xffffffffL;

		//Log.d(TAG, "offs %x, hash %08x", mid, hash);
		if (key < hash) {
			upto = mid;     // repeat search in bottom half.
		} else if (key > hash) {
			first = mid + 1;  // Repeat search in top half.
		} else {
			//found = mid;
			//int len = ((mainHash[mid*6+4]&0xff)<<8) | (mainHash[mid*6+5]&0xff);
			uint16_t len = get<uint16_t>(mainHash, mid*6+4);
			LOGD("LEN: %x", len);
			if((len & 0x8000) != 0) {
				len &= 0x7fff;
				int xl = 0;
				while((xl & 0x8000) == 0) {
					xl = extraLengths[len++];
					songLengths.push_back(xl & 0x7fff);
				}
				
				//for(int i=0; i<n; i++) {
				//	Log.d(TAG, "LEN: %02d:%02d", songLengths[i]/60, songLengths[i]%60);
				//}
			} else {
				LOGD("SINGLE LEN: %02d:%02d", len/60, len%60);
				songLengths.push_back(len);
			}
			break;
		}
	}
	return songLengths;
}

}