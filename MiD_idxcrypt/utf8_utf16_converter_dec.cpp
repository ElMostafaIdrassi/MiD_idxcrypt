/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifdef _WIN32

#ifndef UTF8_UTF16_CONV
#define UTF8_UTF16_CONV

#include <vector>
#include <iostream>
#include <Windows.h>

#define MAX_UTF_LEN 2147483647 // int(size_t) max value

std::vector<wchar_t> Utf8_To_Utf16(const std::vector<char> & utf8);

std::vector<char> Utf16_To_Utf8(const std::vector<wchar_t> & utf16);

#endif // UTF8_UTF16_CONV

#endif // _WIN32