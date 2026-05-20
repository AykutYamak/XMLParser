#include "XmlElement.h"
#include <vector>
#include <iostream>


void XmlElement::print(int indent) const
{
    std::string spaces(indent, ' ');

    std::cout << spaces << "<" << name;

    for (const auto& [key, value] : attributes )
    {

    }
}

XmlNode* XmlElement::clone() const
{
    return nullptr;
}

void XmlElement::addChild(XmlNode* child) {
    if (child)
    {
        children.push_back(child);
        child->setParent(this);
    }
}

void XmlElement::addAttribute(const std::string& key, const std::string& value) {
    attributes[key] = value;
}

XmlElement::~XmlElement() {
    for (XmlNode* child : children) {
        delete child;
    }
}
