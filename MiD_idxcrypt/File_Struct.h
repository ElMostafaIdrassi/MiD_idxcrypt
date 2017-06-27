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

#ifndef FILE_STRUCT_H

#define FILE_STRUCT_H

#define STRONG_ITERATIONS	500000
#define READ_BUFFER_SIZE	65536

/* Headers of our crypto libraries */
#include "HashLib.h"		// Hash Lib
#include "HMACLib.h"		// Hmac Pseudo-random function
#include "PBKDF_Init.h"		// Hmac-PBKDF2 implementation
#include "AesApiFuncs.h"	// AES 

/* Openssl headers for */
#include "openssl/rand.h"	// pseudo-random data generation
#include "openssl/err.h"	// error strings

#include <time.h>
#include <string>

class File_Struct
{
public:
	File_Struct();
	static File_Struct* FileConstructor();
	virtual ~File_Struct() = 0;

	/*	
	 *	=============================================================================
	 *	 Transforms relative paths into absolute ones, then stores them in the object
	 *	=============================================================================
	 */
	virtual int relativeToAbsolutePath(const int & inOut) = 0;

	/* 
	 *	==================================================================================================================================
	 *	 Checks whether pIn and pOut are relative or absolute paths
	 *	 If pIn is relative for example, it fills the corresponding fields in the object (i.e. relativeInPath)
	 *	 then it proceeds to transforming the relative path to an absolute one, and stores it as well (i.e. absInPath)
	 *	==================================================================================================================================
	 */
	virtual int setPaths(const std::string & pIn, const std::string & pOut) = 0;

	/*	
	 *	========================================
	 *	Starts the encryption/decryption process
	 *	========================================
	 */
	virtual int Op(Hmac_PRF & prf, const char szPassword[], const size_t & cbSalt, const int & bForDecrypt) = 0;
};

#endif // !FILE_STRUCT_H
