#include "Tests/doctest.h"
#include "XmlCore/XmlDocument.h"
#include "Exceptions/Exception.h"
#include <fstream>
#include <string>
#include <vector>

TEST_CASE("XmlDocument: File Loading and Parsing") {
    XmlDocument doc;
    std::string tempXmlFile = "test_parser_input.xml";

    SUBCASE("Loading a valid, well-formed XML structure") {
        // 1. Създаваме тестов XML файл с комплексна структура
        std::ofstream outFile(tempXmlFile);
        REQUIRE(outFile.is_open());
        outFile << "<library>\n"
                << "    <book id=\"b1\">\n"
                << "        <title>Introduction to C++</title>\n"
                << "    </book>\n"
                << "    <book id=\"b2\" />\n" // Самозатварящ се таг
                << "</library>\n";
        outFile.close();

        // 2. Зареждаме го през парсъра
        doc.load(tempXmlFile);

        // 3. Проверяваме дали дървото е построено вярно
        REQUIRE(doc.getRoot() != nullptr);
        CHECK(doc.getRoot()->getName() == "library");

        std::vector<XmlElement*> books = doc.getRoot()->getChildrenOfType<XmlElement>();
        REQUIRE(books.size() == 2);
        CHECK(books[0]->getName() == "book");
        CHECK(books[0]->getAttributeValue("id") == "b1");
        CHECK(books[1]->getAttributeValue("id") == "b2");

        // Почистваме (зануляваме файла)
        std::ofstream clearFile(tempXmlFile);
        clearFile.close();
    }

    SUBCASE("Handling missing root and generating document_wrapper") {
        // XML стандартът изисква 1 корен. Подаваме файл с 2 корена на нулево ниво.
        std::ofstream outFile(tempXmlFile);
        outFile << "<first_root>\n</first_root>\n" 
                << "<second_root>\n</second_root>\n";
        outFile.close();

        doc.load(tempXmlFile);

        // Парсърът трябва интелигентно да е създал спасителния обвивач
        REQUIRE(doc.getRoot() != nullptr);
        CHECK(doc.getRoot()->getName() == "document_wrapper");

        std::vector<XmlElement*> roots = doc.getRoot()->getChildrenOfType<XmlElement>();
        CHECK(roots.size() == 2);

        std::ofstream clearFile(tempXmlFile);
        clearFile.close();
    }

    SUBCASE("Throwing ParseException on broken XML") {
        std::ofstream outFile(tempXmlFile);
        outFile << "<root>\n    <child> Missing closing root tag!\n"; // Счупен XML
        outFile.close();

        // Очакваме парсърът да хване грешката и да хвърли изключение
        CHECK_THROWS(doc.load(tempXmlFile));

        std::ofstream clearFile(tempXmlFile);
        clearFile.close();
    }
}

TEST_CASE("XmlDocument: Move Semantics (Rule of 5 Validation)") {
    XmlDocument sourceDoc;
    std::string tempFile = "test_move.xml";

    std::ofstream outFile(tempFile);
    outFile << "<catalog><item id=\"1\">Data</item></catalog>\n";
    outFile.close();

    sourceDoc.load(tempFile);
    XmlElement* originalRoot = sourceDoc.getRoot();
    REQUIRE(originalRoot != nullptr);

    SUBCASE("Move Constructor transfers ownership safely") {
        // Извикваме Move конструктора
        XmlDocument destinationDoc(std::move(sourceDoc));

        // 1. Новият документ трябва да е взел корена
        CHECK(destinationDoc.getRoot() == originalRoot);
        CHECK(destinationDoc.getFilePath() == tempFile);

        // 2. Старият документ задължително трябва да е занулен
        CHECK(sourceDoc.getRoot() == nullptr);
        CHECK(sourceDoc.getFilePath() == "");
    }

    SUBCASE("Move Assignment Operator cleans old and takes new resource") {
        XmlDocument destinationDoc;

        // Прехвърляме чрез оператора =
        destinationDoc = std::move(sourceDoc);

        CHECK(destinationDoc.getRoot() == originalRoot);
        CHECK(sourceDoc.getRoot() == nullptr);
    }

    std::ofstream clearFile(tempFile);
    clearFile.close();
}

TEST_CASE("XmlDocument: The XPath Engine Pipeline") {
    XmlDocument doc;
    std::string xpathFile = "test_xpath.xml";

    std::ofstream outFile(xpathFile);
    outFile << "<fmi>\n"
            << "    <student id=\"s1\" status=\"active\">\n"
            << "        <name>Ivan</name>\n"
            << "        <course>1</course>\n"
            << "    </student>\n"
            << "    <student id=\"s2\">\n"
            << "        <name>Maria</name>\n"
            << "        <course>2</course>\n"
            << "    </student>\n"
            << "    <professor id=\"p1\">\n"
            << "        <name>Dr. Petrov</name>\n"
            << "    </professor>\n"
            << "</fmi>\n";
    outFile.close();

    doc.load(xpathFile);

    SUBCASE("Simple tag path selection and text extraction") {
        std::vector<std::string> names = doc.xpath("student/name");
        REQUIRE(names.size() == 2);
        CHECK(names[0] == "Ivan");
        CHECK(names[1] == "Maria");
    }

    SUBCASE("(*) selection") {
        // Търсим името на абсолютно всички под-тагове на fmi
        std::vector<std::string> allNames = doc.xpath("*/name");
        REQUIRE(allNames.size() == 3);
        CHECK(allNames[2] == "Dr. Petrov");
    }

    SUBCASE("Index [n] predicate filtering") {
        // Вземаме само втория студент (индекс 1)
        std::vector<std::string> secondStudentName = doc.xpath("student[1]/name");
        REQUIRE(secondStudentName.size() == 1);
        CHECK(secondStudentName[0] == "Maria");
    }

    SUBCASE("Attribute presence [@attr] predicate filtering") {
        // Само студентите, които имат атрибут 'status'
        std::vector<std::string> activeStudents = doc.xpath("student[@status]/name");
        REQUIRE(activeStudents.size() == 1);
        CHECK(activeStudents[0] == "Ivan");
    }

    SUBCASE("Target attribute value extraction") {
        // Искаме да извлечем самите ID-та на студентите
        std::vector<std::string> ids = doc.xpath("student[@id]");
        REQUIRE(ids.size() == 2);
        CHECK(ids[0] == "s1");
        CHECK(ids[1] == "s2");
    }

    SUBCASE("Child value [child=\"value\"] complex predicate filtering") {
        // Намери студента, чийто курс е точно "2", и ми върни неговото име
        std::vector<std::string> matched = doc.xpath("student[course=\"2\"]/name");
        REQUIRE(matched.size() == 1);
        CHECK(matched[0] == "Maria");
    }

    std::ofstream clearFile(xpathFile);
    clearFile.close();
}