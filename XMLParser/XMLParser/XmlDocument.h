#pragma once
#include "XmlElement.h"
#include "IdDictionary.h"

class XmlDocument {
private:
	XmlElement* root;
	IdDictionary registry;
	int autoIdCounter;

	//Помощни функции за съкращение на load функцията
	std::string trim(const std::string& str) const;
	std::string ensureUniqueId(const std::string& parseId);
	XmlElement* parseOpeningTag(const std::string& content) const;

	//Помощни функции за xpath функцията
	std::vector<std::string> splitXpath(const std::string& query) const;
	std::vector<XmlElement*> evaluateXpathStep(const std::vector<XmlElement*>& currentNodes, const std::string& step) const;
	std::string extractElementText(XmlElement* element) const;

public:
	XmlDocument() : root(nullptr), autoIdCounter(0){}
	XmlDocument(const XmlDocument& other) = delete;
	XmlDocument& operator=(const XmlDocument& other) = delete;
	~XmlDocument();

	void load(const std::string& filename);
	void save(const std::string& filename) const;
	void clear();
	XmlElement* getRoot() const { return root; }
	XmlElement* getElementById(const std::string& id) const { return registry.get(id); }
	
	std::vector<std::string> xpath(const std::string& query) const;
};