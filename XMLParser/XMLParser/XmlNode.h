#pragma once

class XmlNode {
protected:
	XmlNode* parent;
public:
	XmlNode() : parent(nullptr){}

	virtual ~XmlNode() = default;
	XmlNode(const XmlNode&) = delete;
	XmlNode& operator=(const XmlNode&) = delete;

	virtual void print(int indent = 0) const = 0;
	virtual XmlNode* clone() const = 0;


	XmlNode* getParent() const { return parent; }
	void setParent(XmlNode* p) { parent = p; }
};