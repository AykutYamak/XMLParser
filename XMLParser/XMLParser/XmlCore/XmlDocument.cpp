#include "XmlDocument.h"
#include "../Exceptions/Exception.h"
#include "XmlText.h"
#include <fstream>
#include <iostream>
#include <vector>
/**
 * @brief Помощна функция, реализираща Move семантика (прехвърляне на собственост).
 * * @details В съвременния C++, копирането на огромни обекти (като цяло XML дърво
 * и неговият ID речник) е изключително бавна и тежка операция. Move семантиката решава
 * този проблем, като вместо да копира данните, тя просто "открадва" (пренасочва)
 * указателите от стария обект (`other`) към текущия.
 * * **Процес на прехвърляне:**
 * 1. Указателят `root` на текущия документ се насочва към дървото на `other`.
 * 2. Речникът `registry` и пътят `openedFilePath` се прехвърлят ефективно чрез `std::move`.
 * 3. **Критична стъпка:** Указателят `root` на `other` се нулира (`nullptr`). Ако не направим това,
 * когато деструкторът на `other` се изпълни, той ще изтрие дървото и текущият документ
 * ще остане с "висящ" (dangling) указател, което води до краш на програмата (Segfault).
 * * @param other Обектът (rvalue референция), чиито ресурси ще бъдат прехвърлени.
 * След изпълнението на функцията, `other` остава в празно, но валидно състояние.
 */
void XmlDocument::moveFrom(XmlDocument&& other) {
	root = other.root;
	registry = std::move(other.registry);
	autoIdCounter = other.autoIdCounter;
	openedFilePath = std::move(other.openedFilePath);
	other.root = nullptr;
	other.autoIdCounter = 0;
}

/**
 * @brief Move конструктор.
 * Инициализира празен документ и веднага прехвърля ресурсите от подадения обект.
 */
XmlDocument::XmlDocument(XmlDocument&& other) : root(nullptr), autoIdCounter(0) {
	moveFrom(std::move(other));
}

/**
 * @brief Move оператор за присвояване.
 * Първо изчиства текущите ресурси на обекта, след което присвоява новите чрез moveFrom.
 */
XmlDocument& XmlDocument::operator=(XmlDocument&& other) {
	if (this!=&other)
	{
		clear();
		moveFrom(std::move(other));
	}
	return *this;
}

/**
 * @brief Деструктор.
 * Гарантира безопасното освобождаване на паметта, заделена за XML дървото.
 */
XmlDocument::~XmlDocument() {
	clear();
}

/**
 * @brief Премахва излишните интервали и празни символи в началото и края на стринг.
 * @param str Оригиналният низ.
 * @return Новият низ (форматиран).
 */
std::string XmlDocument::trim(const std::string& str) const {
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) return "";

	size_t last = str.find_last_not_of(" \t\r\n");
	std::string trimmed = str.substr(first, (last - first + 1));

	return trimmed;
}

/**
 * @brief Гарантира, че всеки елемент има уникално ID.
 * @details Ако тагът няма ID, се генерира автоматично (напр. "gen_0").
 * Ако ID-то съществува, към него се добавя числов суфикс (напр. "id_1").
 * @param parseId ID-то, прочетено от XML файла (или празен низ).
 * @return Уникалният идентификатор, готов за запис в речника.
 */
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

/**
 * @brief Парсва съдържанието на отварящ таг (име и атрибути).
 * @details Извлича името на елемента (до първия интервал) и след това итерира
 * през остатъка от стринга, за да намери и запише всички атрибути във формат key="value".
 * @param content Текстът, намиращ се вътре в скобите < и >.
 * @return Указател към новосъздадения обект XmlElement.
 */
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

/**
 * @brief Зарежда, валидира и построява йерархичното XML дърво от текстов файл.
 * * @details Тази функция е сърцето на парсъра. Тя чете файла ред по ред и използва
 * структура от данни "стек" (std::vector<XmlElement*> parseStack), за да следи
 * вложеността на таговете и да изгради правилни връзки родител-дете.
 * * **Алгоритъм на работа:**
 * 1. **Отварящи тагове (`<tag>`):** Създава нов XmlElement, регистрира го в ID речника
 * (генерирайки ID, ако липсва) и го добавя като дете на елемента, който в момента
 * е на върха на стека. След това новият елемент се поставя (push) в стека.
 * 2. **Затварящи тагове (`</tag>`):** Премахва (pop) най-горния елемент от стека,
 * тъй като неговият обхват е приключил и всички негови деца са вече добавени.
 * 3. **Текстови възли:** Ако прочетеният ред не е таг, той се третира като текст (XmlText)
 * и се добавя директно към елемента на върха на стека.
 * * **Специални случаи, които парсърът обработва успешно:**
 * - **Inline тагове:** Разпознава тагове с текст на един ред (напр. `<name>John</name>`),
 * парсва ги и ги добавя в дървото, без да ги задържа в стека.
 * - **Самозатварящи се тагове:** Обработва тагове завършващи на `/>` (напр. `<br/>`),
 * като ги инстанцира, но "прескача" добавянето им в стека.
 * - **Множество Root елементи:** XML стандартът изисква само 1 корен. Ако парсърът
 * открие множество елементи на нулево ниво, той автоматично генерира невидим
 * `<document_wrapper>`, който да ги обедини и да спаси структурата от презаписване.
 * - **Липсващ файл:** Ако подаденият файл не съществува на диска, функцията не гърми,
 * а създава нов празен файл, готов за работа.
 * * @param filename Пътят към файла, който ще бъде прочетен и зареден в паметта.
 * @throw FileException Ако възникне критична грешка при I/O операциите.
 * @throw ParseException При невалиден XML (напр. затварящ таг без отварящ, или текст извън root-а).
 */
void XmlDocument::load(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::ofstream newFile(filename);
		newFile.close();
		clear();
		this->openedFilePath = filename;
		return;
	}
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
				continue;
			}

			size_t firstClose = trimmed.find('>');
			size_t lastOpen = trimmed.rfind("</");
			if (firstClose!=std::string::npos && lastOpen != std::string::npos && firstClose < lastOpen)
			{
				std::string openTagContent = trimmed.substr(1, firstClose - 1);
				std::string textContent = trimmed.substr(firstClose + 1, lastOpen - firstClose - 1);
				XmlElement* newElement = parseOpeningTag(openTagContent);
				std::string uniqueId = ensureUniqueId((*newElement)["id"]);
				(*newElement)["id"] = uniqueId;
				registry.add(uniqueId, newElement);
				if (!textContent.empty())
				{
					newElement->addChild(new XmlText(textContent));
				}
				if (parseStack.empty())
				{
					if (root == nullptr)
					{
						root = newElement;
					}
					else
					{
						if (root->getName() != "document_wrapper")
						{
							XmlElement* wrapper = new XmlElement("document_wrapper");
							wrapper->addChild(root);
							root = wrapper;
						}
						root->addChild(newElement);
					}
				}
				else
				{
					parseStack.back()->addChild(newElement);
				}
				continue;

			}
			else if (trimmed.front() == '<' && trimmed.back() == '>')
			{
				bool isSelfClosing = (trimmed.length() >= 2 && trimmed[trimmed.length() - 2] == '/');
				size_t charsToDrop = isSelfClosing ? 3 : 2;
				std::string content = trimmed.substr(1, trimmed.length() - charsToDrop);

				XmlElement* newElement = parseOpeningTag(content);
				std::string uniqueId = ensureUniqueId((*newElement)["id"]);
				
				(*newElement)["id"] = uniqueId;
				registry.add(uniqueId, newElement);

				if (parseStack.empty()){
					if (root == nullptr)
					{
						root = newElement;
					}
					else
					{
						if (root->getName() != "document_wrapper")
						{
							XmlElement* wrapper = new XmlElement("document_wrapper");
							wrapper->addChild(root);
							root = wrapper;
						}
						root->addChild(newElement);
					}
				}
				else{
					parseStack.back()->addChild(newElement);
				}

				if (!isSelfClosing)
				{
					parseStack.push_back(newElement);
				}
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

/**
 * @brief Запазва текущия XML документ в специфичен файл.
 * @details Извиква рекурсивната функция `print` на root елемента. Ако root-ът е
 * скритият "document_wrapper", принтира само неговите деца, за да не замърсява ID-тата.
 * @param filename Пътят, където да се запази файлът.
 * @throw FileException Ако документът е празен или файлът не може да се отвори.
 */
void XmlDocument::saveAs(const std::string& filename) const{
	if (!root) throw FileException("Cannot save an empty document. Please open or create a file first."); 

	std::ofstream file(filename);

	if (!file.is_open()) throw FileException("Could not open file for writing: " + filename);

	if (root->getName() == "document_wrapper")
	{
		for (auto i = root->begin(); i != root->end(); ++i)
		{
			(*i)->print(file, 0);
		}
	}
	else
	{
		root->print(file, 0);
	}
	file.close();
}

/**
 * @brief Запазва документа в текущо отворения файл.
 * @throw FileException Ако няма зареден файл (напр. потребителят не е извикал open).
 */
void XmlDocument::save() const{
	if (openedFilePath.empty())
	{
		throw FileException("No file is currently open to save into. Use 'saveas <path>' instead.");
	}
	saveAs(openedFilePath);
}

/**
 * @brief Изчиства цялата памет и нулира състоянието на документа.
 * @details Извиква delete върху root, което каскадно изтрива всички деца.
 * Изчиства ID речника и нулира брояча.
 */
void XmlDocument::clear() {
	if (root)
	{
		delete root;
		root = nullptr;
	}
	registry.clear();
	autoIdCounter = 0;
}

/**
 * @brief Разделя XPath заявка на отделни стъпки.
 * @param query Цялата заявка (напр. "person/address/city").
 * @return Вектор, съдържащ отделните елементи за търсене (напр. ["person", "address", "city"]).
 */
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

/**
 * @brief Извлича съединения текст на всички дъщерни текстови възли.
 * @param element Елементът, чийто вътрешен текст търсим.
 * @return Форматираният текст без излишни интервали.
 */
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

/**
 * @brief Оценява (филтрира) една конкретна стъпка от XPath заявката върху списък от възли.
 * * @details Тази функция получава списък с възли от предишната стъпка и търси вътре
 * в техните деца. Тя съдържа същинската бизнес логика на XPath търсачката, тъй като
 * е способна да разпознава и обработва сложни предикати в квадратни скоби `[]`.
 * * **Поддържани операции и филтри:**
 * - **Търсене по име:** Намира всички деца, чието име съвпада с търсеното (напр. `address`).
 * - **Wildcard (`*`):** Връща абсолютно всички дещерни елементи, независимо от името на тага им.
 * - **Индексиране (`[n]`):** Връща само n-тото поред намерено дете (напр. `[0]` връща първия резултат).
 * - **Търсене по атрибут (`[@attr]`):** Връща само тези елементи, които притежават
 * посочения атрибут, независимо от неговата стойност (напр. `[@id]`).
 * - **Проверка на стойност (`[child="value"]`):** Филтрира елементите, като оставя само тези,
 * които имат конкретно дете с точно определено текстово съдържание (напр. `[address="Bulgaria"]`).
 * * @param currentNodes Вектор с XML възлите, които са резултат от предишната стъпка.
 * @param step Стринг, представляващ текущата част от заявката за парсване.
 * @return Нов вектор, съдържащ само указателите към онези дещерни възли, които отговарят на условията.
 */
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
			if (index >= 0 && index < (int)matchingChildren.size()) {
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

/**
 * @brief Главна функция за изпълнение на пълни XPath заявки върху XML документа.
 * * @details Обработва потребителски заявки за търсене на информация чрез задаване на път.
 * Логиката е разделена на последователни фази, за да се гарантира правилно филтриране:
 * * 1. **Разделяне:** Разделя пълния стринг на заявката (напр. `person/address/city`)
 * на отделни стъпки по разделителя `/` чрез помощната функция `splitXpath`.
 * 2. **Анализ на целта:** Проверява дали последната стъпка в заявката е насочена към
 * извличане на атрибут (т.е. дали завършва на `[@attributeName]`). Ако да, запаметява
 * името на атрибута и премахва тази част от пътя.
 * 3. **Последователно обхождане:** Започвайки от корена на документа (`root`),
 * функцията подава списъка с текущи възли към `evaluateXpathStep` за всяка една стъпка.
 * Резултатът от всяка стъпка се превръща във вход за следващата, стеснявайки търсенето.
 * 4. **Извличане на резултата:** След като всички стъпки са оценени,
 * функцията итерира през финалните намерени възли:
 * - Ако заявката изисква атрибут, се извлича стойността на този атрибут за всеки възел.
 * - Ако заявката търси таг, се събира и обединява цялото му текстово съдържание
 * (чрез `extractElementText`).
 * * @param query Пълният стринг на XPath заявката, подаден от потребителя през CLI.
 * @return Вектор от стрингове, съдържащ намерените текстови стойности или стойности на атрибути.
 * Ако търсенето не намери съвпадения или документът е празен, се връща празен вектор.
 */
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
