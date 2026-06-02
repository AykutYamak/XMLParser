#include "doctest.h"
#include "../XmlCore/XmlNode.h"

class MockNode : public XmlNode {
public:
	void print(std::ostream& os, int indent = 0) const override {}
	XmlNode* clone() const override { return nullptr; }
};

TEST_CASE("XmlNode: Parent management and initialization.") {
	MockNode childNode;
	MockNode parentNode;

	SUBCASE("Default constructor initializes parent to nullptr") {
        // Когато създадем възел, той не трябва да има родител
        CHECK(childNode.getParent() == nullptr);
    }

    SUBCASE("setParent correctly updates the parent pointer") {
        // Задаваме parentNode като родител на childNode
        childNode.setParent(&parentNode);

        // Проверяваме дали указателят сочи към правилния адрес в паметта
        CHECK(childNode.getParent() == &parentNode);
    }

    SUBCASE("setParent can remove the parent by passing nullptr") {
        // Първо задаваме родител
        childNode.setParent(&parentNode);

        // После го "осиротяваме" (например ако го местим в дървото)
        childNode.setParent(nullptr);

        CHECK(childNode.getParent() == nullptr);
    }
}


