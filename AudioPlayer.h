#include <functional>
#include <memory>
#include <stdexcept>
#include <coreutils/log.h>
#include <SDL/SDL.h>



static const int bufSize = 65536;

class audio_exception : public std::exception {
public:
	audio_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

class AudioPlayer {
private:
	class InternalPlayer {
	public:
		InternalPlayer(std::function<void(uint16_t *, int)> cb) : callback(cb) {
			LOGD("Opening audio");
			SDL_AudioSpec wanted = { 44100, AUDIO_S16, 2, 0, bufSize/2, 0, 0, fill_audio, this };
		    if(SDL_OpenAudio(&wanted, NULL) < 0 ) {
		        //fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		        throw audio_exception("Could not open audio");
		    } else
				SDL_PauseAudio(0);
		}
		~InternalPlayer() {
			LOGD("Destroying InternalPlayer");
		}
		static void fill_audio(void *udata, Uint8 *stream, int len) {	
			//static vector<int16_t> buffer(bufSize);
			auto player = static_cast<InternalPlayer*>(udata);
			player->callback((uint16_t*)stream, len/2);
		}
		std::function<void(uint16_t *, int)> callback;
	};
public:
	AudioPlayer(std::function<void(uint16_t *, int)> cb) : internalPlayer(std::make_shared<InternalPlayer>(cb)) {
		if(!globalInternalPlayer)
			globalInternalPlayer = internalPlayer;
	}
	~AudioPlayer() {
		LOGD("Destroying AudioPlayer");
	}
private:
	std::shared_ptr<InternalPlayer> internalPlayer;
	static std::shared_ptr<InternalPlayer> globalInternalPlayer;

};


