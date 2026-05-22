#pragma once
#include <string>
#include <vector>
#include "XmlElement.h"

struct IdEntry {
	std::string id;
	XmlElement* element;
};

class IdDictionary {
private:
	std::vector<IdEntry> entries;
public:
	bool contains(const std::string& id) const {
		for (const IdEntry& entry : entries )
		{
			if (entry.id == id)
			{
				return true;
			}
		}
		return false;
	}

	void add(const std::string& id, XmlElement* element) {
		entries.push_back({ id,element });
	}

	XmlElement* get(const std::string& id) const {
		for (const IdEntry& entry : entries)
		{
			if (entry.id == id)
			{
				return entry.element;
			}
		}
		return nullptr;
	}

	void clear() {
		entries.clear();
	}
};