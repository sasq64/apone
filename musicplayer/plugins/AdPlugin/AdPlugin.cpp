
#include "AdPlugin.h"

#include <string>
#include <stdio.h>
#include <math.h>


#include "adplug/emuopl.h"
#include "adplug/kemuopl.h"
#include "opl/dbemuopl.h"
#include "adplug/temuopl.h"
#include "adplug/adplug.h"
#include "libbinio/binfile.h"
#include "libbinio/binio.h"
#include "resampler.h"
#include "../../chipplayer.h"

#include <coreutils/utils.h>
#include <set>
#include <unordered_map>

extern "C" {
}

using namespace std;

namespace chipmachine {

#define INFO_TITLE 0
#define INFO_AUTHOR 1
#define INFO_LENGTH 2
#define INFO_TYPE 3
#define INFO_COPYRIGHT 4
#define INFO_GAME 5
#define INFO_SUBTUNES 6
#define INFO_STARTTUNE 7 

#define INFO_INSTRUMENTS 100
#define INFO_CHANNELS 101
#define INFO_PATTERNS 102  


class AdPlugPlayer : public ChipPlayer {

	CEmuopl *  p_cemu = nullptr;
	CKemuopl * p_kemu = nullptr;
	DBemuopl * p_demu = nullptr;
	CPlayer * m_player = nullptr;
	bool is_cemu = false;
	bool is_kemu = false;
	bool is_demu = false;
	/* STATIC! */ CAdPlugDatabase * g_database = nullptr; 
	void * resampler;
	int samples;

public:
	//load(jstring fname, jstring dbname, jint core, jint sample_rate, jint stereo)
	AdPlugPlayer(const std::string &fileName) {

		int core = 0;
		int sample_rate = 44100;

		//const char * filename = env->GetStringUTFChars(fname, nullptr);
		//const char * db_filename = env->GetStringUTFChars(dbname, nullptr);
			
		if(!g_database) {
			binistream *fp  = new binifstream("data/adplug.db");
			fp->setFlag(binio::BigEndian, false);
			fp->setFlag(binio::FloatIEEE);
				
			g_database = new CAdPlugDatabase();
			g_database->load( *fp );
			delete fp;
			CAdPlug::set_database( g_database );
		}

		samples = 0;

		resampler_init();
		resampler = resampler_create();
		resampler_set_rate(resampler, 49716. / (float)sample_rate);
		resampler_clear(resampler);
			
		is_cemu = is_kemu = is_demu = false;
			
		switch (core) {
		case 2:
			p_cemu = new CEmuopl( 49716, true, true );
			m_player = CAdPlug::factory(fileName.c_str(), p_cemu, CAdPlug::players );
			is_cemu = true;
			break;
		case 1:
			p_kemu = new CKemuopl( 49716, true, true );
			m_player = CAdPlug::factory(fileName.c_str(), p_kemu, CAdPlug::players );
			is_kemu = true;
			break;

		case 0:
			p_demu = new DBemuopl( 49716, true );
			m_player = CAdPlug::factory(fileName.c_str(), p_demu, CAdPlug::players );
			is_demu = true;
			break;
		}
	}

	//JNIEXPORT void JNICALL Java_com_ssb_droidsound_plugins_AdPlugin_N_1unload(JNIEnv *env, jobject obj, jlong song)
	~AdPlugPlayer() {

		if(m_player)
			delete m_player;
		if(p_cemu)
			delete p_cemu;
		if(p_kemu)
			delete p_kemu;
		if(p_demu)
			delete p_demu;
		p_cemu = nullptr;
		p_kemu = nullptr;
		p_demu = nullptr;
		m_player = nullptr;
		if(resampler)
			resampler_delete(resampler);
		resampler = nullptr;
	}


	//JNIEXPORT jint JNICALL Java_com_ssb_droidsound_plugins_AdPlugin_N_1getSoundData(JNIEnv *env, jobject obj, jlong song, jshortArray sArray, jint size)
	virtual int getSamples(int16_t *target, int noSamples) override {

		unsigned int chunk_size = 0;
		bool is_playing;
		int free_count = 0;
		unsigned int sample = 0;
		int resampled_sample = 0;
		int samples_written = 0;
		int refresh_rate = 0;

		//jshort *target = env->GetShortArrayElements(sArray, nullptr);	
		short temp_buffer[128];
		memset(temp_buffer,0,256);
		
		if (!samples)
		{
			is_playing = m_player->update();
			//if (!is_playing)
			//	return -1;

			refresh_rate = (int)m_player->getrefresh();
			fprintf(stderr, "%d\n", refresh_rate);
			samples = noSamples;
			if (refresh_rate) // divide by zero gives sigfpe exception
				samples = noSamples / refresh_rate;
		}

		chunk_size = samples;
		if (chunk_size > noSamples / 2)
			chunk_size = noSamples / 2;

		while (chunk_size)
		{ 
			free_count = resampler_get_free_count(resampler);
			if (free_count)
			{
				if (is_cemu)
					p_cemu->update(temp_buffer, free_count);
				else if (is_kemu)
					p_kemu->update(temp_buffer, free_count);
				else if (is_demu)
					p_demu->update(temp_buffer, free_count);
							
				for ( unsigned i = 0; i < free_count; i++ )
				{
					sample = 0;
					sample = temp_buffer[i*2];
					//sample |= (temp_buffer[i*2 + 1] << 16 );
					resampler_write_sample(resampler, sample);
				}
			}
			resampled_sample = resampler_get_sample(resampler);
			resampler_remove_sample(resampler);

			target[samples_written++] = (short)resampled_sample;
			target[samples_written++] = (short)resampled_sample;
				
			chunk_size--;
		}
		samples -= samples_written/2;
		
		return samples_written;
	}
};

static const set<string> supported_ext {
		"a2m", "adl", "amd", "bam", "cff", "cmf", "d00", "dfm", "dmo", "dro", "dtm", 
		"hcs", "hsp", "imf", "ksm", "laa", "lds", "m",   "mad", "mid", "mkj", 
		"msc", "mtk", "rad", "raw", "rix", "rol", "as3m", "sa2", "sat", "sci", 
		"sng", "xad", "xms", "xsm" };

bool AdPlugin::canHandle(const std::string &name) {
	return supported_ext.count(utils::path_extension(name)) > 0;
}

ChipPlayer *AdPlugin::fromFile(const std::string &fileName) {
	return new AdPlugPlayer { fileName };
};

}

/*
JNIEXPORT jstring JNICALL Java_com_ssb_droidsound_plugins_AdPlugin_N_1getStringInfo(JNIEnv *env, jobject obj, jlong song, jint what)
{
	switch (what)
	{
		case INFO_AUTHOR:
			return NewString(env,m_player->getauthor().c_str());
		case INFO_TITLE:
			return NewString(env,m_player->gettitle().c_str());
	}
	return NewString(env,"");
}
JNIEXPORT jint JNICALL Java_com_ssb_droidsound_plugins_AdPlugin_N_1getIntInfo(JNIEnv *env, jobject obj, jlong song, jint what)
{	
	switch(what)
	{

		case INFO_LENGTH:
			return m_player->songlength();
		case INFO_SUBTUNES:
			return m_player->getsubsongs();
		case INFO_STARTTUNE:
			return 0;
		case INFO_CHANNELS:
			return 9;
		case INFO_PATTERNS:
			return m_player->getpatterns();
		
	}
	return -1; 

}
*/