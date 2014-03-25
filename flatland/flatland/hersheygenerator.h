#ifndef HERSHEYGENERATOR_H_
#define HERSHEYGENERATOR_H_

#include "shape.h"
#include <map>

namespace flatland {

class HersheyGenerator {
public:
	static const Shape createText(const char *ascii);
	static const Shape createLetter(int code);
	static bool exists(int code);
	static void init();
private:
	static std::map<int, signed char *> offsets;
	static std::vector<short> ascii;

};

} /* namespace flatland */
#endif /* HERSHEYGENERATOR_H_ */
