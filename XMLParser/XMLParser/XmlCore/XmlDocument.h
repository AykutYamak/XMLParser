#pragma once
#include "XmlElement.h"
#include "IdDictionary.h"

/**
 * @brief Главен клас, представляващ целия XML документ.
 * * @details Този клас управлява жизнения цикъл на XML дървото (започвайки от корена),
 * поддържа речника за бързо търсене по ID и предоставя основния интерфейс за
 * зареждане, запазване и търсене (XPath) във файла.
 */
class XmlDocument {
private:
	XmlElement* root; ///< Указател към коренния елемент на XML дървото.
	IdDictionary registry; ///< Речник за бързо търсене и достъп до елементи по ID.
	int autoIdCounter; ///< Брояч, използван за генериране на уникални ID-та (напр. gen_0).
	std::string openedFilePath; ///< Пътят към текущо отворения файл.

	/**
	 * @brief Помощна функция за прехвърляне на ресурси (Move семантика).
	 * @param other Обектът, чиито ресурси ще бъдат "откраднати" (присвоени).
	 */
	void moveFrom(XmlDocument&& other);
	
	// --- Помощни функции за парсване на документа ---

	/** @brief Премахва излишните интервали (whitespaces) в началото и края на низ. */
	std::string trim(const std::string& str) const;

	/** * @brief Гарантира уникалност на ID-то.
	 * Ако подаденото ID вече съществува или е празно, генерира ново (напр. добавя суфикс_1 или gen_N).
	 */
	std::string ensureUniqueId(const std::string& parseId);

	/** @brief Парсва съдържанието на отварящ таг и създава съответния XmlElement с атрибутите му. */
	XmlElement* parseOpeningTag(const std::string& content) const;

	// --- Помощни функции за XPath търсачката ---
	
	/** @brief Разделя XPath заявката на отделни стъпки (напр. по символа '/'). */
	std::vector<std::string> splitXpath(const std::string& query) const;

	/** @brief Оценява една конкретна стъпка от XPath заявката върху текущ списък от възли. */
	std::vector<XmlElement*> evaluateXpathStep(const std::vector<XmlElement*>& currentNodes, const std::string& step) const;

	/** @brief Извлича и обединява цялото текстово съдържание (XmlText възли) на даден елемент. */
	std::string extractElementText(XmlElement* element) const;

public:
	/**
	 * @brief Конструктор по подразбиране. Създава празен документ.
	 */
	XmlDocument() : root(nullptr), autoIdCounter(0), openedFilePath("") {}

	// Забраняваме копирането
	XmlDocument(const XmlDocument& other) = delete;
	XmlDocument& operator=(const XmlDocument& other) = delete;

	/**
	 * @brief Деструктор. Изчиства паметта, като изтрива корена (което каскадно изтрива цялото дърво).
	 */
	~XmlDocument();

	/**
	 * @brief Move конструктор. Позволява ефективно прехвърляне на дървото без копиране.
	 */
	XmlDocument(XmlDocument&& other);
	
	/**
	 * @brief Move оператор за присвояване.
	 */
	XmlDocument& operator=(XmlDocument&& other);

	/**
	 * @brief Зарежда и парсва XML файл от диска.
	 * @param filename Пътят към файла за четене (ако не съществува, създава празен).
	 * @throw ParseException При синтактична грешка в XML структурата.
	 * @throw FileException При проблем с достъпа до файла.
	 */
	void load(const std::string& filename);

	/**
	 * @brief Запазва промените върху текущо отворения файл.
	 * @throw FileException Ако няма отворен файл.
	 */
	void save() const;
	
	/**
	 * @brief Запазва документа в нов файл.
	 * @param filename Пътят, където да бъде записан новият файл.
	 * @throw FileException Ако документът е празен или файлът не може да бъде създаден.
	 */
	void saveAs(const std::string& filename) const;

	/**
	 * @brief Изчиства текущия документ от паметта (унищожава дървото и речника).
	 */
	void clear();

	/**
	 * @brief Регистрира елемент в ID речника за бързо търсене.
	 * @param id Уникалният идентификатор.
	 * @param element Указател към елемента.
	 */
	void registerElement(const std::string& id, XmlElement* element) {
		registry.add(id, element);
	}

	/** @brief Връща указател към корена на XML дървото. */
	XmlElement* getRoot() const { return root; }

	/** * @brief Търси елемент по неговото ID.
	 * @return Указател към елемента или nullptr, ако не е намерен.
	 */
	XmlElement* getElementById(const std::string& id) const { return registry.get(id); }

	/** @brief Връща пътя на текущо отворения файл. */
	const std::string& getFilePath() const { return openedFilePath; }

	/**
	 * @brief Изпълнява XPath заявка върху документа.
	 * @param query Стринг, съдържащ XPath израза (поддържа /, [], @ и =).
	 * @return Вектор от стрингове (текстовите резултати от намерените възли/атрибути).
	 */
	std::vector<std::string> xpath(const std::string& query) const;
};