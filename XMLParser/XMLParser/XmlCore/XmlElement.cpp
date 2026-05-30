#include "XmlElement.h"
#include "../Exceptions/Exception.h"
#include <iostream>

/**
 * @brief Извежда елемента, неговите атрибути и съдържание във форматиран вид.
 * * @details Логиката автоматично разпознава дали елементът е празен (няма деца)
 * и в такъв случай го принтира като самозатварящ се (напр. `<tag />`). Ако има деца,
 * отваря тага, извиква рекурсивно `print` за всяко дете с увеличен отстъп (4 интервала),
 * и накрая принтира затварящия таг.
 * * @param os Изходният поток, в който се записват данните.
 * @param indent Броят интервали за отместване на текущия елемент.
 * @throw Exception Хвърля изключение, ако отстъпът е отрицателно число.
 */
void XmlElement::print(std::ostream& os, int indent) const
{
    if (indent<0)
    {
        throw Exception("Indentation can't be a negative number in XmlElement::print");
    }
    std::string spaces(indent, ' ');
    os << spaces << "<" << name;
    
    for (const XmlAttribute& attr : attributes) {
        os << " " << attr.key << "=\"" << attr.value << "\"";
    }
    if (children.empty())
    {
        os << " />" << std::endl;
    }
    else
    {
        os << ">" << std::endl;
        for (XmlNode* child : children) {
            child->print(os ,indent + 4);
        }
        os << spaces << "</" << name << ">" << std::endl;
    }

}
/**
 * @brief Създава динамично дълбоко копие (Deep Copy) на текущия XML елемент и цялото му поддърво.
 * * @details Използването на виртуална функция `clone()` е класически OOP шаблон (Virtual Constructor Pattern).
 * Той е абсолютно задължителен тук, тъй като работим с полиморфна колекция от указатели
 * към базовия клас (`std::vector<XmlNode*>`). Ако се опитаме да използваме стандартен
 * Copy Constructor, ще се получи т.нар. "Object Slicing" (изрязване на обекта) и ще
 * загубим специфичните данни на наследниците.
 * * **Как работи:**
 * 1. Създава нов `XmlElement` със същото име.
 * 2. Копира директно всички атрибути (тъй като векторът от `XmlAttribute` се копира по стойност).
 * 3. Обхожда рекурсивно всички дещерни възли (`children`) и извиква техния собствен метод `clone()`.
 * По този начин се изгражда напълно независимо ново дърво в паметта, което не споделя
 * никакви указатели с оригиналното.
 * * @return Указател към новосъздадения корен на копираното поддърво. Паметта трябва да
 * се управлява от извикващата функция.
 */
XmlNode* XmlElement::clone() const
{
    XmlElement* copy = new XmlElement(this->name);
    copy->attributes = this->attributes;
    for (XmlNode* child : children)
    {
        copy->addChild(child->clone());
    }
    return copy;
}

/**
 * @brief Добавя възел като дете на текущия елемент и обновява връзките.
 * * @details Освен че добавя възела във вътрешния вектор `children`, методът
 * автоматично задава текущия обект (`this`) като родител на добавеното дете.
 * * @param child Указател към възела, който трябва да бъде добавен.
 * @throw Exception Хвърля изключение, ако се направи опит за добавяне на nullptr.
 */
void XmlElement::addChild(XmlNode* child) {
    if (child)
    {
        children.push_back(child);
        child->setParent(this);
    }
    else
    {
        throw Exception("Can't add a null child to XML Element: " + name);
    }
}

/**
 * @brief Добавя нов атрибут към елемента (без проверка за дубликати).
 * * @param key Името на атрибута.
 * @param value Стойността на атрибута.
 * @throw Exception Хвърля изключение, ако подаденият ключ е празен низ.
 */
void XmlElement::addAttribute(const std::string& key, const std::string& value) {
    if (key.empty())
    {
        throw Exception("Can't add an attribute with an empty key ot element: " + name);
    }
    attributes.push_back({key,value});

}

/**
 * @brief Деструктор на елемента, гарантиращ липсата на изтичане на памет.
 * * @details Тъй като XML дървото е йерархична структура, изградена от динамично заделени
 * обекти (чрез `new`), паметта трябва да се освобождава внимателно.
 * Този деструктор реализира **каскадно (рекурсивно) изтриване**.
 * * Когато се извика `delete` върху коренния елемент (например от деструктора на `XmlDocument`),
 * този деструктор се активира и завърта цикъл през всички деца във вектора `children`.
 * Извикването на `delete child` вътре в цикъла автоматично извиква деструкторите на
 * самите деца (които изтриват техните деца и т.н., чак до листата на дървото).
 * Тъй като деструкторът в базовия клас `XmlNode` е маркиран като `virtual`, C++ гарантира,
 * че винаги ще се извика правилният деструктор (`XmlElement` или `XmlText`), дори когато
 * трием през указател към базовия клас.
 */
XmlElement::~XmlElement() {
    for (XmlNode* child : children) {
        delete child;
    }
    children.clear();
}
