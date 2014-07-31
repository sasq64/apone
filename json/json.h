#ifndef JSON_H
#define JSON_H

#include "json-c/json.h"

#include <string>
#include <vector>

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

	JSon() {
		obj = json_object_new_object();
		jtype = json_type_object;
	}

	~JSon() {
		//if(obj)
		//	json_object_object_del(obj, nullptr);
	}

	JSon(json_object *obj) : obj(obj), jtype(json_object_get_type(obj)) {}

	static JSon parse(const std::string &s) {
		return JSon(s);
	}

	std::string to_string() {
		return json_object_to_json_string(obj);
	}

	size_t size() {
		if(jtype != json_type_array)
			return 0;
		return json_object_array_length(obj);
	}

	bool is_array() { return jtype == json_type_array; }

	//JSon operator=(const std::string &val) {
	//}

	JSon operator()(const std::string &key) const {
		if(jtype != json_type_object)
			throw json_exception("Not an object");
		return JSon(json_object_object_get(obj, key.c_str()));
	}
/*
	JSon operator()(const std::string &key) {
		if(jtype != json_type_object)
			throw json_exception("Not an object");
		auto *j = json_object_object_get(obj, key.c_str());
		if(j == nullptr) {
			j = json_object_object_new();
			json_object_object_add(obj, key.c_str(), j);
		}
		return JSon(j);
	}
*/

	JSon add_array(const std::string &key) {
		if(jtype != json_type_object)
			throw json_exception("Not an object");
		auto *j = json_object_new_array();
		json_object_object_add(obj, key.c_str(), j);
		return JSon(j);
	}

	void add(const std::string &key, const std::string &val) {
		if(jtype != json_type_object)
			throw json_exception("Not an object");
		auto *j = json_object_new_string(val.c_str());
		json_object_object_add(obj, key.c_str(), j);
	}

	void add(const std::string &key, uint64_t v) {
		if(jtype != json_type_object)
			throw json_exception("Not an object");
		auto *j = json_object_new_int64(v);
		json_object_object_add(obj, key.c_str(), j);
	}

	void add(const std::string &val) {
		if(jtype != json_type_array)
			throw json_exception("Not an array");
		auto *j = json_object_new_string(val.c_str());
		json_object_array_add(obj, j);
	}

	JSon operator()(const uint32_t index) const {
		if(jtype != json_type_array)
			throw json_exception("Not an array");
		return JSon(json_object_array_get_idx(obj,index));
	}

	operator std::string() {
		if(jtype != json_type_string)
			throw json_exception("Not a string");
		return std::string(json_object_get_string(obj));
	}

	template <typename T, class = typename std::enable_if<std::is_integral<T>::value>::type>
	operator T() {
		if(jtype != json_type_int)
			throw json_exception("Not an integer");
		return json_object_get_int(obj);
	} 

	template <typename T, class = typename std::enable_if<std::is_floating_point<T>::value>::type>
	operator double() {
		if(jtype != json_type_double)
			throw json_exception("Not a double");
		return json_object_get_double(obj);
	}

	template <typename T> operator std::vector<T>() {
		if(jtype != json_type_array)
			throw json_exception("Not an array");
		auto sz = json_object_array_length(obj);
		std::vector<T> v;
		v.reserve(sz);
		for(int i=0; i<sz; i++) {
			auto j = JSon(json_object_array_get_idx(obj, i));
			v.push_back(static_cast<T>(j));
		}
		return v;
	} 

	struct const_iterator  {
		const_iterator(json_object *obj, int index) : obj(obj), index(index) {}
		const_iterator(const const_iterator& rhs) : obj(rhs.obj), index(rhs.index) {}

		bool operator!= (const const_iterator& other) const {
			return index != other.index;
		}
 
		JSon operator* () const {
			return JSon(json_object_array_get_idx(obj, index));
		}
 
		const const_iterator& operator++ () {
			++index;
			return *this;
		}

		json_object *obj;
		int index;
	};


	const_iterator begin() {
		return const_iterator(obj, 0);
	}

	const_iterator end() {
		return const_iterator(obj, json_object_array_length(obj));
	}


	json_object *obj;
	enum json_type jtype;
};


#endif // JSON_H
