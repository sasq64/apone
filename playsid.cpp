
//#include "ModPlugin.h"
#include "VicePlugin.h"
#include "ChipPlugin.h"
#include "ChipPlayer.h"

#include <coreutils/utils.h>
#include <SDL/SDL.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace utils;
using namespace std;

#include "c64dat.cpp"

static const int bufSize = 32768;
static ChipPlayer *player = nullptr;
static SDL_AudioSpec wanted;
static unique_ptr<VicePlugin> vicePlugin;
static bool audioOn = true;
static int currentSong = -1;
static int maxSongs = 0;
static int changeToSong = -1;
static int doneCallback;
static int progressCallback;
static string target;

static void fill_audio(void *udata, Uint8 *stream, int len) {
	if(changeToSong >= 0) {
		if(player) {
			player->seekTo(changeToSong);
		}
		changeToSong = -1;
	}
	if(audioOn && player) {
		player->getSamples((int16_t*)stream, len/2);
	} else {
		memset(stream, 0, len);
	}
}

static void on_error(void *arg, int code) {
}

static void on_load(void *arg, const char *fileName) {
	LOGD("onLoad '%s'", fileName);
	if(player)
		delete player;
	player = vicePlugin->fromFile(fileName);
	auto title = player->getMeta("title");
	auto composer = player->getMeta("composer");
	auto copyright = player->getMeta("copyright");
	maxSongs = player->getMetaInt("songs");
	currentSong = player->getMetaInt("startsong");
	if(doneCallback > 0) {
		auto f = reinterpret_cast<void (*)(const char*, const char*, const char*, int, int)>(doneCallback);
		f(title.c_str(), composer.c_str(), copyright.c_str(), maxSongs, currentSong);
	}
}

extern "C" {

void playsid_pause(int pause) {
	audioOn = (pause == 0);
}

void playsid_setsong(int song) {
	changeToSong = song;
}

void playsid_play(const char *url, int done_cb, int progress_cb) {

	doneCallback = done_cb;
	progressCallback = progress_cb;

	audioOn = true;

	LOGD("Done cb %d", done_cb);

	if(!vicePlugin) {

		mkdir("files", 0777);
		vicePlugin  = make_unique<VicePlugin>(c64_dat);

	    // Set the audio format
	   	SDL_AudioSpec wanted = { 44100, AUDIO_S16, 2, 0, bufSize/2, 0, 0, fill_audio, nullptr };
		// Open the audio device, forcing the desired format
	    if(SDL_OpenAudio(&wanted, NULL) < 0 ) {
	        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	        exit(0);
	    }
	    SDL_PauseAudio(0);
	}

	target = "files/" + urlencode(url, ":/\\?;");
	currentSong = -1;
	maxSongs = 0;
	if(!File::exists(target)) {
		LOGD("Geting '%s' to '%s'", url, target);
		emscripten_async_wget2(url, target.c_str(), "GET", nullptr, nullptr, on_load, on_error, nullptr);
	} else {
		LOGD("Getting %s from cache", target);
		on_load(nullptr, target.c_str());
	}
}

}
