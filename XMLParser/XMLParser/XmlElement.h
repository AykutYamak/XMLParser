#pragma once
#include "XmlNode.h"
#include <string>
#include <vector>
struct XmlAttribute {
	std::string key;
	std::string value;
};

class XmlElement : public XmlNode {
private:
	std::string name;
	std::vector<XmlAttribute> attributes;
	std::vector<XmlNode*> children;
public:
	XmlElement() : XmlNode(), name(""){}
	XmlElement(const std::string& elementName) : XmlNode(), name(elementName) {}
	XmlElement(const XmlElement& other) = delete;
	XmlElement& operator=(const XmlElement& other) = delete;
	~XmlElement() override;

	void print(std::ostream& os, int indent = 0) const override;
	XmlNode* clone() const override;

	void addChild(XmlNode* child);
	void addAttribute(const std::string& key, const std::string& value);
	const std::string& getName() const { return this->name; }

	void removeAttribute(const std::string& key) {
		size_t attributesSize = attributes.size();
		for (size_t i = 0; i < attributesSize; ++i) {
			if (attributes[i].key == key)
			{
				attributes.erase(attributes.begin() + i);
				break;
			}
		}
	}

	std::string& operator[](const std::string& key) {
		for (XmlAttribute& attribute : attributes)
		{
			if (attribute.key == key)
			{
				return attribute.value;
			}
		}
		attributes.push_back({ key,"" });
		return attributes.back().value;
	}

	template<typename T>
	std::vector<T*> getChildrenOfType() const {
		std::vector<T*> result;
		for (XmlNode* child : children) {
			if (T* castedChild = dynamic_cast<T*>(child)) {
				result.push_back(castedChild);
			}
		}
		return result;
	}


	class Iterator {
	private:
		std::vector<XmlNode*>::iterator current;
	public:
		Iterator(std::vector<XmlNode*>::iterator it) : current(it){}
		Iterator& operator++() { ++current; return *this; }
		XmlNode*& operator*() { return *current; }
		bool operator!=(const Iterator& other) const { return current != other.current; }
	};

	Iterator begin() { return Iterator(children.begin()); }
	Iterator end() { return Iterator(children.end()); }
};