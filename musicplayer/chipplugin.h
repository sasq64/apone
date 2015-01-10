#ifndef CHIP_PLUGIN_H
#define CHIP_PLUGIN_H

#include <string>
#include <memory>
#include <vector>
#include <functional>

#include "chipplayer.h"

namespace chipmachine {

class ChipPlugin {
public:

	typedef std::function<std::shared_ptr<ChipPlugin>(const std::string &)> PluginConstructor;

	virtual ~ChipPlugin() {};

	virtual std::string name() const = 0; 

	virtual bool canHandle(const std::string &name) = 0;
	virtual ChipPlayer *fromFile(const std::string &fileName) = 0;
	virtual ChipPlayer *fromData(uint8_t *data, int size) {
		FILE *fp = fopen("tmpfile", "wb");
		fwrite(data, size, 1, fp);
		fclose(fp);
		return fromFile("tmpfile");
	}

	static void createPlugins(const std::string &configDir, std::vector<std::shared_ptr<ChipPlugin>> &plugins) {
		auto &functions = addAndGetPlugins();
		for(auto &f : functions) {
			plugins.push_back(f(configDir));
		}
	}

	struct RegisterMe {
		RegisterMe(PluginConstructor f) {
			ChipPlugin::addAndGetPlugins(&f);
		}
	};


private:
	static std::vector<PluginConstructor> &addAndGetPlugins(PluginConstructor *f = nullptr) {
		static std::vector<PluginConstructor> constructors;
		if(f)
			constructors.push_back(*f);
		return constructors;
	};

};


}

#endif // CHIP_PLUGIN_H