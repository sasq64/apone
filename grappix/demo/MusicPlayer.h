#include <ModPlugin/ModPlugin.h>
#include <memory>
namespace grappix {

class DelegatingChipPlayer : public chipmachine::ChipPlayer {
public:
	DelegatingChipPlayer(chipmachine::ChipPlayer *cp) : player(cp) {}
	~DelegatingChipPlayer() {}

	int getSamples(int16_t *target, int size) override {
		return player->getSamples(target, size);
	}

private:
	std::shared_ptr<ChipPlayer> player;
};

class MusicPlayer {
public:
	static DelegatingChipPlayer fromFile(const std::string &fileName);
};

}