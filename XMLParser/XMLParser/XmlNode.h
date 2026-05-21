#pragma once
#include <ostream>
class XmlNode {
protected:
	XmlNode* parent;
public:
	XmlNode() : parent(nullptr){}

	virtual ~XmlNode() = default;
	XmlNode(const XmlNode& other) = delete;
	XmlNode& operator=(const XmlNode& other) = delete;

	virtual void print(std::ostream& os,int indent = 0) const = 0;
	virtual XmlNode* clone() const = 0;


	XmlNode* getParent() const { return parent; }
	void setParent(XmlNode* p) { parent = p; }
};