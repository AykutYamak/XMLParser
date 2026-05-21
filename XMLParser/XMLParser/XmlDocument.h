#pragma once
#include "XmlElement.h"
class XmlDocument {
private:
	XmlElement* root;
public:
	XmlDocument() : root(nullptr){}
	XmlDocument(const XmlDocument& other) = delete;
	XmlDocument& operator=(const XmlDocument& other) = delete;
	~XmlDocument();

	void load(const std::string& filename);
	void save(const std::string& filename) const;
	XmlElement* getRoot() const { return root; }
};