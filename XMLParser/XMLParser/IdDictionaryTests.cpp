#include "Tests/doctest.h"
#include "XmlCore/IdDictionary.h"
#include "XmlCore/XmlElement.h"

TEST_CASE("IdDictionary: Basic Operations and Integrity") {
    IdDictionary registry;

    // Създаваме тестови елементи на стека. 
    // Речникът само ще ги наблюдава чрез адресите им (&el1, &el2).
    XmlElement el1("user");
    XmlElement el2("profile");

    SUBCASE("Empty dictionary returns false and nullptr") {
        // Проверяваме базовото състояние на празен речник
        CHECK(registry.contains("gen_0") == false);
        CHECK(registry.get("gen_0") == nullptr);
    }

    SUBCASE("Add correctly registers and retrieves elements by ID") {
        registry.add("u1", &el1);
        registry.add("p1", &el2);

        // 1. Проверка на метода contains
        CHECK(registry.contains("u1") == true);
        CHECK(registry.contains("p1") == true);
        CHECK(registry.contains("missing_id") == false);

        // 2. Проверка на метода get (сравняваме адреси)
        CHECK(registry.get("u1") == &el1);
        CHECK(registry.get("p1") == &el2);
    }

    SUBCASE("Clear flushes the dictionary references but preserves objects") {
        registry.add("u1", &el1);

        // Изчистваме речника
        registry.clear();

        // 1. Речникът вече трябва да е празен
        CHECK(registry.contains("u1") == false);
        CHECK(registry.get("u1") == nullptr);

        // 2. Тъй като clear() трие само записите във вектора, но НЕ и обектите,
        // елементът el1 трябва да е напълно жив и достъпен тук.
        CHECK(el1.getName() == "user");
    }
}