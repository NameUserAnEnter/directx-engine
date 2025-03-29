#ifndef UTILITY_H
#define UTILITY_H


#include <string>
#include <vector>
#include <windows.h>


void Popup(std::wstring, std::wstring = L"");
void Popup(std::string, std::string = "");
void Popup(float);
void Popup();

std::wstring toWide(std::string);
std::string toNarrow(std::wstring);

bool StrContains(std::string, char);
std::string StrStripR(std::string, char);

std::wstring NumStr(unsigned long long, unsigned int = 10);
std::wstring HexStr(unsigned long long);

template<typename T>
inline std::vector<std::basic_string<T>> SplitByChar(std::basic_string<T> str, T delimiter, bool leaveDelimiters = false) {
	std::vector<std::basic_string<T>> parts;
	parts.push_back(std::basic_string<T>());

	for (int i = 0; i < str.size(); i++) {
		parts.back() += str[i];

		if (str[i] == delimiter) {
			if (!leaveDelimiters) parts.back().pop_back();

			parts.push_back(std::basic_string<T>());
		}
	}

	if (!parts.empty()) {
		if (parts.back().size() == 0) parts.pop_back();
	}
	return parts;
}


#endif

