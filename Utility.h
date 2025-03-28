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
std::vector<std::basic_string<T>> SplitByChar(std::basic_string<T>, T, bool = false);

std::string GetFileData(const wchar_t*);

#endif

