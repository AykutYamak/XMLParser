#include "Tests/doctest.h"
#include "XmlCore/XmlElement.h"
#include "XmlCore/XmlText.h"
#include "Exceptions/Exception.h"
#include <vector>

TEST_CASE("XmlElement: Basic Initialization and Name") {
    XmlElement root("university");
    CHECK(root.getName() == "university");
}

TEST_CASE("XmlElement: Attribute Management") {
    XmlElement student("student");

    SUBCASE("Adding and getting attribute safely") {
        student.addAttribute("id", "54321");
        CHECK(student.getAttributeValue("id") == "54321");

        // Търсене на несъществуващ атрибут връща празен низ
        CHECK(student.getAttributeValue("age") == "");
    }

    SUBCASE("Operator[] reads existing and auto-creates missing attributes") {
        student.addAttribute("faculty", "FMI");

        // Четене на съществуващ
        CHECK(student["faculty"] == "FMI");

        // Писане на нов чрез оператора (Страничен ефект)
        student["year"] = "1";
        CHECK(student.getAttributeValue("year") == "1");

        // Автоматично създаване при четене на липсващ
        std::string missing = student["course"];
        CHECK(missing == "");
        CHECK(student.getAttributes().size() == 3); // faculty, year, course
    }

    SUBCASE("Removing an attribute") {
        student.addAttribute("status", "active");
        student.removeAttribute("status");
        CHECK(student.getAttributeValue("status") == "");
    }
}

TEST_CASE("XmlElement: Tree Structure and Parent Linking") {
    XmlElement parent("faculty");
    XmlElement* childTag = new XmlElement("major");
    XmlText* childText = new XmlText("Computer Science");

    SUBCASE("Adding children sets the parent pointer correctly") {
        parent.addChild(childTag);
        parent.addChild(childText);

        // Проверяваме дали децата "знаят" кой е баща им
        CHECK(childTag->getParent() == &parent);
        CHECK(childText->getParent() == &parent);
    }

    SUBCASE("getChildrenOfType filters children correctly using dynamic_cast") {
        parent.addChild(childTag);
        parent.addChild(childText);

        // Търсим само текстовите възли
        std::vector<XmlText*> texts = parent.getChildrenOfType<XmlText>();
        REQUIRE(texts.size() == 1);
        CHECK(texts[0]->getContent() == "Computer Science");

        // Търсим само таговете
        std::vector<XmlElement*> tags = parent.getChildrenOfType<XmlElement>();
        REQUIRE(tags.size() == 1);
        CHECK(tags[0]->getName() == "major");
    }
}

TEST_CASE("XmlElement: Exceptions") {
    XmlElement node("test");

    SUBCASE("Adding nullptr as child throws Exception") {
        CHECK_THROWS_AS(node.addChild(nullptr), const Exception&);
    }

    SUBCASE("Adding attribute with empty key throws Exception") {
        CHECK_THROWS_AS(node.addAttribute("", "value"), const Exception&);
    }
}

TEST_CASE("XmlElement: Deep Copy") {
    // 1. Построяваме оригинално дърво: <root id="1">Inner Text<child/></root>
    XmlElement original("root");
    original.addAttribute("id", "1");

    XmlText* textNode = new XmlText("Inner Text");
    XmlElement* tagNode = new XmlElement("child");

    original.addChild(textNode);
    original.addChild(tagNode);

    // 2. Клонираме го
    XmlNode* clonedRaw = original.clone();
    XmlElement* clone = dynamic_cast<XmlElement*>(clonedRaw);
    REQUIRE(clone != nullptr);

    // 3. Проверяваме данните (трябва да са същите)
    CHECK(clone->getName() == "root");
    CHECK(clone->getAttributeValue("id") == "1");

    std::vector<XmlText*> clonedTexts = clone->getChildrenOfType<XmlText>();
    REQUIRE(clonedTexts.size() == 1);
    CHECK(clonedTexts[0]->getContent() == "Inner Text");

    // 4. Паметта трябва да е различна!
    // Оригиналът и копието имат различни адреси
    CHECK(clone != &original);

    // Текстовият възел на копието трябва да е на различен адрес от този на оригинала
    CHECK(clonedTexts[0] != textNode);

    // 5. Почистваме клонинга. 
    // Ако каскадният деструктор е счупен, тук ще гръмне (утечка на памет или двойно освобождаване)
    delete clone;
}