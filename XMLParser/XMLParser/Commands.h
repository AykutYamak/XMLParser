#pragma once
#include "Command.h"
#include <iostream>
#include "Exception.h"
#include "XmlText.h"

class PrintCommand :public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (doc.getRoot())
		{
			doc.getRoot()->print(std::cout, 0);
		}
		else
		{
			std::cout << "No document loaded or document is emtpy." << std::endl;
		}
	}
};

class XPathCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (args.empty())
		{
			std::cout << "Error: XPath query missing. Usage: xpath <query>." << std::endl;
			return;
		}
		std::vector<std::string> results = doc.xpath(args);

		if (results.empty())
		{
			std::cout << "No results found." << std::endl;
		}
		else
		{
			std::cout << "Results: " << std::endl;
			size_t resultsSize = results.size();
			for (size_t i = 0; i < resultsSize; i++)
			{
				std::cout << "[" << i << "]" << results[i] << std::endl;
			}
		}
	}
};

class SaveCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override
	{
		std::string filename = args.empty() ? "saved_file.xml" : args;
		try
		{
			doc.save(filename);
			std::cout << "Successfully saved to: " << filename << "." << std::endl;
		}
		catch (const Exception& e)
		{
			std::cout << "Error saving: " << e.what() << std::endl;
		}
	}
};

class SelectCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		size_t spacePos = args.find(' ');
		if (spacePos == std::string::npos)
		{
			std::cout << "Error: Usage: select <id> <key>" << std::endl;
			return;
		}
		std::string id = args.substr(0, spacePos);
		std::string key = args.substr(spacePos + 1);

		XmlElement* el = doc.getElementById(id);
		if (!el)
		{
			std::cout << "Element with id '" << id << "' not found." << std::endl;
		}
		else
		{
			std::string val = (*el)[key];
			if (val.empty()) std::cout << "Attribute '" << key << "' not found or empty." << std::endl;
			else std::cout << "Value: " << val << std::endl;
		}
	}
};

class SetCommand :public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		size_t firstSpace = args.find(' ');
		if (firstSpace == std::string::npos) { std::cout << "Error: Usage: set <id> <key> <value>" << std::endl; return; }
		size_t secondSpace = args.find(' ', firstSpace + 1);
		if (secondSpace == std::string::npos) { std::cout << "Error: Usage: set <id> <key> <value>" << std::endl; return; }

		std::string id = args.substr(0, firstSpace);
		std::string key = args.substr(firstSpace + 1, secondSpace - firstSpace - 1);
		std::string value = args.substr(secondSpace + 1);

		XmlElement* el = doc.getElementById(id);
		if (!el) { std::cout << "Element with id '" << id << "' not found." << std::endl; }
		else { (*el)[key] = value; std::cout << "Attribute set successfully." << std::endl; }
	}
};

class DeleteCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		size_t spacePos = args.find(' ');
		if (spacePos == std::string::npos)
		{
			std::cout << "Error: Usage: delete <id> <key>" << std::endl;
			return;
		}
		std::string id = args.substr(0, spacePos);
		std::string key = args.substr(spacePos + 1);
		XmlElement* el = doc.getElementById(id);

		if (!el)
		{
			std::cout << "Element with id '" << id << "' not found." << std::endl;
		}
		else
		{
			el->removeAttribute(key);
			std::cout << "Attribute '" << key << "' deleted" << std::endl;
		}
	}
};

class ChildrenCommand :public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		std::string id = args;
		XmlElement* el = doc.getElementById(id);

		if (!el) { std::cout << "Element with id '" << id << "' not found." << std::endl; return; }
		std::vector<XmlElement*> kids = el->getChildrenOfType<XmlElement>();
		if (kids.empty()) { std::cout << "No child elements found." << std::endl; return; }
		size_t kidsSize = kids.size();
		for (size_t i = 0; i < kidsSize; i++)
		{
			std::cout << "Child " << i << " (" << kids[i]->getName() << ") attributes: ";
			std::cout << "id=\"" << (*kids[i])["id"] << "\" " << std::endl;
		}
	}
};

class ChildCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		size_t spacePos = args.find(' ');
		if (spacePos == std::string::npos) { std::cout << "Error: Usage: child <id> <n>" << std::endl; return; }

		std::string id = args.substr(0, spacePos);
		std::string n_str = args.substr(spacePos + 1);
		XmlElement* el = doc.getElementById(id);

		if (!el) { std::cout << "Element with id '" << id << "' not found." << std::endl; return; }

		int n = std::stoi(n_str);
		std::vector<XmlElement*> kids = el->getChildrenOfType<XmlElement>();

		if (n >= 0 && n < kids.size()) {
			std::cout << "Child " << n << " details" << std::endl;
			kids[n]->print(std::cout, 0);
		}
		else
		{
			std::cout << "Error: Index " << n << " is out of bounds. " << std::endl;
		}
	}
};

class TextCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		std::string id = args;
		XmlElement* el = doc.getElementById(id);
		if(!el) {std::cout << "Element with id '" << id << "' not found." << std::endl; return;}

		std::vector<XmlText*> txtNodes = el->getChildrenOfType<XmlText>();
		std::string result = "";
		size_t txtNodesSize = txtNodes.size();
		for (size_t i = 0; i < txtNodesSize; i++)
		{
			result += txtNodes[i]->getContent();
		}
		std::cout << "Text: " << result << std::endl;
	}
};

std::string intToString(int n) {
	if (n == 0) return "0";
	std::string result = "";
	while (n>0)
	{
		char digit = (n % 10) + '0';
		result += digit;
		n /= 10;
	}

	size_t len = result.length();
	for (size_t i = 0; i < len/2; i++)
	{
		char temp = result[i];
		result[i] = result[len - 1 - i];
		result[len - 1 - i] = temp;
	}
	return result;
}

class NewChildCommand :public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		std::string id = args;
		XmlElement* el = doc.getElementById(id);
		if(!el) { std::cout << "Element with id '" << id << "' not found." << std::endl; return; }

		XmlElement* newElement = new XmlElement("new_element");
		static int newChildCounter = 1;
		std::string tempId = "newChild_" + intToString(newChildCounter++);

		(*newElement)["id"] = tempId;	
		el->addChild(newElement);

		std::cout << "New child added to '" << id << "' with generated id '" << tempId << "'." << std::endl;
	}
};
