
class Translation {
	static Translation& getInstance() {
		static Translation r;
		return r;
	}

	Translation() {
	}

	std::string text(const std::string &lookup) {
		return texts[lookup];
	};

private:
	std::unordered_map<std::string, std::string> texts;
};


#define _T(x) Translation::getInstance().text(x)