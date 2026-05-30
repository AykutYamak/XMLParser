#pragma once
#include <string>
#include <exception>

class Exception : public std::exception {
protected:
	std::string message;
public:
	Exception(const std::string& msg) : message("XML error: " + msg){}
	const char* what() const noexcept override {
		return message.c_str();
	}
}; 

class ParseException : public Exception {
public:
	ParseException(const std::string& msg) : Exception("Parsing error - " + msg) {}
};

class FileException : public Exception {
public:
	FileException(const std::string& msg) : Exception("File error - " + msg) {}
};