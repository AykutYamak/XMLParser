#pragma once
#include <ostream>

/**
 * @brief Абстрактен базов клас за всички възли в XML дървото.
 * * Този клас дефинира общия интерфейс за всеки един компонент от XML документа
 * (например елементи или обикновен текст). Съдържа информация за родителския възел
 * и налага имплементацията на основни полиморфни методи от наследниците си.
 */
class XmlNode {
protected:
	XmlNode* parent; ///< Указател към родителския възел (nullptr, ако е корен).
public:
	/**
	 * @brief Конструктор по подразбиране.
	 * Инициализира възела без родител.
	 */
	XmlNode() : parent(nullptr){}

	/**
	 * @brief Виртуален деструктор.
	 * Гарантира правилното изчистване на паметта при изтриване на обекти
	 * чрез указател към базовия клас.
	 */
	virtual ~XmlNode() = default;
	
	// Забраняваме копирането и преместването (Rule of Deleted Functions),
	// за да предотвратим проблеми с паметта при работа с полиморфни обекти.	
	XmlNode(const XmlNode& other) = delete;
	XmlNode& operator=(const XmlNode& other) = delete;
	XmlNode(XmlNode&& other) = delete;
	XmlNode& operator=(XmlNode&& other) = delete;

	/**
	 * @brief Извежда съдържанието на възела в подадения изходен поток.
	 * @param os Изходният поток (напр. std::cout или std::ofstream), в който ще се пише.
	 * @param indent Броят интервали за отстъп, използвани за форматиран (pretty-print) изход.
	 */
	virtual void print(std::ostream& os,int indent = 0) const = 0;
	
	/**
	 * @brief Създава точно (дълбоко) копие на текущия възел.
	 * @return Указател към новосъздаденото копие. Паметта трябва да се управлява от извикващия.
	 */
	virtual XmlNode* clone() const = 0;

	/**
	 * @brief Връща родителския възел на текущия обект.
	 * @return Указател към родителя или nullptr, ако възелът е корен.
	 */
	XmlNode* getParent() const { return parent; }
	
	/**
	 * @brief Задава нов родителски възел.
	 * @param p Указател към новия родителски възел.
	 */
	void setParent(XmlNode* p) { parent = p; }
};