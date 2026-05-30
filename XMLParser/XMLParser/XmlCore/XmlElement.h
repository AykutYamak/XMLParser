#pragma once
#include "XmlNode.h"
#include <string>
#include <vector>

/**
 * @brief Структура, представяща единичен XML атрибут.
 * Съхранява двойка от ключ (име на атрибута) и неговата стойност.
 */
struct XmlAttribute {
	std::string key; ///< Името (ключът) на атрибута  
	std::string value; ///< Стойността на атрибута
};

/**
 * @brief Клас, представящ XML елемент (таг) в дървото.
 * * Наследява XmlNode. Може да съдържа име (напр. "person"), списък от атрибути
 * (напр. id="1") и вектор от дещерни възли (други елементи или текст).
 */
class XmlElement : public XmlNode {
private:
	std::string name; ///< Името на XML тага.
	std::vector<XmlAttribute> attributes; ///< Списък с атрибутите на елемента.
	std::vector<XmlNode*> children; ///< Списък с дещерните възли.
public:
	/**
	 * @brief Конструктор по подразбиране. Създава празен елемент.
	 */
	XmlElement() : XmlNode(), name(""){}
	
	/**
	 * @brief Конструктор с параметър.
	 * @param elementName Името на XML тага, който се създава.
	 */
	XmlElement(const std::string& elementName) : XmlNode(), name(elementName) {}

	// Забраняваме копирането и присвояването за безопасно управление на паметта
	XmlElement(const XmlElement& other) = delete;
	XmlElement& operator=(const XmlElement& other) = delete;
	
	/**
	 * @brief Деструктор.
	 * Отговаря за рекурсивното изтриване (delete) на всички дещерни възли
	 * в children вектора, за да предотврати изтичане на памет.
	 */
	~XmlElement() override;
		
	/**
	 * @brief Извежда елемента (тага, атрибутите и децата му) в изходен поток.
	 * @param os Изходният поток (напр. std::cout или std::ofstream).
	 * @param indent Броят интервали за отстъп, спрямо дълбочината в дървото.
	 */
	void print(std::ostream& os, int indent = 0) const override;
	
	/**
	 * @brief Създава динамично (дълбоко) копие на текущия елемент.
	 * Копира името, атрибутите и рекурсивно клонира всички дещерни възли.
	 * @return Указател към новия обект. Паметта се управлява от извикващия.
	 */
	XmlNode* clone() const override;

	/**
	 * @brief Добавя нов дещерен възел към текущия елемент.
	 * Автоматично задава текущия елемент като родител на добавеното дете.
	 * @param child Указател към възела (XmlElement или XmlText), който ще бъде добавен.
	 */
	void addChild(XmlNode* child);

	/**
	 * @brief Връща списък с всички атрибути на елемента.
	 * @return Константна референция към вектора с атрибути.
	 */
	const std::vector<XmlAttribute>& getAttributes() const {
		return attributes;
	}

	/**
	 * @brief Добавя нов атрибут или обновява стойността на съществуващ.
	 * @param key Името на атрибута.
	 * @param value Стойността на атрибута.
	 */
	void addAttribute(const std::string& key, const std::string& value);
	
	/**
	 * @brief Връща името на XML тага.
	 * @return Константна референция към низа, съдържащ името.
	 */
	const std::string& getName() const { return this->name; }

	/**
	 * @brief Премахва атрибут по зададен ключ, ако съществува.
	 * @param key Името на атрибута, който трябва да бъде премахнат.
	 */
	void removeAttribute(const std::string& key) {
		size_t attributesSize = attributes.size();
		for (size_t i = 0; i < attributesSize; ++i) {
			if (attributes[i].key == key){
				attributes.erase(attributes.begin() + i);
				break;
			}
		}
	}

	/**
	 * @brief Оператор за индексиране по ключ на атрибут.
	 * Позволява лесен достъп и промяна на стойности (напр. element["id"] = "5").
	 * Ако атрибут с такъв ключ не съществува, той автоматично се създава.
	 * @param key Името на атрибута.
	 * @return Референция към стойността на атрибута (позволява промяна).
	 */
	std::string& operator[](const std::string& key) {
		for (XmlAttribute& attribute : attributes){
			if (attribute.key == key){
				return attribute.value;
			}
		}
		attributes.push_back({ key,"" });
		return attributes.back().value;
	}

	/**
	 * @brief Безопасно извлича стойността на атрибут (read-only).
	 * За разлика от operator[], тази функция не създава нов атрибут, ако ключът липсва.
	 * @param key Името на атрибута.
	 * @return Стойността на атрибута или празен низ (""), ако не е намерен.
	 */
	std::string getAttributeValue(const std::string& key) const
	{
		for (const XmlAttribute& attribute : attributes){
			if (attribute.key == key){
				return attribute.value;
			}
		}
		return "";
	}

	/**
	 * @brief Шаблонна функция, която връща всички дещерни възли от специфичен тип.
	 * Използва dynamic_cast за безопасно филтриране (напр. само XmlElement или само XmlText).
	 * @tparam T Типът на възлите, които търсим.
	 * @return Вектор от указатели към филтрираните възли.
	 */
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

	/**
     * @brief Вграден клас Итератор за безопасно и удобно обхождане на XML дървото.
     * * @details Имплементирането на собствен итератор енкапсулира (скрива) вътрешното представяне 
     * на децата (`std::vector<XmlNode*>`). Най-голямото предимство на този клас е, че 
     * предоставяйки методите `begin()` и `end()`, той прави `XmlElement` съвместим със 
     * стандартните C++ range-based for цикли (напр. `for(auto child : *element)`).
     * Това прави кода в целия проект много по-четим и устойчив на грешки, в сравнение 
     * с ръчното индексиране.
     */
	class Iterator {
	private:
		std::vector<XmlNode*>* collection; ///< Указател към колекцията от деца.
		size_t index; ///< Текуща позиция на итератора.
	public:
		/**
		 * @brief Конструктор за итератора.
		 */
		Iterator(std::vector<XmlNode*>* coll, size_t index) : collection(coll), index(index){}
		
		/** @brief Оператор за преминаване към следващия елемент. */
		Iterator& operator++() { ++index; return *this; }
		
		/** @brief Оператор за дереференциране. Връща референция към указателя на възела. */
		XmlNode*& operator*() { return (*collection)[index]; }
		
		/** @brief Оператор за сравнение (проверка за край на колекцията). */
		bool operator!=(const Iterator& other) const { return this->index != other.index; }
	};

	/** @brief Връща итератор, сочещ към първото дете на елемента. */	
	Iterator begin() { return Iterator(&children, 0); }

	/** @brief Връща итератор, сочещ след последното дете на елемента. */
	Iterator end() { return Iterator(&children, children.size()); }
};