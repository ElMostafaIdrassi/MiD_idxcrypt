/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifdef _WIN32

#include "File_Struct.h"

class Win32_File 
	: public File_Struct
{
private:

	std::string relativeInpath{};			// relative input path 
	std::string relativeOutpath{};			// relative output path 

	std::wstring relativeWideInpath{};		// relative input path	(used with Wide Win32-API functions)
	std::wstring relativeWideOutpath{};		// relative output path	(used with Wide Win32-API functions)

	std::string absInpath{};				// absolute input path 
	std::string absOutpath{};				// absolute output path

	std::wstring absWideInpath{};			// absolute input path	(used with Wide Win32-API functions)
	std::wstring absWideOutpath{};			// absolute output path	(used with Wide Win32-API functions)

public:

	Win32_File();

	/*	Copy, Move constructor and assignment operators declared to fulfill "Rule of Five"
		Declared as delete because they are never used 
	*/

	// Copy constructor
	Win32_File(const Win32_File & other) = delete;
	// Copy assignment
	Win32_File & operator=(const Win32_File & other) = delete;
	// Move constructor
	Win32_File(Win32_File && other) = delete;
	// Move assignment
	Win32_File & operator=(Win32_File && other) = delete;

	~Win32_File();

	int relativeToAbsolutePath(const int & inOut) override;

	int setPaths(const std::string & pIn, const std::string & pOut) override;

	int Op(Hmac_PRF & prf, const char szPassword[], const size_t & cbSalt, const int & bForDecrypt) override;
};

#endif // _WIN32