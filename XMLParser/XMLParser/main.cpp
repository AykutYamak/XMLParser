#include <iostream>
#include <vector>
#include <string>
#include "XmlCore/XmlDocument.h"
#include "Exceptions/Exception.h"
#include "CLI/Commands.h"

struct EntryCommand {
    std::string name;
    Command* command;
};

int main() {
    XmlDocument doc;
    std::vector<EntryCommand> menu;
    menu.push_back({ "print", new PrintCommand() });
    menu.push_back({ "save", new SaveCommand() });
    menu.push_back({ "xpath", new XPathCommand() });
    menu.push_back({ "select", new SelectCommand() });
    menu.push_back({ "set", new SetCommand() });
    menu.push_back({ "delete", new DeleteCommand() });
    menu.push_back({ "children", new ChildrenCommand() });
    menu.push_back({ "child", new ChildCommand() });
    menu.push_back({ "newChild", new NewChildCommand() });
    menu.push_back({ "text", new TextCommand() });

    std::string inputLine;

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, inputLine);
        size_t firstChar = inputLine.find_first_not_of(" \t\r\n");
        if (firstChar == std::string::npos) continue;
        inputLine = inputLine.substr(firstChar);

        std::string commandName = "";
        std::string args = "";
           
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

        if (commandName == "exit")
        {
            std::cout << "Exiting XML Parser..." << std::endl;
            break;
        }
        else if (commandName == "help")
        {
            std::cout << "Available command: " << std::endl;
            std::cout << " open <file> - Loads an XML file into memory" << std::endl;
            std::cout << " close - Closes the currently opened file" << std::endl;
            std::cout << " exit - Quits the program" << std::endl;
            size_t menuCount = menu.size();
            for (size_t i = 0; i < menuCount; i++)
            {
                std::cout << "  " << menu[i].name << std::endl;
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
                std::cout << "Successfully loaded " << args << std::endl;
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
            std::cout << "Document closed and memory cleared!" << std::endl;
            continue;
        }
        bool commandFound = false;
        size_t menuCount = menu.size();
        for (size_t i = 0; i < menuCount; i++)
        {
            if (menu[i].name == commandName)
            {
                menu[i].command->execute(doc, args);
                commandFound = true;
                break;
            }
        }
        if (!commandFound)
        {
            std::cout << "Unknown command: '" << commandName << "'. Type 'help' to see available commands." << std::endl;
        }
    }
    
    size_t menuCount = menu.size();
    for (size_t i = 0; i < menuCount; i++)
    {
        delete menu[i].command;
    }

	return 0;
}