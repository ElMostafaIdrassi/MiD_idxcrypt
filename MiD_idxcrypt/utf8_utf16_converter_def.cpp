/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifdef _WIN32

#include "utf8_utf16_converter_dec.cpp"

std::vector<wchar_t> Utf8_To_Utf16(const std::vector<char> & utf8)
{
	std::vector<wchar_t> utf16 = {}; // Result

	// If input is empty, return an empty output 

	if (utf8.empty())
	{
		return utf16;
	}

	// Since we provide ut8.length() as 4th argument to MultiByteToWideChar
	// which expects it to be an int, and because ut8.length() is a size_t <=> 
	// (unsigned int) in 32-bit env and <=> (signed long long) in 64-bit env,
	// we avoid loss of data by checking that ut8 string length does not 
	// exceed max value of an int, which is the same in 32-bit and 64-bit env
	// (the size_t must fit in the int)

	if (utf8.size() > MAX_UTF_LEN)
	{
		std::cerr << "UTF-8 length too long (> MaX_INT).\n";
		return utf16; // empty output
	}

	// Get length (in wchars) of resulting UTF-16 string
	// Check whether there are invalid chars in UTF-8 string
	// No actual conversion takes place here

	const int utf16Length = MultiByteToWideChar(
		CP_UTF8,				// Input is UTF-8
		MB_ERR_INVALID_CHARS,	// Conversion flags (make function fail if an invalid input character is encountered)
		utf8.data(),			// Input UTF-8 data
		(int)utf8.size(),		// Length of the UTF-8 data, in bytes = 8-bit characters 
		nullptr,				// No actual conversion
		0						// 0 to get the actual size of result, in 32-bit characters = wchars
	);

	// In case function fails, length of output is 0

	if (utf16Length == 0)
	{
		std::cerr << "Invalid UTF-8 chars found in UTF-8 input.\nMultiByteToWideChar failed - 1st call.\n";
		std::cerr << "Error code : " << GetLastError() << " .\n";
		return utf16; // empty output
	}

	// Else, resize the output with appropriate length

	utf16.resize(utf16Length);

	// Actual conversion 

	if (0 == MultiByteToWideChar(
		CP_UTF8,            // Code page to use for conversion - Input is UTF-8
		0,                  // No need for flags here, we checked the input in previous call				
		utf8.data(),        // Input UTF-8 data
		(int)utf8.size(),	// Length of the UTF-8 data, in bytes = 8-bit characters 
		utf16.data(),       // Actual conversion => output
		(int)utf16.size()	// We already know the actual size of the output from last call (in wchars)
	))
	{
		std::cerr << "Conversion from UTF-8 to UTF-16 failed.\nMultiByteToWideChar failed - 2nd call.\n";
		std::cerr << "Error code : " << GetLastError() << " .\n";
		return std::vector<wchar_t>(); // empty output
	}

	// Return output
	return utf16;
}

std::vector<char> Utf16_To_Utf8(const std::vector<wchar_t> & utf16)
{
	std::vector<char> utf8 = {}; // Result

								 // If input is empty, return an empty output 

	if (utf16.empty())
	{
		return utf8;
	}

	// Since we provide ut16.length() as 4th argument to MultiByteToWideChar
	// which expects it to be an int, and because ut16.length() is a size_t <=> 
	// (unsigned int) in 32-bit env and <=> (signed long long) in 64-bit env,
	// we avoid loss of data by checking that ut8 string length does not 
	// exceed max value of an int, which is the same in 32-bit and 64-bit env
	// (the size_t must fit in the int)

	if (utf16.size() > MAX_UTF_LEN)
	{
		std::cerr << "UTF-16 length too long (> MAX_INT).\n";
		return utf8; // empty output
	}

	// Get length (in 8-bit chars) of resulting UTF-8 string
	// Check whether there are invalid chars in UTF-16 string
	// No actual conversion takes place here

	const int utf8Length = WideCharToMultiByte(
		CP_UTF8,				// Code page to use for conversion = Output must be UTF-8
		WC_ERR_INVALID_CHARS,	// Conversion flags (make function fail if an invalid input character is encountered)
		utf16.data(),			// Input UTF-16 data
		(int)utf16.size(),		// Length of the UTF-16 data, in wchars = 32-bit chars 
		NULL,					// No actual conversion
		0,						// 0 to get the actual size of result, in 8-bit characters = chars
		NULL, NULL				// Both set to NULL for UTF-8, otherwise error
	);

	// In case function fails, length of output is 0

	if (utf8Length == 0)
	{
		std::cerr << "Invalid UTF-16 chars found in UTF-16 input.\nMultiByteToWideChar failed - 1st call.\n";
		return utf8; // empty output
	}

	// Else, resize the output with appropriate length

	utf8.resize(utf8Length);

	// Actual conversion 

	if (0 == WideCharToMultiByte(
		CP_UTF8,				// Code page to use for conversion = Output must be UTF-8
		0,						// No need for flags here, we checked the input in previous call
		utf16.data(),			// Input UTF-16 data
		(int)utf16.size(),		// Length of the UTF-16 data, in wchars = 32-bit chars 
		utf8.data(),			// Actual conversion => output
		(int)utf8.size(),		// We already know the actual size of the output from last call (in 8-bit chars)
		NULL, NULL				// Both set to NULL for UTF-8, otherwise error
	))
	{
		std::cerr << "Conversion from UTF-16 to UTF-8 failed.\nMultiByteToWideChar failed - 2nd call.\n";
		return std::vector<char>(); // empty output
	}

	// Return output
	return utf8;
}

#endif // _WIN32