#include <iostream>
#include "XmlDocument.h"
#include "XmlElement.h"
#include "XmlText.h"
#include "Exception.h"

int main() {
    std::cout << "=== FMI XML Parser Engine Test ===\n\n";

    try {
       
        std::cout << "[Test 1] Loading 'test.xml' into memory...\n";
        XmlDocument doc;
        doc.load("D:\\text.xml");

        std::cout << "Success! Here is the parsed tree:\n\n";
        if (doc.getRoot()) {
            doc.getRoot()->print(std::cout, 0);
        }

        std::cout << "\n[Test 2] Modifying the tree in RAM...\n";
        XmlElement* root = doc.getRoot();
        if (root) {

            XmlElement* newBook = new XmlElement("book");

            newBook->addAttribute("id", "99");
            newBook->addAttribute("author", "Isaac Asimov");

            newBook->addChild(new XmlText("Foundation"));

            root->addChild(newBook);
            std::cout << "Added 'Foundation' to the library.\n";
        }

        std::cout << "\n[Test 3] Saving modified tree to 'output.xml'...\n";
        doc.save("output.xml");
        std::cout << "Success! Check your folder for output.xml.\n";

    }

    catch (const Exception& e) {
        std::cerr << "\n[CRITICAL ERROR] XML Exception caught: \n" << e.what() << "\n";
    }

    catch (const std::exception& e) {
        std::cerr << "\n[CRITICAL ERROR] Standard Exception caught: \n" << e.what() << "\n";
    }

    try {
        std::cout << "\n[Test 4] Attempting to load a missing file...\n";
        XmlDocument docError;
        docError.load("does_not_exist.xml"); // This should fail!
    }
    catch (const FileException& e) {
        std::cout << "Defense successful! Caught the expected error:\n -> " << e.what() << "\n";
    }

    std::cout << "\n=== All tests completed. Exiting safely. ===\n";


	return 0;
}