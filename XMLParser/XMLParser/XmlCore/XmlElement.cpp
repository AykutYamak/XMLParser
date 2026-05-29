#include "XmlElement.h"
#include "../Exceptions/Exception.h"
#include <iostream>


void XmlElement::print(std::ostream& os, int indent) const
{
    if (indent<0)
    {
        throw Exception("Indentation can't be a negative number in XmlElement::print");
    }
    std::string spaces(indent, ' ');
    os << spaces << "<" << name;
    
    for (const XmlAttribute& attr : attributes) {
        os << " " << attr.key << "=\"" << attr.value << "\"";
    }
    if (children.empty())
    {
        os << " />" << std::endl;
    }
    else
    {
        os << ">" << std::endl;
        for (XmlNode* child : children) {
            child->print(os ,indent + 4);
        }
        os << spaces << "</" << name << ">" << std::endl;
    }

}
XmlNode* XmlElement::clone() const
{
    XmlElement* copy = new XmlElement(this->name);
    copy->attributes = this->attributes;
    for (XmlNode* child : children)
    {
        copy->addChild(child->clone());
    }
    return copy;
}

void XmlElement::addChild(XmlNode* child) {
    if (child)
    {
        children.push_back(child);
        child->setParent(this);
    }
    else
    {
        throw Exception("Can't add a null child to XML Element: " + name);
    }
}

void XmlElement::addAttribute(const std::string& key, const std::string& value) {
    if (key.empty())
    {
        throw Exception("Can't add an attribute with an empty key ot element: " + name);
    }
    attributes.push_back({key,value});

}

XmlElement::~XmlElement() {
    for (XmlNode* child : children) {
        delete child;
    }
    children.clear();
}
