#include "XmlDocument.h"
#include "../Exceptions/Exception.h"
#include "XmlText.h"
#include <fstream>
#include <iostream>
#include <vector>

void XmlDocument::moveFrom(XmlDocument&& other) {
	root = other.root;
	registry = std::move(other.registry);
	autoIdCounter = other.autoIdCounter;
	openedFilePath = std::move(other.openedFilePath);
	other.root = nullptr;
	other.autoIdCounter = 0;
}

XmlDocument::XmlDocument(XmlDocument&& other) : root(nullptr), autoIdCounter(0) {
	moveFrom(std::move(other));
}

XmlDocument& XmlDocument::operator=(XmlDocument&& other) {
	if (this!=&other)
	{
		clear();
		moveFrom(std::move(other));
	}
	return *this;
}

XmlDocument::~XmlDocument() {
	clear();
}

std::string XmlDocument::trim(const std::string& str) const {
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) return "";

	size_t last = str.find_last_not_of(" \t\r\n");
	std::string trimmed = str.substr(first, (last - first + 1));

	return trimmed;
}

std::string XmlDocument::ensureUniqueId(const std::string& parseId) {
	std::string currentId = parseId;
	if (currentId.empty())
	{
		do
		{
			currentId = "gen_" + std::to_string(autoIdCounter++);
		} while (registry.contains(currentId));
	}
	else
	{
		if (registry.contains(currentId))
		{
			int suffix = 1;
			std::string baseId = currentId;
			do
			{
				currentId = baseId + "_" + std::to_string(suffix++);
			} while (registry.contains(currentId));
		}
	}

	return currentId;
}

XmlElement* XmlDocument::parseOpeningTag(const std::string& content) const {
	size_t spacePos = content.find(' ');
	if (spacePos == std::string::npos)
	{
		return new XmlElement(content);
	}

	std::string tagName = content.substr(0, spacePos);
	XmlElement* newElement = new XmlElement(tagName);

	std::string attributesString = content.substr(spacePos + 1);

	while (!attributesString.empty())
	{
		size_t firstNonSpace = attributesString.find_first_not_of(" \t\r\n");
		if (firstNonSpace == std::string::npos) break;
		
		attributesString = attributesString.substr(firstNonSpace);
		size_t eqPos = attributesString.find('=');
		if (eqPos == std::string::npos) break;

		std::string key = attributesString.substr(0, eqPos);

		attributesString = attributesString.substr(eqPos + 1);
		size_t firstMention = attributesString.find('"');
		if (firstMention == std::string::npos) break;

		size_t secondMention = attributesString.find('"', firstMention + 1);
		if (secondMention == std::string::npos) break;

		std::string val = attributesString.substr(firstMention + 1, secondMention - firstMention - 1);
		newElement->addAttribute(key, val);
		attributesString = attributesString.substr(secondMention + 1);
	}
	return newElement;
}

void XmlDocument::load(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) throw FileException("Could not open file for reading: " + filename);
	clear();
	std::vector<XmlElement*> parseStack;
	std::string line;

	try
	{
		while (std::getline(file, line))
		{
			std::string trimmed = trim(line);
			if (trimmed.empty()) continue;

			if (trimmed.length() >= 2 && trimmed.substr(0,2) == "</")
			{
				if (parseStack.empty()) throw ParseException("Mismatched closing tag found without an opening tag.");

				parseStack.pop_back();
			}
			else if (trimmed.front() == '<' && trimmed.back() == '>')
			{
				std::string content = trimmed.substr(1, trimmed.length() - 2);
				XmlElement* newElement = parseOpeningTag(content);
				std::string uniqueId = ensureUniqueId((*newElement)["id"]);
				
				(*newElement)["id"] = uniqueId;
				registry.add(uniqueId, newElement);

				if (parseStack.empty()){
					root = newElement;
				}
				else{
					parseStack.back()->addChild(newElement);
				}
				parseStack.push_back(newElement);
			}
			else
			{
				if (parseStack.empty()) throw ParseException("Unexpected text found before or after the root element.");

				parseStack.back()->addChild(new XmlText(trimmed));
			}
		}
		if (!parseStack.empty()) throw ParseException("End of file reached, but there are unclosed XML tags.");
	}
	catch (...)
	{
		clear();
		file.close();
		throw;
	}
	file.close();
	this->openedFilePath = filename;
}

void XmlDocument::saveAs(const std::string& filename) const{
	if (!root) return;

	std::ofstream file(filename);

	if (!file.is_open())
	{
		throw FileException("Could not open file for writing: " + filename);
	}

	root->print(file, 0);
	file.close();
}

void XmlDocument::save() const{
	if (openedFilePath.empty())
	{
		throw FileException("Could not open file for writing: " + openedFilePath);
	}
	saveAs(openedFilePath);
}

void XmlDocument::clear() {
	if (root)
	{
		delete root;
		root = nullptr;
	}
	registry.clear();
	autoIdCounter = 0;
}

std::vector<std::string> XmlDocument::splitXpath(const std::string& query) const {
	std::vector<std::string> steps;
	size_t start = 0;
	size_t end = query.find('/');
	while (end!=std::string::npos)
	{
		std::string step = query.substr(start, end - start);
		if (!step.empty()) steps.push_back(step);
		start = end + 1;
		end = query.find('/',start);
	}

	if (start<query.length())
	{
		std::string step = query.substr(start);
		if (!step.empty()) steps.push_back(step);
	}
	return steps;
}

std::string XmlDocument::extractElementText(XmlElement* element) const {
	std::string text = "";
	std::vector<XmlText*> txtNodes = element->getChildrenOfType<XmlText>();
	size_t txtNodesSize = txtNodes.size();
	for (size_t i = 0; i < txtNodesSize; i++)
	{
		text += txtNodes[i]->getContent();
	}

	size_t first = text.find_first_not_of(" \t\r\n");
	if (first==std::string::npos)
	{
		return "";
	}
	size_t last = text.find_last_not_of(" \t\r\n");
	return text.substr(first, last - first + 1);
}

std::vector<XmlElement*> XmlDocument::evaluateXpathStep(const std::vector<XmlElement*>& currentNodes, const std::string& step) const {
	std::vector<XmlElement*> filteredNodes;

	std::string tagName = step;
	std::string condition = "";
	size_t bracketOpen = step.find('[');
	if (bracketOpen != std::string::npos)
	{
		size_t bracketClose = step.find(']');
		if (bracketClose!=std::string::npos)
		{
			tagName = step.substr(0, bracketOpen);
			condition = step.substr(bracketOpen + 1, bracketClose - bracketOpen - 1);
		}
	}
	size_t currentNodesSize = currentNodes.size();
	for (size_t i = 0; i < currentNodesSize; i++)
	{
		std::vector<XmlElement*> children = currentNodes[i]->getChildrenOfType<XmlElement>();
		std::vector<XmlElement*> matchingChildren;
		size_t childrenSize = children.size();
		for (size_t j = 0; j < childrenSize; j++)
		{
			if (tagName == "*" || children[j]->getName() == tagName)
			{
				matchingChildren.push_back(children[j]);
			}
		}
		if (condition.empty())
		{
			size_t matchingChildrenSize = matchingChildren.size();
			for (size_t j = 0; j < matchingChildrenSize; j++)
			{
				filteredNodes.push_back(matchingChildren[j]);
			}
			continue;
		}
		if (condition[0] == '@')
		{
			std::string attributeName = condition.substr(1);
			size_t matchingChildrenSize = matchingChildren.size();
			for (size_t j = 0; j < matchingChildrenSize; j++)
			{
				if (!matchingChildren[j]->getAttributeValue(attributeName).empty())
				{
					filteredNodes.push_back(matchingChildren[j]);
				}
			}
			continue;
		}
		bool isIndex = true;
		size_t conditionLength = condition.length();
		for (size_t j = 0; j < conditionLength; j++)
		{
			if (condition[j] < '0' || condition[j] > '9')
			{
				isIndex = false;
				break;
			}
		}
		if (isIndex && !condition.empty())
		{
			int index = std::stoi(condition);
			if (index >= 0 && index < matchingChildren.size()) {
				filteredNodes.push_back(matchingChildren[index]);
			}
		}
		else
		{
			size_t eqPos = condition.find('=');
			if (eqPos != std::string::npos)
			{
				std::string childTagName = condition.substr(0, eqPos);
				std::string expectedValue = condition.substr(eqPos + 1);
				if (expectedValue.length() >= 2 && expectedValue[0] == '"')
				{
					expectedValue = expectedValue.substr(1, expectedValue.length() - 2);
				}
				size_t matchingChildrenSize = matchingChildren.size();
				for (size_t j = 0; j < matchingChildrenSize; j++)
				{
					std::vector<XmlElement*> subChildren = matchingChildren[j]->getChildrenOfType<XmlElement>();
					bool conditionMet = false;
					size_t subChildrenSize = subChildren.size();
					for (size_t n = 0; n < subChildrenSize; n++)
					{
						if (subChildren[n]->getName() == childTagName)
						{
							std::string actualValue = extractElementText(subChildren[n]);
							if (actualValue == expectedValue)
							{
								conditionMet = true;
								break;
							}
						}
					}
					if (conditionMet)
					{
						filteredNodes.push_back(matchingChildren[j]);
					}
				}
			}
		}
	}
	return filteredNodes;
}
std::vector<std::string> XmlDocument::xpath(const std::string& query) const {
	std::vector<std::string> results;
	if (!root || query.empty()) return results;

	std::vector<std::string> steps = splitXpath(query);
	if (steps.empty()) return results;

	bool extractAttribute = false;
	std::string targetAttribute = "";

	std::string lastStep = steps.back();
	size_t bracketPos = lastStep.find("[@");
	if (bracketPos != std::string::npos)
	{
		extractAttribute = true;
		size_t closeBracket = lastStep.find(']', bracketPos);
		targetAttribute = lastStep.substr(bracketPos +2, closeBracket-bracketPos -2);
	}

	std::vector<XmlElement*> currentNodes;
	currentNodes.push_back(root);
	size_t stepsSize = steps.size();
	for (size_t i = 0; i < stepsSize; i++)
	{
		currentNodes = evaluateXpathStep(currentNodes, steps[i]);
		if (currentNodes.empty()) break;
	}
	size_t currentNodesSize = currentNodes.size();
	for (size_t i = 0; i < currentNodesSize ; i++)
	{
		if (extractAttribute)
		{
			std::string attrValue = currentNodes[i]->getAttributeValue(targetAttribute);
			if (!attrValue.empty())
			{
				results.push_back(attrValue);
			}
		}
		else
		{
			std::string text = extractElementText(currentNodes[i]);
			if (!text.empty())
			{
				results.push_back(text);
			}
		}
	}
	return results;
}
