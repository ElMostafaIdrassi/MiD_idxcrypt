/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifdef _WIN32

#include "ANSI_UTF16_converter_dec.cpp"

std::wstring ANSI_To_UTF16(const std::string & ansi)
{
	std::wstring utf16 = {};		// Result

	// If input is empty, return an empty output 

	if (ansi.empty())
	{
		return utf16;
	}

	// Since we provide ansi.length() as 4th argument to MultiByteToWideChar
	// which expects it to be an int, and because ansi.length() is a size_t <=> 
	// (unsigned int) in 32-bit env and <=> (signed long long) in 64-bit env,
	// we avoid loss of data by checking that ansi string length does not 
	// exceed max value of a signed int, which is the same in 32-bit and 64-bit env
	// (the size_t must fit in the int)

	if (ansi.size() > MAX_UTF_LEN)
	{
		printf("ANSI length too long (> MaX_INT).\n");
		return utf16; // empty output
	}

	// Get length (in wchars) of resulting UTF-16 string
	// Check whether there are invalid chars in ANSI string
	// No actual conversion takes place here

	const int utf16Length = MultiByteToWideChar(
		GetACP(),					// Input is ANSI (this flag represents the type of the Multibyte string)
		MB_ERR_INVALID_CHARS,	// Conversion flags (make function fail if an invalid input character is encountered)
		ansi.data(),			// Input ANSI data
		(int)ansi.size(),		// Length of the ANSI data, in bytes = 8-bit characters 
		nullptr,				// No actual conversion
		0						// 0 to get the actual size of result, in 32-bit characters = wchars
	);

	// In case function fails, length of output is 0

	if (utf16Length == 0)
	{
		printf("Invalid ANSI chars found in ANSI input.\nMultiByteToWideChar failed - 1st call.\n");
		printf("Error code : %d.\n", GetLastError());
		return utf16; // empty output
	}

	// Else, resize the output with appropriate length

	utf16.resize(utf16Length);

	// Actual conversion 

	if (0 == MultiByteToWideChar(
		GetACP(),				// Code page to use for conversion - Input is ANSI
		0,                  // No need for flags here, we checked the input in previous call				
		ansi.data(),        // Input ANSI data
		(int)ansi.size(),	// Length of the ANSI data, in bytes = 8-bit characters 
		&utf16.front(),     // Actual conversion => output
		(int)utf16.size()	// We already know the actual size of the output from last call (in wchars)
	))
	{
		printf("Conversion from ANSI to UTF-16 failed.\nMultiByteToWideChar failed - 2nd call.\n");
		printf("Error code : %d.\n", GetLastError());
		return std::wstring(); // empty output
	}

	// Return output
	return utf16;
}

std::string UTF16_To_ANSI(const std::wstring & utf16)
{
	std::string ansi = {}; // Result

								 // If input is empty, return an empty output 

	if (utf16.empty())
	{
		return ansi;
	}

	// Since we provide ut16.length() as 4th argument to MultiByteToWideChar
	// which expects it to be an int, and because ut16.length() is a size_t <=> 
	// (unsigned int) in 32-bit env and <=> (signed long long) in 64-bit env,
	// we avoid loss of data by checking that utf16 string length does not 
	// exceed max value of an int, which is the same in 32-bit and 64-bit env
	// (the size_t must fit in the int)

	if (utf16.size() > MAX_UTF_LEN)
	{
		printf("UTF-16 length too long (> MAX_INT).\n");
		return ansi; // empty output
	}

	// Get length (in 8-bit chars) of resulting ANSI string
	// Check whether there are invalid chars in UTF-16 string
	// No actual conversion takes place here

	const int ansiLength = WideCharToMultiByte(		// Maps a UTF - 16 (wide character) string to a new character string
		GetACP(),				// Code page to use for conversion = Output must be ANSI
		0,						// Conversion flags 0, since input is UTF-16
		utf16.data(),			// Input UTF-16 data
		(int)utf16.size(),		// Length of the UTF-16 data, in wchars = 32-bit chars 
		NULL,					// No actual conversion
		0,						// 0 to get the actual size of result, in 8-bit characters = chars
		NULL,					// Pointer to the character to use if a character cannot be represented in the specified code page (NULL if the function is to use a system default value)
		NULL					// Pointer to a flag that indicates if the function has used a default character in the conversion (NULL)
	);

	// In case function fails, length of output is 0

	if (ansiLength == 0)
	{
		printf("Invalid UTF-16 chars found in UTF-16 input.\nWideCharToMultiByte failed - 1st call.\n");
		printf("Error code : %d.\n", GetLastError());
		return ansi; // empty output
	}

	// Else, resize the output with appropriate length

	ansi.resize(ansiLength);

	// Actual conversion 

	if (0 == WideCharToMultiByte(
		GetACP(),				// Code page to use for conversion = Output must be ANSI
		0,						// No need for flags here, we checked the input in previous call
		utf16.data(),			// Input UTF-16 data
		(int)utf16.size(),		// Length of the UTF-16 data, in wchars = 32-bit chars 
		&ansi[0],			// Actual conversion => output
		(int)ansi.size(),		// We already know the actual size of the output from last call (in 8-bit chars)
		NULL, NULL
	))
	{
		printf("Conversion from UTF-16 to ANSI failed.\nWideCharToMultiByte failed - 2nd call.\n");
		printf("Error code : %d.\n", GetLastError());
		return std::string(); // empty output
	}

	// Return output
	return ansi;
}

#endif