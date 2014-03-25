
#include "MusicPlayer.h"

namespace grappix {

DelegatingChipPlayer MusicPlayer::fromFile(const std::string &fileName) {
	static chipmachine::ModPlugin modPlugin;
	auto chipPlayer = modPlugin.fromFile(fileName);
	return DelegatingChipPlayer(chipPlayer);
}

}