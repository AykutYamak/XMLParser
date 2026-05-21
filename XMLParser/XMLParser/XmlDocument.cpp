#include "XmlDocument.h"
#include "Exception.h"
#include <fstream>
#include <iostream>

XmlDocument::~XmlDocument() {
	delete root;
	root = nullptr;
}

void XmlDocument::save(const std::string& filename) const{
	if (!root) return;

	std::ofstream file(filename);

	if (!file.is_open())
	{
		throw FileException("Could not open file for writing: " + filename);
	}

	root->print(file, 0);
	file.close();
}
