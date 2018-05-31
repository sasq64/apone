#include <string>
#include <vector>
#include <optional>

#include "utils.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

inline std::string makeSearchPath(std::vector<fs::path> paths, bool resolve) {
    std::string searchPath = "";
    std::string sep = "";
	for(const auto& p : paths) {
		if(!resolve || fs::exists(p)) {
			searchPath = searchPath + sep + (resolve ? fs::canonical(p) : p).string();
			sep = ";";
		}
	}
	return searchPath;

}

inline std::optional<fs::path> findFile(const std::string& searchPath, const std::string &name) {
	//LOGD("Find '%s'", name);
	if(name == "")
		return {};
	auto parts = utils::split(searchPath, ";");
	for(fs::path p : parts) {
		if(!p.empty()) {
			//LOGD("...in path %s", p);
			fs::path f { p / name };
			if(fs::exists(f))
				return f;
		}
	}
	return {};
}


