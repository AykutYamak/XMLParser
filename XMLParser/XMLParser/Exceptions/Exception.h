#pragma once
#include <string>
#include <exception>
/**
 * @brief Базов клас за изключенията в XML парсъра.
 * * Наследява std::exception и служи като основа за всички специфични
 * изключения, хвърляни от приложението. Автоматично добавя префикса
 * "XML error: " към съобщенията за грешка.
 */
class Exception : public std::exception {
protected:
	std::string message; ///< Форматираното съобщение за грешка.
public:
	/**
	 * @brief Конструктор за базовото XML изключение.
	 * @param msg Конкретният текст на грешката.
	 */
	Exception(const std::string& msg) : message("XML error: " + msg){}

	/**
	 * @brief Връща обяснителното съобщение за грешката.
	 * @return Указател към C-style низ (null-terminated), съдържащ съобщението.
	 */
	const char* what() const noexcept override {
		return message.c_str();
	}
}; 

/**
 * @brief Изключение, хвърляно при грешка по време на парсване на XML.
 * * Използва се при откриване на синтактични грешки, липсващи затварящи тагове,
 * неочакван текст или структурни несъответствия в заредения XML документ.
 */
class ParseException : public Exception {
public:
	/**
	 * @brief Конструктор за ParseException.
	 * @param msg Детайли за грешката при парсване.
	 */
	ParseException(const std::string& msg) : Exception("Parsing error - " + msg) {}
};

/**
 * @brief Изключение, свързано с файлови операции.
 * * Използва се, когато даден файл не може да бъде отворен, създаден, прочетен
 * или при опит за запазване на данни без предварително зареден файл.
 */
class FileException : public Exception {
public:
	/**
     * @brief Конструктор за FileException.
     * @param msg Детайли за файловата грешка.
     */
	FileException(const std::string& msg) : Exception("File error - " + msg) {}
};