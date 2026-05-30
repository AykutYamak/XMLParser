#pragma once
#include <string>
#include "XmlNode.h"

/**
 * @brief Клас, представляващ текстов възел в XML дървото.
 * * Наследява абстрактния базов клас XmlNode. Този клас се използва за съхраняване
 * на същинското текстово съдържание, което се намира между отварящия и затварящия таг
 * на даден XML елемент (например текстът "John" в <name>John</name>).
 */
class XmlText : public XmlNode
{
private:
	std::string content; ///< Същинското текстово съдържание на възела.
public:
	/**
	 * @brief Конструктор по подразбиране.
	 * Създава празен текстов възел (без съдържание).
	 */

	XmlText() : XmlNode(), content(""){}

	/**
	 * @brief Конструктор с параметър за текст.
	 * @param textContent Текстът, с който да бъде инициализиран възелът.
	 */
	XmlText(const std::string& textContent) : XmlNode(), content(textContent) {}

	// Забраняваме копирането, за да сме консистентни с базовия клас
	XmlText(const XmlText& other) = delete;
	XmlText& operator=(const XmlText& other) = delete;

	/**
	 * @brief Деструктор по подразбиране.
	 */
	~XmlText() override = default;

	/**
	 * @brief Извежда текстовото съдържание в подадения изходен поток.
	 * @param os Изходният поток (напр. std::cout или std::ofstream).
	 * @param indent Отстъпът се подава за съвместимост с интерфейса, но често
	 * текстът се принтира директно до тага без допълнителен отстъп.
	 */
	void print(std::ostream& os,int indent = 0) const override;
	
	/**
	 * @brief Създава точно копие на текстовия възел.
	 * @return Указател към новия обект XmlText, съдържащ същия текст.
	 * Паметта трябва да се освободи от извикващия.
	 */
	XmlNode* clone() const override;

	/**
	 * @brief Връща съдържанието на текстовия възел.
	 * @return Константна референция към низа (string), съдържащ текста.
	 */
	const std::string& getContent() const { return content; }
};

