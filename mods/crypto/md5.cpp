#include "md5.h"

MD5::MD5(int flags) : flags(flags) {
	MD5_Init(&ctx);
}

void MD5::add(std::vector<uint8_t> &data, int offset) {
	MD5_Update(&ctx, &data[offset], data.size() - offset);
}

std::vector<uint8_t> MD5::get() {
	std::vector<uint8_t> result(16);
	MD5_Final(&result[0], &ctx);
	return result;
}

uint64_t MD5::hash(const std::string &text) {
	MD5_CTX ctx;
	std::vector<uint8_t> result(16);
	MD5_Init(&ctx);
	MD5_Update(&ctx, text.c_str(), text.length());	
	MD5_Final(&result[0], &ctx);
	return *((uint64_t*)&result[8]);
}
