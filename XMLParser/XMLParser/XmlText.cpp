#include <iostream>
#include <string>
#include "XmlText.h"
#include "Exception.h"
void XmlText::print(std::ostream& os, int indent) const {
	if (indent < 0)
	{
		throw Exception("Indentation can't be a negative number in XmlText::print");
	}
	std::string spaces(indent, ' ');

	os << spaces << content << std::endl;
}

XmlNode* XmlText::clone() const{

	return new XmlText(this->content);
}
