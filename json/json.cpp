#include "json.h"

void json_parse(std::string &s) {
	json_object *jobj = json_tokener_parse(s.c_str());

}
