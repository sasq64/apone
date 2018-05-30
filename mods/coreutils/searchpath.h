

string makeSearchPath(vector<fs::path> paths, bool resolve) {
	string searchPath = "";
	string sep = "";
	for(const auto& p : paths) {
		if(!resolve || fs::exists(p)) {
			searchPath = searchPath + sep + (resolve ? fs::canonical(p) : p).string();
			sep = ";";
		}
	}
	return searchPath;

}

std::optional<fs::path> findFile(const string& searchPath, const string &name) {
	LOGD("Find '%s'", name);
	if(name == "")
		return {};
	auto parts = split(searchPath, ";");
	for(fs::path p : parts) {
		if(!p.empty()) {
			LOGD("...in path %s", p);
			fs::path f { p / name };
			if(fs::exists(f))
				return f.parent_path();
		}
	}
	return {};
}


