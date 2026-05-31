#pragma once
#include <string>
#include <vector>
#include "XmlElement.h"

/**
 * @brief Структура, представляваща единичен запис в ID речника.
 * Съвързва уникален идентификатор (текст) с конкретен XML елемент в паметта.
 */
struct IdEntry {
	std::string id;        ///< Уникалният идентификатор (ID) на елемента.
	XmlElement* element;   ///< Указател към самия XML елемент в дървото.
};

/**
 * @brief Речник за регистриране и бързо търсене на XML елементи по тяхното ID.
 * * @details Този клас поддържа плосък списък (вектор) от всички елементи в дървото,
 * които притежават атрибут `id`. Това позволява изпълнението на команди като `select`
 * или `set` да се случва бързо, без нужда от рекурсивно обхождане на цялото XML дърво.
 */
class IdDictionary {
private:
	std::vector<IdEntry> entries; ///< Вътрешен контейнер, съхраняващ регистрираните записи.
public:
	/**
	 * @brief Проверява дали даден идентификатор вече съществува в речника.
	 * @param id Идентификаторът, който търсим.
	 * @return `true`, ако ID-то е намерено, и `false`, ако такова ID не съществува.
	 */
	bool contains(const std::string& id) const {
		for (const IdEntry& entry : entries )
		{
			if (entry.id == id)
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief Регистрира нов елемент в речника със съответното му ID.
	 * @details Методът не прави проверка за дубликати. Гарантирането на уникалност
	 * е отговорност на парсъра (напр. чрез метода ensureUniqueId).
	 * @param id Уникалният идентификатор на елемента.
	 * @param element Указател към XML елемента, който регистрираме.
	 */
	void add(const std::string& id, XmlElement* element) {
		entries.push_back({ id,element });
	}

	/**
	 * @brief Извлича указател към елемент по зададено ID.
	 * @param id Идентификаторът на търсения елемент.
	 * @return Указател към намерения XmlElement, или `nullptr`, ако не бъде открит такъв.
	 */
	XmlElement* get(const std::string& id) const {
		for (const IdEntry& entry : entries)
		{
			if (entry.id == id)
			{
				return entry.element;
			}
		}
		return nullptr;
	}

	/**
	 * @brief Изчиства всички записи от речника.
	 * @details ВНИМАНИЕ: Този метод премахва само самите записи (референциите) от речника,
	 * но НЕ изтрива самите XML елементи от паметта. Жизненият цикъл на елементите
	 * се управлява изцяло от дървовидната структура (йерархията от родители и деца).
	 */
	void clear() {
		entries.clear();
	}
};