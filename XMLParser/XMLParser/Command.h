#pragma once
#include <string>
#include "XmlDocument.h"

class Command {
public:
	virtual ~Command() {}
	virtual void execute(XmlDocument& doc, const std::string& args) = 0;
};
