#ifndef APONE_XML_H
#define APONE_XML_H

#include "tinyxml2.h"
#include <coreutils/file.h>
#include <memory>
#include <string>

template <typename T> class node {
public:
	node(std::shared_ptr<node> next, T value) : value(value), next(next) {}
	T value;
	std::shared_ptr<node> next;

};

template <typename T> class list {
public:

	class const_iterator  {
	public:
		const_iterator(std::shared_ptr<node<T>> n) : n(n) {}
		const_iterator(const const_iterator& rhs) : n(rhs.n) {}

		bool operator!= (const const_iterator& other) const {
	    	return n != other.n;
		}

	    T operator* () const {
	    	return n->value;
	    }

		const const_iterator& operator++ () {
			n = n->next;
			return *this;
		}
	private:
		std::shared_ptr<node<T>> n;
	};

	const_iterator begin() const { return const_iterator(first); }
	const_iterator end() const { return const_iterator(nullptr); } 

	list() {}

	const T& head() const { return first->value; }
	const list tail() const { return list(first->next); }
	const list operator<<(const T &t) const {

		if(!first)
			return list(nullptr, t);

		return list(first->next, t);
	}

private:

	list(std::shared_ptr<node<T>> first) : first(first) {}

	list(std::shared_ptr<node<T>> first, T t) : first( std::make_shared<node<T>>(first, t) ) {}

	std::shared_ptr<node<T>> first;
};


class xml_exception : public std::exception {
public:
	xml_exception(const char *ptr = "XML Exception") : msg(ptr) {}
	virtual const char *what() const throw() { return msg; }
private:
	const char *msg;
};


class xmlnode {
public:

	xmlnode(tinyxml2::XMLNode *node) : node(node) {}

	xmlnode operator[](const std::string &tag) const {
		if(!node)
			throw xml_exception("Invalid node");
		return xmlnode(node->FirstChildElement(tag.c_str()));
	}

	bool valid() { return node != nullptr; }

	class const_iterator  {
	public:
		const_iterator(tinyxml2::XMLNode *node, const char *name = nullptr) : node(node), name(name) {}
		const_iterator(const const_iterator& rhs) : node(rhs.node), name(rhs.name) {}

		bool operator!= (const const_iterator& other) const {
			return node != other.node;
		}

		xmlnode operator* () const {
			return xmlnode(node);
		}

		const const_iterator& operator++ () {
			node = node->NextSiblingElement(name);
			return *this;
		}
	private:
		tinyxml2::XMLNode *node;
		const char *name;
	};

	class xmlnodelist {
	public:
		xmlnodelist(tinyxml2::XMLNode *node, const std::string &name = "") : node(node), name(name) {}
		const_iterator begin() const { return const_iterator(node, name == "" ? nullptr : name.c_str()); }
		const_iterator end() const { return const_iterator(nullptr); } 
	private:
		tinyxml2::XMLNode *node;
		std::string name;
	};

	xmlnodelist all(const std::string &name = "") const {
		if(!node)
			throw xml_exception("Invalid node");
		return xmlnodelist(node->FirstChildElement(name.c_str()), name);
	}

	std::string attr(const std::string &name) const {
		if(!node)
			throw xml_exception("Invalid node");
		return ((tinyxml2::XMLElement*)node)->Attribute(name.c_str());
	}

	std::string text() const {
		if(!node)
			throw xml_exception("Invalid node");
		return ((tinyxml2::XMLElement*)node)->GetText();
	}


protected:
	tinyxml2::XMLNode *node;

};

class xmldoc : public xmlnode {
public:
	static xmldoc fromText(const std::string &text) {
		return xmldoc(text);
	}

	static xmldoc fromFile(utils::File file) {
		auto text = file.read();
		return xmldoc(text);
	}

	xmldoc(const xmldoc &other) = default;

private:
	xmldoc(const std::string &text) : xmlnode(nullptr) {
		doc = std::make_shared<tinyxml2::XMLDocument>();
    	doc->Parse(text.c_str());
    	node = doc.get();
	}

	std::shared_ptr<tinyxml2::XMLDocument> doc;	
};

#endif // APONE_XML_H
