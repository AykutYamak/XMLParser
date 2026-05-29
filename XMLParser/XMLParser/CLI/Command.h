#pragma once
#include <string>
#include "../XmlCore/XmlDocument.h"

class Command {
public:
	virtual ~Command() {}
	virtual void execute(XmlDocument& doc, const std::string& args) = 0;
};
