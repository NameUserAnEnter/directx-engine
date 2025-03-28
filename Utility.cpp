#include "Utility.h"

void Popup(std::wstring message, std::wstring caption) {
	MessageBoxW(NULL, message.c_str(), caption.c_str(), MB_OK);
}

void Popup(std::string message, std::string caption) {
	MessageBoxA(NULL, message.c_str(), caption.c_str(), MB_OK);
}

void Popup(float num) {
	Popup(NumStr(num));
}

void Popup() {
	Popup("");
}

std::wstring toWide(std::string str) {
	std::wstring wide = L"";
	for (auto c : str) {
		wide += (wchar_t) c;
	}
	return wide;
}

std::string toNarrow(std::wstring str) {
	std::string narrow = "";
	for (auto c : str) {
		narrow += (unsigned char) c;
	}
	return narrow;
}

bool StrContains(std::string str, char c) {
	for (char current : str) {
		if (current == c) return true;
	}
	return false;
}

std::string StrStripR(std::string str, char c) {
	while (str.size() > 0) {
		if (str.back() == c) str.pop_back();
		else break;
	}
	return str;
}

std::wstring NumStr(unsigned long long num, unsigned int base) {
	if (base == 0) return nullptr;

	int digits = 0;
	unsigned long long remaining = num;

	do {
		digits++;

		remaining /= base;
	} while (remaining > 0);

	std::wstring str = L"";

	for (int i = 0; i < digits; i++) {
		int digit = (int) ((num / (unsigned long long) pow(base, i)) % base);

		wchar_t starting_point = L'0';
		if (digit >= 10) starting_point = L'A' - (wchar_t) 10;

		str = (wchar_t) (starting_point + (wchar_t) digit) + str;
	}

	return str;
}

std::wstring HexStr(unsigned long long num) {
	return L"0x" + NumStr(num, 16);
}

template<typename T>
std::vector<std::basic_string<T>> SplitByChar(std::basic_string<T> str, T delimiter, bool leaveDelimiters) {
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

std::string GetFileData(const wchar_t* filename) {
	std::string data = "";

	FILE* file;
	_wfopen_s(&file, filename, L"rb");

	if (file == nullptr) {
		Popup(L"Cannot open file \"" + std::wstring(filename) + L"\".");
		return data;
	}

	int currentChar;
	do {
		currentChar = getc(file);

		if (currentChar != EOF) data += (unsigned char) currentChar;
	} while (currentChar != EOF);
	fclose(file);

	return data;
}

