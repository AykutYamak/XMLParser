#include "Tests/doctest.h"
#include "XmlCore/XmlText.h" 
#include "Exceptions/Exception.h" 
#include <fstream>
#include <string>

TEST_CASE("XmlText: Constructors and Content") {
    SUBCASE("Default constructor creates an empty text node") {
        XmlText emptyText;
        CHECK(emptyText.getContent() == "");
    }

    SUBCASE("Parameterized constructor sets correct content") {
        XmlText text("FMI Sofia");
        CHECK(text.getContent() == "FMI Sofia");
    }
}

TEST_CASE("XmlText: Clone functionality (Deep Copy)") {
    XmlText original("Clone me");
    XmlNode* clonedNode = original.clone();

    XmlText* clonedText = dynamic_cast<XmlText*>(clonedNode);
    REQUIRE(clonedText != nullptr);

    CHECK(clonedText->getContent() == "Clone me");
    CHECK(clonedText != &original);

    delete clonedNode;
}

TEST_CASE("XmlText: Print method and Exception handling") {
    XmlText textNode("Hello World");
    std::string tempFilename = "test_temp_output.txt";

    SUBCASE("Print outputs correctly with indentation to a file") {
        // 1. Отваряме файла за писане (това автоматично създава или презаписва файла)
        std::ofstream outFile(tempFilename);
        REQUIRE(outFile.is_open());

        textNode.print(outFile, 2);
        outFile.close();

        // 2. Отваряме същия файл за четене
        std::ifstream inFile(tempFilename);
        REQUIRE(inFile.is_open());

        std::string resultLine;
        std::getline(inFile, resultLine);
        inFile.close();

        CHECK(resultLine == "  Hello World");

        // 3. Отваряме файла отново в режим на писане (което изтрива съдържанието му) 
        // и го затваряме веднага. Файлът остава на диска, но е празен.
        std::ofstream clearFile(tempFilename);
        clearFile.close();
    }

    SUBCASE("Print throws Exception on negative indentation") {
        std::ofstream outFile(tempFilename);

        CHECK_THROWS_AS(textNode.print(outFile, -1), const Exception&);

        outFile.close();

        std::ofstream clearFile(tempFilename);
        clearFile.close();
    }
}