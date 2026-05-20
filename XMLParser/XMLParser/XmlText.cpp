#include <iostream>
#include <string>
#include "XmlText.h"

void XmlText::print(int indent) const {
	std::string spaces(indent, ' ');

	std::cout << spaces << content << '\n';
}

XmlNode* XmlText::clone() const{

	return new XmlText(this->content);
}
