#ifndef JSON_H
#define JSON_H

#include "json-c/json.h"

#include <string>

class json_exception : public std::exception {
public:
	json_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
	std::string msg;
};

class JSon {
public:
	JSon(const std::string &s) : obj(json_tokener_parse(s.c_str())), jtype(json_object_get_type(obj)) {
	}

	JSon(json_object *obj) : obj(obj), jtype(json_object_get_type(obj)) {}

	size_t size() {
		if(jtype != json_type_array)
			return 0;
		return json_object_array_length(obj);
	}

	bool is_array() { return jtype == json_type_array; }

	JSon operator()(const std::string &key) const {
		LOGD("OBJECT %d", jtype);
		if(jtype != json_type_object)
			throw json_exception("Not an object");
		return JSon(json_object_object_get(obj, key.c_str()));
	}

	JSon operator()(const uint32_t index) const {
		LOGD("ARRAY %d", jtype);
		if(jtype != json_type_array)
			throw json_exception("Not an array");
		return JSon(json_object_array_get_idx(obj,index));
	}

	void foreach(std::function<void(JSon&&)> cb) {
		auto l = json_object_array_length(obj);
		for(int i=0; i<l; i++) {
			cb(JSon(json_object_array_get_idx(obj, i)));
		}
	}

	operator std::string() {
		if(jtype != json_type_string)
			throw json_exception("Not a string");
		return std::string(json_object_get_string(obj));
	}

	operator int64_t() {
		if(jtype != json_type_int)
			throw json_exception("Not an integer");
		return json_object_get_int(obj);
	} 

	operator double() {
		if(jtype != json_type_double)
			throw json_exception("Not a double");
		return json_object_get_double(obj);
	}

	json_object *obj;
	enum json_type jtype;
};


#endif // JSON_H
