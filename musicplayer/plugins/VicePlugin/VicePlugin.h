#ifndef VICE_PLAYER_H
#define VICE_PLAYER_H

#include "../../ChipPlugin.h"

#include <string>
#include <vector>

namespace chipmachine {

class VicePlugin : public ChipPlugin {
public:
	virtual std::string name() const override { return "VicePlugin"; }
	VicePlugin(const std::string &dataDir);
	VicePlugin(const unsigned char *data);
	virtual ~VicePlugin();
	virtual bool canHandle(const std::string &name) override;
	virtual ChipPlayer *fromFile(const std::string &fileName) override;

	static void readLengths();
	static std::vector<uint16_t> findLengths(uint32_t key);

private:
	static std::vector<uint8_t> mainHash;
	static std::vector<uint16_t> extraLengths;
};

}

#endif // VICE_PLAYER_H