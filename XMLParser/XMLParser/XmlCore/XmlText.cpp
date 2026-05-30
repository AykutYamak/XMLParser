#include <iostream>
#include <string>
#include "XmlText.h"
#include "../Exceptions/Exception.h"

/**
 * @brief Извежда текстовото съдържание в подадения изходен поток.
 * * Създава низ от празни пространства, отговарящ на зададения отстъп,
 * след което принтира същинския текст, последван от нов ред.
 * * @param os Изходният поток (напр. std::cout или файлов поток), в който ще се записва.
 * @param indent Броят интервали, които ще бъдат добавени преди текста.
 * @throw Exception Хвърля изключение, ако подаденият отстъп е отрицателно число.
 */
void XmlText::print(std::ostream& os, int indent) const {
	if (indent < 0)
	{
		throw Exception("Indentation can't be a negative number in XmlText::print");
	}
	std::string spaces(indent, ' ');

	os << spaces << content << std::endl;
}

/**
 * @brief Създава динамично (дълбоко) копие на текущия текстов възел.
 * * @return Указател към новосъздадения обект XmlText, съдържащ същото текстово съдържание.
 * Отговорност на извикващия код е да освободи заделената памет чрез delete.
 */
XmlNode* XmlText::clone() const{

	return new XmlText(this->content);
}
