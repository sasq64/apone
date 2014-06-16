
#include "OpenMPTPlugin.h"

#include "libopenmpt/libopenmpt.h"
#include "libopenmpt/libopenmpt_stream_callbacks_file.h"

#include "../../chipplayer.h"

#include <coreutils/utils.h>
#include <coreutils/file.h>
#include <set>
#include <unordered_map>

#ifdef EMSCRIPTEN
void srandom(unsigned int _seed)  { srand(_seed); }
long int random() { return rand(); }
#endif

using namespace std;

namespace chipmachine {

class OpenMPTPlayer : public ChipPlayer {
public:
	OpenMPTPlayer(uint8_t *data, int size) {

	mod = openmpt_module_create_from_memory(data, size, nullptr, nullptr, nullptr);

	if(!mod)
		throw player_exception("Could not load module");

	//if(loopmode)
	//	openmpt_module_set_repeat_count(mod, -1);

		auto length = openmpt_module_get_duration_seconds(mod);
		auto songs = openmpt_module_get_num_subsongs(mod);

		auto keys = openmpt_module_get_metadata_keys(mod);
		LOGD("META:%s", keys);

		auto title = openmpt_module_get_metadata(mod, "title");
		auto artist = openmpt_module_get_metadata(mod, "artist");
		auto tracker = openmpt_module_get_metadata(mod, "tracker");
		auto type = openmpt_module_get_metadata(mod, "type");
		auto message = openmpt_module_get_metadata(mod, "message");
		auto type_long = openmpt_module_get_metadata(mod, "type_long");

		if(strcmp(type, "mod") == 0)
			openmpt_module_set_render_param(mod, OPENMPT_MODULE_RENDER_INTERPOLATIONFILTER_LENGTH, 1);
		else
			openmpt_module_set_render_param(mod, OPENMPT_MODULE_RENDER_INTERPOLATIONFILTER_LENGTH, 0);

		auto p = utils::split(string(type_long), " / ");
		if(p.size() > 1)
			type_long = p[0].c_str();

		setMeta(
			"title", title,
			"composer", artist,
			"message", message,
			"tracker", tracker,
			"format", type_long,
			"type", type,
			"songs", songs,
			"length", length
			);

	}
	~OpenMPTPlayer() override {
		if(mod)
			openmpt_module_destroy(mod);
	}

	virtual int getSamples(int16_t *target, int noSamples) override {		
		auto len = openmpt_module_read_interleaved_stereo(mod, 44100, noSamples/2, target);
		return len*2;
	}

	virtual void seekTo(int song, int seconds) {
		if(mod) {
			if(song >= 0)
				openmpt_module_select_subsong(mod, song);
			else
				openmpt_module_set_position_seconds(mod, seconds);
		}
	}

private:
	openmpt_module *mod;
};

static const set<string> supported_ext { "mod", "xm", "s3m" , "oct", /*"okt", "okta", sucks here, use UADE */ "it", "ft", "far", "ult", "669", "dmf", "mdl", "stm", "okt", "gdm", "mt2", "mtm", "j2b", "imf", "ptm", "ams" };

bool OpenMPTPlugin::canHandle(const std::string &name) {
	auto ext = utils::path_extension(name);
	LOGD("EXT %s", openmpt_get_supported_extensions());
	return openmpt_is_extension_supported(ext.c_str());
	//return supported_ext.count(utils::path_extension(name)) > 0;
}

ChipPlayer *OpenMPTPlugin::fromFile(const std::string &fileName) {
	utils::File file { fileName };
	try {
		return new OpenMPTPlayer {file.getPtr(), file.getSize()};
	} catch(player_exception &e) {
		return nullptr;
	}
};

}

/*
JNIEXPORT jint JNICALL Java_com_ssb_droidsound_plugins_OpenMPTPlugin_N_1getIntInfo(JNIEnv *env, jobject obj, jlong song, jint what)
{
	openmpt_module* mod = (openmpt_module*)song;
	switch(what)
	{
		case INFO_LENGTH:
			return openmpt_module_get_duration_seconds(mod) * 1000;
		case INFO_SUBTUNES:
			return openmpt_module_get_num_subsongs(mod);
		case INFO_STARTTUNE:
			return 0;
		case INFO_CHANNELS:
			return openmpt_module_get_num_channels(mod);
		case INFO_PATTERNS:
			return openmpt_module_get_num_patterns(mod);
		
	}
	return -1; 
}
JNIEXPORT jstring JNICALL Java_com_ssb_droidsound_plugins_OpenMPTPlugin_N_1getStringInfo(JNIEnv *env, jobject obj, jlong song, jint what)
{
	openmpt_module* mod = (openmpt_module*)song;
	switch(what)
	{
		case INFO_INSTRUMENTS:
			{
						
			char instruments[2048];
			char *ptr = instruments;
			char *instEnd = instruments + sizeof(instruments) - 48;
			int pat = openmpt_module_get_num_patterns(mod);
			int ns = openmpt_module_get_num_samples(mod);
			memset(ptr, 0, 2048);
			if ((ns * 48) > 2048)
				return  NewString(env, instruments); 
			
			if(ns > 0)
			{
				for(int i=1; i<ns; i++)
				{
					const char* instr_name = openmpt_module_get_sample_name(mod, i);
					memcpy((void*)ptr, instr_name, strlen(instr_name));
					ptr += strlen(instr_name);
					if(ptr >= instEnd)
						break;
					*ptr++ = 0x0a;
					*ptr = 0;
				}
			}
			return  NewString(env, instruments); 
			}
		
		return NewString(env, ""); 
	
	}

}

*/