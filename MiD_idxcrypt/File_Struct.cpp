/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*	Copyright OpenSSL 2017
*	Contents licensed under the terms of the OpenSSL license
*	See http://www.openssl.org/source/license.html for details
*/

#include "File_Struct.h"

#include "Win32_File_dec.cpp"
#include "Linux_File_dec.cpp"

#include <cstdio>

File_Struct::File_Struct()
{
}

File_Struct * File_Struct::FileConstructor()
{
#ifdef  __linux__
	return (new Linux_File());
#else
#ifdef _WIN32
	return (new Win32_File());
#endif
#endif
	printf ("Unrecognized OS.\nCompatible OSes are Linux and Windows. Aborting...\n");
	return (nullptr);
}


File_Struct::~File_Struct()
{
}

