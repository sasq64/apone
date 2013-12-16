
#include "ModPlugin.h"
#include "VicePlugin.h"
#include "ChipPlugin.h"
#include "ChipPlayer.h"

#include "Fifo.h"

#include <grappix/grappix.h>
#include <SDL/SDL.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace utils;
using namespace std;
using namespace grappix;

static const int bufSize = 8192;
static double percent = 50;

#ifdef EMSCRIPTEN
static const char *itoa(int l) {
	static char t[12];
	sprintf(t, "%d", l);
	return t;
}
#endif

struct App {

	Texture sprite;
	vec2f xy;
	int xpos;
	Texture scr;
	GLuint program;
	float tstart;
	ModPlugin *modPlugin;
	ChipPlayer *player;
 	SDL_AudioSpec wanted;
	TileSet font;
	TileLayer tiles;
#ifdef EMSCRIPTEN
	worker_handle worker;
#else
	VicePlugin *vicePlugin;
#endif
	int asciiToCbm[256];
	Fifo fifo;
	//int inProgress;
	bool delay;


	App() : sprite {64, 64}, xy {0, 0}, xpos {-9999}, scr {screen.width()+200, 400}, tstart {0}, player(nullptr), font(8,8), tiles(40,25,640,400, font), fifo(512*1024), delay(false) { //inProgress(9999999) {

		for(int i=0; i<256; i++) {
			if(i < 0x20)
				asciiToCbm[i] = 0;
			else if(i <= '?')
				asciiToCbm[i] = i;
			else if(i == '@')
				asciiToCbm[i] = 0;
			else if(i <= 'Z')
				asciiToCbm[i] = i - 'A' + 65;
			else if(i <= 'z')
				asciiToCbm[i] = i - 'a' + 1;
			else if(i <= 97)
				asciiToCbm[i] = 0;
			else
				asciiToCbm[i] = 0;
		}


		//emscripten_async_wget("C64Music/MUSICIANS/H/Hubbard_Rob/ACE_II.sid", "Ace.sid",
		//	onLoad, onError);
		//emscripten_async_wget_data("http://swimsuitboys.com/droidsound/dl/C64Music/MUSICIANS/H/Hubbard_Rob/ACE_II.sid", (void*)"Ace.sid",
		//	onLoad, onError);

		//modPlugin = new ModPlugin();
		//player = modPlugin->fromFile("data/test.mod");
		//vicePlugin = make_unique<VicePlugin>(std::string("data/c64"));
		//vicePlugin = new VicePlugin("data/c64");
		//player = vicePlugin->fromFile("data/test.sid");
		//player = modPlugin->fromFile("data/test.mod");
#ifdef EMSCRIPTEN
		worker = emscripten_create_worker("viceplugin.js");
		char data[8192+8192+4096];
		FILE *fp = fopen("data/c64/kernal", "rb");
		fread(&data[0], 1, 8192, fp);
		fclose(fp);
		fp = fopen("data/c64/basic", "rb");
		fread(&data[8192], 1, 8192, fp);
		fclose(fp);
		fp = fopen("data/c64/chargen", "rb");
		fread(&data[8192*2], 1, 4096, fp);
		fclose(fp);
		emscripten_call_worker(worker, "init_sid", data, sizeof(data), init_callback, this);
#else
		vicePlugin = new VicePlugin("data/c64");
		//player = vicePlugin->fromFile("data/test.sid");
#endif
		//LOGD("Player is %p", player);
		auto bm = load_png("data/c64.png");
		fflush(stdout);
		font.add_tiles(bm);
		for(int i=0; i<40*25; i++)
			tiles[i] = 0x20;

	    // Set the audio format
	    wanted.freq = 44100;
	    wanted.format = AUDIO_S16;
	    wanted.channels = 2;    // 1 = mono, 2 = stereo
	    wanted.samples = (bufSize/2);
	    wanted.callback = App::fill_audio;
	    wanted.userdata = this;

	    // Open the audio device, forcing the desired format
	    if(SDL_OpenAudio(&wanted, NULL) < 0 ) {
	        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	        exit(0);
	    }

#ifdef EMSCRIPTEN
//		emscripten_async_wget2("C64Music/MUSICIANS/F/Fanta/Natural_Wonders_2.sid", "Ace.sid", "GET", nullptr, (void*)this,
//			App::onLoad, App::onError, nullptr);
#else
		player = vicePlugin->fromFile("html/C64Music/MUSICIANS/F/Fanta/Natural_Wonders_2.sid");
		print(player->getMeta("title"), 0, 0);
		print(player->getMeta("composer"), 0, 1);
		print(player->getMeta("copyright"), 0, 2);
#endif
	    SDL_PauseAudio(0);

	}

	bool audio = true;

	void update() {
		screen.clear();
		print(format("%03d%", (int)percent), 36, 0);
		tiles.render(screen);
		screen.flip();
		if(screen.get_key() == Window::ENTER || screen.get_key() == Window::SPACE) {
			SDL_PauseAudio(audio ? 1 : 0);
			audio = !audio;
		}
	}

	void print(const std::string &text, int x, int y) {
		int i = x + y * tiles.width();
		for(const auto &c : text) {
			tiles[i++] = asciiToCbm[c & 0xff];
		}
	}

	static void onLoad(void *arg, const char *name);
	static void onError(void *arg, int code);
	static void fill_audio(void *udata, Uint8 *stream, int len);
	static void init_callback(char *data, int size, void *arg);
	static void load_callback(char *data, int size, void *arg);
	static void play_callback(char *data, int size, void *arg);
};

#ifdef EMSCRIPTEN
void App::init_callback(char *data, int size, void *arg) {
	LOGD("Init result %s", data);
	emscripten_async_wget2("C64Music/MUSICIANS/F/Fanta/Natural_Wonders_2.sid", "Ace.sid", "GET", nullptr, (void*)arg,
		App::onLoad, App::onError, nullptr);
}

void App::load_callback(char *data, int size, void *arg) {
	LOGD("Loaded %s", data);
	App *app = static_cast<App*>(arg);
	//char t[12];
	//sprintf(t, "%d", app->fifo.size()/2);
	const char *t = itoa(app->fifo.size()/2);
	emscripten_call_worker(app->worker, "play_sid_song", t, strlen(t)+1, play_callback, app);
	//app->inProgress = 0;//app->fifo.size()/2;
}

void App::play_callback(char *data, int size, void *arg) {
	App *app = static_cast<App*>(arg);
	LOGD("Got %d bytes samples", size);
	app->fifo.putBytes(data, size);

	int left = app->fifo.size() - app->fifo.filled();
	if(left > 128*1024) {
		const char *t = itoa(128*1024);
		emscripten_call_worker(app->worker, "play_sid_song", t, strlen(t)+1, play_callback, app);
	} else {
		app->delay = true;
		LOGD("Buffer full");
	}
}

void App::onLoad(void *arg, const char *name) {
	LOGD("### Got %s", name);
	App *app = (App*)arg;
	/*if(app->player)
		delete app->player;
	app->player = app->vicePlugin->fromFile(name);
	app->print(app->player->getMeta("title"), 0, 0);
	app->print(app->player->getMeta("composer"), 0, 1);
	app->print(app->player->getMeta("copyright"), 0, 2);*/
	char temp[64*1024];
	FILE *fp = fopen(name, "rb");
	int size = fread(temp, 1, sizeof(temp), fp);
	fclose(fp);

	LOGD("Read %d bytes sid", size);

	emscripten_call_worker(app->worker, "load_sid_song", temp, size, load_callback, app);
}

void App::onError(void *arg, int code) {
	LOGD("Failed");
}

#endif

void App::fill_audio(void *udata, Uint8 *stream, int len) {
	App *app = static_cast<App*>(udata);
#ifdef EMSCRIPTEN
	if(app->fifo.filled() > len)
		app->fifo.getBytes((char*)stream, len);
	else {
		memset(stream, 0, len);
	}

	int left = app->fifo.size() - app->fifo.filled();
	LOGD("Filled %d", app->fifo.filled());
	if(app->delay && left > 128*1024) {
		const char *t = itoa(128*1024);
		emscripten_call_worker(app->worker, "play_sid_song", t, strlen(t)+1, play_callback, app);
		app->delay = false;
	}

#else
	static vector<int16_t> buffer(bufSize);
	//float now = emscripten_get_now();
	int rc = 0;
	if(app->player) {
		auto now = getms();//emscripten_get_now();
		rc = app->player->getSamples(&buffer[0], len/2);
		auto t = getms() - now;
		double p = (t * 100) / (len / 4.0 / 44.1);
		percent = percent * 0.95 + p * 0.05;
		//LOGD("Sound CPU %dms for %d samples ie %d%%", t, len, (t * 100) / (len / 4.0 / 44.1));  
		memcpy(stream, &buffer[0], rc*2);
	} else {
		memset(stream, 0, len);
	}
	//float t = emscripten_get_now() - now;
	//LOGD("Sound CPU %fms for %d samples ie %fms", t, len, len / 4.0 / 44.1);  
	//LOGD("Returning");
#endif
}

void runMainLoop() {
	static App app;
	app.update();
}

int main() {
	LOGD("main");	
	screen.open(640, 400, false);
	LOGD("Screen is open");

	screen.renderLoop(runMainLoop);
	return 0;
}