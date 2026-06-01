/**
 * @mainpage XML Parser - Документация на курсов проект
 * * @section intro_sec Въведение
 * Това е конзолно приложение, разработено на C++, което предоставя възможност за
 * зареждане, редактиране и запазване на XML документи. Проектът имплементира
 * собствена дървовидна структура и не използва външни библиотеки за парсване.
 * * @section features_sec Основни функционалности
 * - Пълно управление на паметта с каскадни деструктори и Move семантики.
 * - Поддръжка на XPath заявки за бързо търсене на възли и атрибути.
 * - Използване на Command Design Pattern за лесно разширяване на CLI интерфейса.
 * - Автоматично управление на уникални идентификатори (ID регистър).
 */

#include <iostream>
#include <vector>
#include <string>
#include "XmlCore/XmlDocument.h"
#include "Exceptions/Exception.h"
#include "CLI/Commands.h"
#include "doctest.h"
 /**
  * @brief Структура за съхранение на регистрираните команди в менюто.
  * @details Свързва текстовото име на командата, указател към обекта,
  * който я изпълнява, и кратко описание за help менюто.
  */
struct EntryCommand {
    std::string name;        ///< Името на командата (напр. "print")
    Command* command;        ///< Указател към обекта, имплементиращ логиката
    std::string description; ///< Описание и очаквани параметри (за help менюто)
};

/**
 * @brief Главна функция (Entry point) на приложението.
 * @details Инициализира XML документа и списъка с налични команди.
 * Стартира безкраен цикъл, който чете потребителския
 * вход, разделя го на име на команда и аргументи, и динамично извиква
 * съответния Command обект. Съдържа защити срещу изключения (Exceptions)
 * и отговаря за безопасното освобождаване на паметта при изход.
 * @return 0 при успешно завършване на програмата.
 */
int main() {
    XmlDocument doc;
    std::vector<EntryCommand> menu;

    // Регистрация на всички поддържани команди
    menu.push_back({ "print", new PrintCommand(), "Prints the XML tree"});
    menu.push_back({ "save", new SaveCommand(), "Saves changes to the current file"});
    menu.push_back({ "saveas", new SaveAsCommand(), "Saves changes to a specific file. Usage: saveas <file>"});
    menu.push_back({ "xpath", new XPathCommand(), "Executes an XPath query. Usage: xpath <query>"});
    menu.push_back({ "select", new SelectCommand() ,"Selects an attribute by element ID. Usage: select <id> <key>"});
    menu.push_back({ "set", new SetCommand(), "Sets or updates an attribute. Usage: set <id> <key> <value>"});
    menu.push_back({ "delete", new DeleteCommand(), "Deletes an attribute by element ID. Usage: delete <id> <key>"});
    menu.push_back({ "children", new ChildrenCommand(), "Lists attributes of all children of an element. Usage: children <id>"});
    menu.push_back({ "child", new ChildCommand(), "Retrieves the child element at the specified index N. Usage child <id> <n>"});
    menu.push_back({ "newChild", new NewChildCommand() ,"Creates a new child element. Usage newChild <id>"});
    menu.push_back({ "text", new TextCommand() ,"Sets the inner text of an element. Usage: text <id> <text>"});

    std::string inputLine;

    // Главен цикъл на приложението
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, inputLine);
        size_t firstChar = inputLine.find_first_not_of(" \t\r\n");
        if (firstChar == std::string::npos) continue;
        inputLine = inputLine.substr(firstChar);

        std::string commandName = "";
        std::string args = "";
           
        // Разделяне на команда и аргументи
        size_t spacePos = inputLine.find(' ');
        if (spacePos != std::string::npos) 
        { 
            commandName = inputLine.substr(0, spacePos); 
            args = inputLine.substr(spacePos + 1);

            size_t argsFirst = args.find_first_not_of(" \t\r\n");
            if (argsFirst != std::string::npos)
            {
                args = args.substr(argsFirst);
            }
            else
            {
                args = "";
            }
        }
        else
        {
            commandName = inputLine;
        }

        // Обработка на вградените команди
        if (commandName == "exit")
        {
            std::cout << "Exiting the program..." << std::endl;
            break;
        }
        else if (commandName == "help")
        {
            std::cout << "Available commands: " << std::endl;
            std::cout << " open <file> - Loads an XML file into memory" << std::endl;
            std::cout << " close - Closes the currently opened file" << std::endl;
            std::cout << " exit - Quits the program" << std::endl;
            size_t menuCount = menu.size();
            for (size_t i = 0; i < menuCount; i++)
            {
                std::cout << "  " << menu[i].name << " - " << menu[i].description << std::endl;
            }
            continue;
        }
        else if(commandName == "open")
        {
            if (args.empty())
            {
                std::cout << "Error: Usage: open <filename>" << std::endl;
                continue;
            }
            try
            {
                doc.load(args);
                std::cout << "Successfully opened " << args << std::endl;
            }
            catch (const Exception& e)
            {
                std::cout << "Error: " << e.what() << std::endl;
            }
            continue;
        }
        else if (commandName == "close")
        {
            doc.clear();
            std::cout << "Document closed " << doc.getFilePath() << " and memory cleared!" << std::endl;
            continue;
        }

        // Търсене и изпълнение на динамичните команди от менюто
        bool commandFound = false;
        size_t menuCount = menu.size();
        for (size_t i = 0; i < menuCount; i++)
        {
            if (menu[i].name == commandName)
            {
                try 
                {
                    menu[i].command->execute(doc, args);
                }
                catch (const Exception& e) {
                    std::cout << "Command error: " << e.what() << std::endl;
                }
                catch (const std::exception& e) {
                    std::cout << "System error: " << e.what() << std::endl;
                }
                catch (...) {
                    std::cout << "An unknown error occured during execution." << std::endl;
                }
                commandFound = true;
                break;
            }
        }
        if (!commandFound)
        {
            std::cout << "Unknown command: '" << commandName << "'. Type 'help' to see available commands." << std::endl;
        }
    }

    // Безопасно освобождаване на паметта за командите при изход
    size_t menuCount = menu.size();
    for (size_t i = 0; i < menuCount; i++)
    {
        delete menu[i].command;
    }

	return 0;
}