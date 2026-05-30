#pragma once
#include "Command.h"
#include <iostream>
#include "../XmlCore/XmlText.h"
#include "../Exceptions/Exception.h"

/**
 * @brief Команда за принтиране на XML дървото (команда: print).
 * @details Извежда цялото XML дърво в конзолата с подходящо форматиране (отстъпи).
 */
class PrintCommand :public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return;
		}
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

/**
 * @brief Команда за изпълнение на XPath заявки (команда: xpath).
 * @details Извиква търсачката на документа и извежда номериран списък с резултатите.
 * Очаква заявката да бъде подадена като аргумент.
 */
class XPathCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}
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

/**
 * @brief Команда за запазване на текущия файл (команда: save).
 * @details Презаписва оригиналния файл, от който е зареден документът.
 */
class SaveCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override
	{
		if (!doc.getRoot())
		{
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return;
		}
		if (!args.empty())
		{
			std::cout << "Error: 'save' command does not take any arguments. Use 'saveas <path>' to save to a different file." << std::endl;
			return;
		}

		try
		{
			doc.save();
			std::cout << "Successfully saved changes to the current file." << std::endl;
		}
		catch (const Exception& e)
		{
			std::cout << "Error saving: " << e.what() << std::endl;
		}
	}
};

/**
 * @brief Команда за запазване във файл с ново име (команда: saveas).
 * @details Позволява пътища с интервали, ако са оградени в кавички.
 */
class SaveAsCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}
		if (args.empty())
		{
			std::cout << "Error: 'saveas' requires a file path. Usage: saveas <path>" << std::endl;
			return;
		}
		std::string targetPath = args;
		if (targetPath.front() == '"' && targetPath.back() == '"')
		{
			targetPath = targetPath.substr(1, targetPath.length() - 2);
		}
		try
		{
			doc.saveAs(targetPath);
			size_t lastSlash = targetPath.find_last_of("/\\");
			std::string filename = (lastSlash == std::string::npos) ? targetPath : targetPath.substr(lastSlash + 1);
			
			std::cout << "Successfully saved " << filename << "\n";
		}
		catch (const Exception& e)
		{
			std::cout << "Error saving file: " << e.what() << '\n';
		}
	}

};

/**
 * @brief Команда за извличане на стойност на атрибут (команда: select).
 * @details Очаква ID на елемент и ключ на атрибут. Извежда стойността му.
 */
class SelectCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}
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
			std::string val = el->getAttributeValue(key);
			if (val.empty()) std::cout << "Attribute '" << key << "' not found or empty." << std::endl;
			else std::cout << "Value: " << val << std::endl;
		}
	}
};

/**
 * @brief Команда за задаване на стойност на атрибут (команда: set).
 * @details Добавя нов атрибут или обновява съществуващ по дадено ID на елемент.
 */
class SetCommand :public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}

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

/**
 * @brief Команда за изтриване на атрибут (команда: delete).
 * @details Премахва атрибут от елемент по зададено ID и ключ.
 */
class DeleteCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}
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

/**
 * @brief Команда за извеждане на атрибутите на всички деца (команда: children).
 * @details Извежда списък с всички деца на подадения елемент (по ID) и техните атрибути.
 */
class ChildrenCommand :public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}
		std::string id = args;
		XmlElement* el = doc.getElementById(id);

		if (!el) { std::cout << "Element with id '" << id << "' not found." << std::endl; return; }
		
		std::vector<XmlElement*> kids = el->getChildrenOfType<XmlElement>();
		
		if (kids.empty()) { std::cout << "No child elements found." << std::endl; return; }
		
		size_t kidsSize = kids.size();
		for (size_t i = 0; i < kidsSize; i++)
		{
			std::cout << "Child " << i << " (" << kids[i]->getName() << ") attributes: ";
			const auto& attributes = kids[i]->getAttributes();
			if (attributes.empty())
			{
				std::cout << "None" << std::endl;
			}
			else
			{
				size_t attributesSize = attributes.size();
				for (size_t j = 0; j < attributesSize; j++)
				{
					std::cout << attributes[j].key << "=\"" << attributes[j].value << "\" ";
				}
			}
			std::cout << std::endl;
		}
	}
};

/**
 * @brief Команда за достъпване на N-тото дете (команда: child).
 * @details Очаква ID на родителя и индекс N. Принтира форматирано самото дете.
 */
class ChildCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}
		size_t spacePos = args.find(' ');
		if (spacePos == std::string::npos) { std::cout << "Error: Usage: child <id> <n>" << std::endl; return; }

		std::string id = args.substr(0, spacePos);
		std::string n_str = args.substr(spacePos + 1);
		XmlElement* el = doc.getElementById(id);

		if (!el) { std::cout << "Element with id '" << id << "' not found." << std::endl; return; }

		int n = std::stoi(n_str);
		std::vector<XmlElement*> kids = el->getChildrenOfType<XmlElement>();

		if (n >= 0 && n < (int)kids.size()) {
			std::cout << "Child " << n << " details" << std::endl;
			kids[n]->print(std::cout, 0);
		}
		else
		{
			std::cout << "Error: Index " << n << " is out of bounds. " << std::endl;
		}
	}
};

/**
 * @brief Команда за извличане на текст (команда: text).
 * @details Връща цялото текстово съдържание (XmlText възли) в даден елемент по ID.
 */
class TextCommand : public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}
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

/**
*@brief Команда за създаване на ново дете(команда: newChild).
* @details Добавя нов празен елемент(<new_element>) към зададен родител и го регистрира с уникално ID.
*/
class NewChildCommand :public Command {
public:
	void execute(XmlDocument& doc, const std::string& args) override {
		if (!doc.getRoot()) {
			std::cout << "Error: No file is currently loaded. Please use 'open' first." << std::endl;
			return; 
		}
		std::string id = args;
		XmlElement* el = doc.getElementById(id);
		if(!el) { std::cout << "Element with id '" << id << "' not found." << std::endl; return; }

		XmlElement* newElement = new XmlElement("new_element");
		static int newChildCounter = 1;
		std::string tempId = "newChild_" + std::to_string(newChildCounter++);

		(*newElement)["id"] = tempId;
		doc.registerElement(tempId, newElement);
		el->addChild(newElement);

		std::cout << "New child added to '" << id << "' with generated id '" << tempId << "'." << std::endl;
	}
};
