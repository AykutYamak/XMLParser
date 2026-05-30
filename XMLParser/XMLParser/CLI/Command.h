#pragma once
#include <string>
#include "../XmlCore/XmlDocument.h"

/**
 * @brief Базов интерфейс за всички CLI команди в приложението.
 * @details Използва Command Design Pattern, което позволява лесно добавяне
 * на нови команди без промяна по основния цикъл на програмата (Open-Closed Principle).
 */
class Command {
public:
	virtual ~Command() {}
    
    /**
     * @brief Изпълнява съответната команда върху подадения XML документ.
     * @param doc Референция към текущо заредения XML документ.
     * @param args Аргументите на командата, подадени от потребителя (като стринг).
     */
	virtual void execute(XmlDocument& doc, const std::string& args) = 0;
};
