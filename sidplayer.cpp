
//#include "ModPlugin.h"
#include "VicePlugin.h"
#include "ChipPlugin.h"
#include "ChipPlayer.h"

#include "SongDb.h"

//#include "Fifo.h"
#include <mutex>
#include <webutils/webgetter.h>
#include <grappix/grappix.h>
#include <SDL/SDL.h>
#include <algorithm>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace utils;
using namespace std;
using namespace grappix;
using namespace chipmachine;

static const int bufSize = 32768;
static double percent = 20;

string setSearchString;
bool newSS = false;

int playIndex = -1;
extern "C" {

void play_index(int i) {
	playIndex = i;
}

void set_searchstring(char *s) {
	setSearchString = s;
	newSS = true;
}

}

//extern "C" {
//extern unsigned int opcode_stats[256];
//};

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
	//ModPlugin *modPlugin;
	ChipPlayer *player;
 	SDL_AudioSpec wanted;
	TileSet font;
	TileLayer tiles;
	unique_ptr<VicePlugin> vicePlugin;
	int asciiToCbm[256];
	//Fifo fifo;
	WebGetter webGetter;
	unique_ptr<WebGetter::Job> sidJob;
	SongDatabase db;
	IncrementalQuery query;

	~App() {
		SDL_PauseAudio(1);
		int i = 0;
		//for(auto &r : opcode_stats)
		//	r = (r << 8) | i++;
		//std::sort(begin(opcode_stats), end(opcode_stats));
		//i = 0;
		//for(const auto &r : opcode_stats)
		//	printf("%02x : %d\n", r&0xff, r>>8);
	}

	App() : sprite {64, 64}, xy {0, 0}, xpos {-9999}, scr {screen.width()+200, 400}, tstart {0}, player(nullptr), font(8,8), tiles(40,25,16*40,16*25, font),
	/*fifo(512*1024),*/ webGetter("files"), db { "data/hvsc.db" } { //inProgress(9999999) {

		db.generateIndex();
#ifndef EMSCRIPTEN
		webGetter.setBaseURL("http://swimsuitboys.com/hvsc/");
#endif

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

		vicePlugin = make_unique<VicePlugin>("data/c64");

		auto bm = load_png("data/c64/c64.png");
		for(int i=0; i<bm.size(); i++) {
			if(bm[i])
				bm[i] = 0xffb55E6C;
		}
		fflush(stdout);
		font.add_tiles(bm);
		for(int i=0; i<tiles.size(); i++)
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
	    SDL_PauseAudio(0);
		//player = vicePlugin->fromFile("data/test.sid");
	    sidJob = unique_ptr<WebGetter::Job>(webGetter.getURL("C64Music/MUSICIANS/G/Galway_Martin/Ocean_Loader_2.sid"));
	   	query = db.find();

		print("@", 0, 4);

	}

	mutex m;
	bool audio = true;
	int cursorx = 1;
	int marker = -1;
	int lastMarker = -1;
	int toPlay = -1;
	int currentSong = -1;
	int maxSongs = 0;
	int lastSong = -1;
	int scrollpos = 0;
	int delay = 0;

	void update(int d) {

		if(newSS) {
			query.setString(setSearchString);
			newSS = false;
		}

		if(playIndex >= 0) {
			toPlay = playIndex;
			playIndex = -1;
		}

		if(sidJob && sidJob->isDone()) {
		    //SDL_PauseAudio(1);
			tiles.fill(0x20, 0, 0, 0, 3);
#ifndef EMSCRIPTEN
			m.lock();
#endif
			if(player)
				delete player;
			player = vicePlugin->fromFile(sidJob->getFile());
			print(player->getMeta("title"), 0, 0);
			print(player->getMeta("composer"), 0, 1);
			print(player->getMeta("copyright"), 0, 2);
			maxSongs = player->getMetaInt("songs");
			lastSong = -1;
			currentSong = player->getMetaInt("startsong");
#ifndef EMSCRIPTEN
			m.unlock();
#endif
			//SDL_PauseAudio(0);
		    //delete sidJob;
		    sidJob = nullptr;
		}

		auto k = screen.get_key();
		bool updateResult = query.newResult();
		if(updateResult) {
			marker = scrollpos = 0;
		}

		if(delay) {
			if(delay == 1) {
				if(screen.key_pressed(Window::UP))
					marker--;
				else if(screen.key_pressed(Window::DOWN))
					marker++;
				else
					delay = 0;
			}
			else delay--;
		} 


		if(k != Window::NO_KEY) {
			LOGD("Pressed 0x%02x", k);
			switch(k) {
			case 0x08:
			case Window::BACKSPACE:
				query.removeLast();
				updateResult = true;
				break;
			case Window::ESCAPE:
			case Window::F1:
			case 255:
				query.clear();
				updateResult = true;
				break;
			case Window::UP:
				if(!delay) {
					marker--;
					delay = 4;
				}
				break;
			case Window::DOWN:
				if(!delay) {
					marker++;
					delay = 4;
				}
				break;
			case Window::PAGEUP:
				marker-=(tiles.height()-5);
				break;
			case Window::PAGEDOWN:
				marker+=(tiles.height()-5);
				break;
			case Window::LEFT:
				if(currentSong > 0)
					player->seekTo(--currentSong);
				break;
			case Window::RIGHT:
				if(currentSong+1 < maxSongs)
					player->seekTo(++currentSong);
				break;
			case Window::ENTER:
			case 0x0d:
			case 0x0a:
				toPlay = marker;
				break;
			default:
				break;
			}

			if(k < 0x100) {
				if(isalnum(k) || k == ' ') {
					query.addLetter(tolower(k));
					updateResult = true;
				}
			}
			//if(k == Window::ENTER || k == Window::SPACE) {
			//	SDL_PauseAudio(audio ? 1 : 0);
			//	audio = !audio;
			//}
		}
		int h = tiles.height()-5;

		if(marker < 0)
			marker = 0;
		if(marker >= query.numHits())
			marker = query.numHits()-1;
		//if(marker >= tiles.height())
		//	marker = tiles.height()-1;

		while(marker >= scrollpos + h) {
			scrollpos++;
			updateResult = true;
		}
		while(marker > 0  && marker < scrollpos) {
			scrollpos--;
			updateResult = true;
		}

		if(updateResult) {
			tiles.fill(0x20, 1, 4, 0, 1);
			print(query.getString(), 1, 4);
			tiles.fill(0X20, 1, 5);
			const auto &results = query.getResult(scrollpos, h);
			LOGD("%d %d -> %d", scrollpos, h, results.size());
			int i=0;
			for(const auto &r : results) {
				auto p = split(r, "\t");
				if(p.size() < 3) {
					LOGD("Illegal result line '%s' -> [%s]", r, p);
				} else {
					int index = atoi(p[2].c_str());
					int fmt = db.getFormat(index);
					//int color = Console::WHITE;
					print(format("%s / %s", p[0], p[1]), 1, i+5);
					//console.put(1, i+3, utf8_encode(format("%s - %s", p[1], p[0])), color);
				}
				i++;
				//if(i >= h-3)
				//	break;
			}
		}

		if(currentSong != lastSong) {
			print(format("SONG %02d/%02d", currentSong+1, maxSongs), tiles.width()-10, 0);
			lastSong = currentSong;
		}

		if(toPlay >= 0) {
			string r = query.getFull(toPlay);
			toPlay = -1;
			LOGD("RESULT: %s", r);
			auto p  = utils::split(r, "\t");
			for(size_t i = 0; i<p[2].length(); i++) {
				if(p[2][i] == '\\')
					p[2][i] = '/';
			}
			LOGD("Playing '%s'", p[2]);
			sidJob = unique_ptr<WebGetter::Job>(webGetter.getURL(p[2])); //  + 
		}


		if(marker != lastMarker) {
			if(lastMarker >= 0)
				print(" ", 0, lastMarker);
			if(marker >=0 )
				print(">", 0, marker+5-scrollpos);
			lastMarker = marker+5-scrollpos;
		}

		screen.clear(0x352879);
		print(format("%03d%", (int)percent), 36, 2);
		tiles.render(screen);
		screen.flip();
	}

	void print(const std::string &text, int x, int y) {
		int i = x + y * tiles.width();
		int maxi = (y+1) * tiles.width();
		int total = tiles.width() * tiles.height();
		for(const auto &c : text) {
			if(i >= maxi || i >= total)
				break;
			tiles[i++] = asciiToCbm[c & 0xff];
		}
	}

	static void onLoad(void *arg, const char *name);
	static void onError(void *arg, int code);
	static void fill_audio(void *udata, Uint8 *stream, int len);
};

void App::fill_audio(void *udata, Uint8 *stream, int len) {
	App *app = static_cast<App*>(udata);
	//static vector<int16_t> buffer(bufSize);
	//float now = emscripten_get_now();
	int rc = 0;
	if(app->player) {
#ifndef EMSCRIPTEN
		app->m.lock();
#endif
		auto now = getms();//emscripten_get_now();
		rc = app->player->getSamples((int16_t*)stream, len/2);
		auto t = getms() - now;
#ifndef EMSCRIPTEN
		app->m.unlock();
#endif
		double p = (t * 100) / (len / 4.0 / 44.1);
		percent = percent * 0.8 + p * 0.2;
		//LOGD("Sound CPU %dms for %d samples ie %d%%", t, len, (t * 100) / (len / 4.0 / 44.1));  
		//memcpy(stream, &buffer[0], rc*2);
	} else {
		memset(stream, 0, len);
	}
}

void runMainLoop(int d) {
	static App app;
	app.update(d);
}

int main() {
	LOGD("main");	
	screen.open(640, 400, false);
	LOGD("Screen is open");

	screen.render_loop(runMainLoop, 20);
	return 0;
}