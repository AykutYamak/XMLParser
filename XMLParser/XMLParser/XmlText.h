#pragma once
#include <string>
#include "XmlNode.h"
class XmlText : public XmlNode
{
private:
	std::string content;
public:
	XmlText() : XmlNode(), content(""){}
	XmlText(const std::string& textContent) : XmlNode(), content(textContent) {}

	XmlText(const XmlText& other) = delete;
	XmlText& operator=(const XmlText& other) = delete;

	~XmlText() override = default;

	void print(std::ostream& os,int indent = 0) const override;
	XmlNode* clone() const override;

	const std::string& getContent() const { return content; }
};

