#include "audioplayer.h"

#ifdef SDL_AUDIO
#include "player_sdl.h"
#elif defined _WIN32
#include "player_win.h"
#elif defined LINUX
#include "player_linux.h"
#elif defined ANDROID
#include "player_sl.h"
#else
#include "player_sdl.h"
#endif

std::shared_ptr<InternalPlayer> AudioPlayer::staticInternalPlayer;

AudioPlayer::AudioPlayer(int hz) : internalPlayer(std::make_shared<InternalPlayer>(hz)) {}
AudioPlayer::AudioPlayer(std::function<void(int16_t *, int)> cb, int hz) : internalPlayer(std::make_shared<InternalPlayer>(cb, hz)) {
	staticInternalPlayer = internalPlayer;
}

void AudioPlayer::play(std::function<void(int16_t *, int)> cb, int hz) {
	staticInternalPlayer = std::make_shared<InternalPlayer>(cb, hz);
}

void AudioPlayer::close() {
	staticInternalPlayer = nullptr;
}

void AudioPlayer::pause() { internalPlayer->pause(true); }
void AudioPlayer::resume() { internalPlayer->pause(false); }

void AudioPlayer::pause_audio() { 
	if(staticInternalPlayer)
		staticInternalPlayer->pause(true);
}

void AudioPlayer::set_volume(int v) {
#ifdef LINUX
	if(staticInternalPlayer)
		staticInternalPlayer->set_volume(v);
#endif
}

void AudioPlayer::resume_audio() {
	if(staticInternalPlayer)
		staticInternalPlayer->pause(false);
}

int AudioPlayer::get_delay() {
	if(staticInternalPlayer)
		return staticInternalPlayer->get_delay();
	return 0;
}
