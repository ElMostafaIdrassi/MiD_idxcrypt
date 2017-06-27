/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifndef ANSI_UTF16
#define ANSI_UTF16

#ifdef _WIN32

#include <string>
#include <cstdio>
#include <Windows.h>

#define MAX_UTF_LEN 2147483647 // signed int max value

// User's input via cmd is stored as system's default ANSI (CP) when using chars
// For it to be used with wide WIN32 APIs, we convert it to UTF16
std::wstring ANSI_To_UTF16(const std::string & ansi);

// To print wide chars (UTF-16) correctly in cmd, the chars need to be encoded in system's ANSI default code page
std::string UTF16_To_ANSI(const std::wstring & utf16);

#endif // !_WIN32

#endif // !ANSI_UTF16