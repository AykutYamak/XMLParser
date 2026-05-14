#pragma once
#include <iostream>
#include <memory>

class XmlNode {
protected:
	XmlNode* parent;
public:
	XmlNode() : parent(nullptr){}

	virtual ~XmlNode() = default;
	XmlNode(const XmlNode&) = delete;
	XmlNode& operator=(const XmlNode&) = delete;
	XmlNode(XmlNode&&) = default;
	XmlNode& operator=(XmlNode&&) = default;

	virtual void print(int indent = 0) const = 0;
	virtual std::unique_ptr<XmlNode> clone() const = 0;
	
	XmlNode* getParent() const { return parent; }
};