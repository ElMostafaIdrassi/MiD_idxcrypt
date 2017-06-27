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

#include "mem_impl_dec.cpp"                     // my_memclr

#include <cstdio>				// printf
#include <cstring>				// memcpy, memcmp
#include <cstddef>				// size_t

#ifdef	__linux__
#include <sys/mman.h>
#else

#ifdef	_WIN32

#ifndef _WIN32_WINNT         
#define _WIN32_WINNT 0x0501 /* Windows XP minimum */
#endif

#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x05010300 /* XP SP3 minimum */
#endif

#include <windows.h>
#endif

#endif

#ifdef _MSC_VER

/* N.B : SDL checks need to be disabled as well */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#endif

void ShowUsage()
{
	printf("\nMiD_idxcrypt - Simple yet Strong file encryptor. By IDRASSI Mounir (mounir@idrix.fr)\n\nCopyright 2017 IDRIX\n\n\n");
	printf("To encrypt an entire folder : MiD_idxcrypt InputFolder Password OutputFolder [/d] [/hash algo]\n");
	printf("\tInputFolder example : C:\\inputFolder (absolute path) or inputFolder (relative path to the current working directory) \n");
	printf("\tOutputFolder example : C:\\outputFolder (absolute path) or outputFolder (relative path to the current working directory) \n\n");
	printf("To encrypt a file : MiD_idxcrypt InputFile Password OutputFile [/d] [/hash_algo]\n");
	printf("\tInputFile example : C:\\inputFile (absolute path) or inputFile (relative path to the current working directory) \n");
	printf("\tOutputFile example : C:\\outputFile (absolute path) or outputFile (relative path to the current working directory)\n\n");
	printf("\tParameters:\n");
	printf("\t  /d: Perform decryption instead of encryption (default)\n");
	printf("\t  /hash algo: Specifies hash algorithm to use for key derivation.\n");
	printf("\t              Possible values of algo are md5, sha1, sha256, sha384 and sha512.\n");
	printf("\t              sha256 is the default\n");
	printf("\n");
#ifdef _WIN32
	printf("\nPlease use backslashes rather than slashes!\n");
#endif
}

int main(int argc, char* argv[])
{
	/*

		On Windows	: 
			NTFS filesystem :	filenames are encoded in Unicode (UTF-16)
			FAT filesystem	:	filenames are encoded in OEM code page

		This program relies on user's keyboard input through the console. No GUI is used.

		On Windows	:

		*The console is by default configured to translate user's keyboard and output characters using the OEM
		Code Page. 
		*All internal C functions make use, by default, of the ANSI(Windows) Code Page when dealing with native 
		(non-wide) character and string literals. Both the source and execution charset are set by default to 
		the ANSI Code Page.
		*All non-native wide-targetted C functions make use of Windows Unicode charset (UTF-16-LE).

		It is clear that there will be unconsistencies when processing data coming from the console.
		Here are a few possible, but not (always) appliable solution : 

			-	Set the console input and output code page to UTF-16 and use wide functions everywhere : 
				=>	The console CP cannot be set to UTF-16 for "unmanaged" applications. This means this can only 
					be done when using "managed" C++ code (think of CLI)
				=>	Not appliable for this program (which uses CRT)

			-	Set the console input and output code page to UTF-8, use wide functions everywhere and do some kind of 
				conversion in between :
				=>	There is no intermediate conversion that can be implemented between the console receiving the char 
					from the user and storing the hex value of it in memory.
					This generated unexpected behaviour. Either the UTF-8 hex value will get truncated to fit in a UTF-16
					or the function (scanf for example) recognizes that the UTF-8 char is not a valid one and stores \0 or '?'
					instead.

			-	Set the console input and output code page to match the system ANSI code page, and use native chars and functions
				=>	This is actually a good idea. Even though the filenames and filepaths are supposed to be encoded using Unicode,
					a user's input, using a keyboard, may not contain chars that are outside of the code space of the ANSI code page
					of the machine. Therefore, the only extra step which we need to do is convert the ANSI chars to UTF-16 for them
					to be used by specific Wide Win32-API functions (the one that don't have their ANSI version), also when we need
					to exceed the MAX_PATH=260 (by prepending \\?\), which can only be done using Unicode Win32 API functions, also fopen). So, 
					we get the job done, then convert back to ANSI. There will be no data loss since we'll be able to represent every 
					character present in the specific Unicode data string, because we got this Unicode data string from the ANSI input.

					This last solution is the one implemented.

	*/

#ifdef _WIN32
	// Set console's input and output Code Page to the system's ANSI Code Page
	SetConsoleCP(GetACP());
	SetConsoleOutputCP(GetACP());
#endif

	/* Load the human readable error strings for libcrypto */
	ERR_load_crypto_strings();
	
	int iStatus = 0;

	Hmac_PRF prf{};
	prf.setHmacContext(sha256h);	// Hmac-PBKDF2-SHA256 is used by default
	size_t cbSalt = 16;				// 16 bytes salt for SHA-256 and 64-bytes otherwise
	char szPassword[129]{};         // Maximum 128 ANSI-encoded chars + trailing \0

	int bForDecrypt = 0;
        
	File_Struct * RootFile = nullptr;

	std::string finPath(argv[1]);
	
	std::string foutPath(argv[3]);
	
	
	// HashLib Initialization
	iStatus = HashLib_Init();
	if (0 == iStatus)
	{
		printf("\nHashLib initialization OK. Moving on...\n");

		// HmacLib Initialization
		iStatus = HMACLib_Init();
		if (0 == iStatus)
		{
			printf("\nHmacLib initialization OK. Moving on...\n");

			// PBKDF2 Initialization
			iStatus = PBKDF2_Init();
			if (0 == iStatus)
			{
				printf("\nPBKDF2 initialization OK. Moving on...\n");

				// AES Initialization
				iStatus = AesLib_Init();
				if (0 == iStatus)
				{
					printf("\nAesLib initialization OK. Moving on...\n\n");
				}

				else 
				{
					printf("\nAesLib initialization KO. Aborting...\n\n");
				}

			}

			else 
			{
				printf("\nPBKDF2 initialization KO. Aborting...\n");
			}
		}

		else 
		{
			printf("\nHmacLib initialization KO. Aborting...\n");
		}

	}

	else 
	{
		printf("\nHashLib initialization KO. Aborting...\n");
	}

	if (iStatus == 0) {

		// Interpretation of user's input

		if ((2 == argc && (0 == memcmp(argv[1], "-h", 2) || 0 == memcmp(argv[1], "--help", 6))) ||
			argc < 4 || argc > 7)
		{
			ShowUsage();
			iStatus = 1;
		}
		else if (argc >= 5)
		{
			for (int i = 4; i < argc; i++)
			{
				if (0 == memcmp(argv[i], "/hash", 5))
				{
					if ((i + 1) >= argc)
					{
						printf("Missing hash algorithm.\n");
						ShowUsage();
						iStatus = 1;
						break;
					}
					else  if (0 == memcmp(argv[i + 1], "md5", 3)) {
						prf.cleanData();
						prf.setHmacContext(md5h);
						cbSalt = 64;
					}
					else if (0 == memcmp(argv[i + 1], "sha1", 4)) {
						prf.cleanData();
						prf.setHmacContext(sha1h);
						cbSalt = 64;
					}
					else if (0 == memcmp(argv[i + 1], "sha256", 6)) {}
					else if (0 == memcmp(argv[i + 1], "sha384", 6))
					{
						prf.cleanData();
						prf.setHmacContext(sha384h);
						cbSalt = 64;
					}
					else if (0 == memcmp(argv[i + 1], "sha512", 6))
					{
						prf.cleanData();
						prf.setHmacContext(sha512h);
						cbSalt = 64;
					}
					else
					{
						printf("Unexpected hash algorithm.\n");
						ShowUsage();
						iStatus = 1;
						break;
					}
					i++;
				}
				else if (0 == memcmp(argv[i], "/d", 2))
				{
					bForDecrypt = 1;
				}
				else
				{
					printf("Unexpected Parameter.\n");
					ShowUsage();
					iStatus = 1;
					break;
				}
			}
		}
	}

	if (iStatus == 0)
	{

		// Check length of the password (\0 included)
		if (strlen(argv[2]) > 129) {
			printf("Password too long. Maximum password length is : 128 ANSI-encoded characters. Aborting...\n");
			iStatus = 1;
		}
		else {
			RootFile = File_Struct::FileConstructor();

			// Set root input and output filepaths
			iStatus = RootFile->setPaths(finPath, foutPath);

			if (0 != iStatus) {
				printf("\nAn error occured while setting input/output paths. Aborting...\n");
			}

			else {
				setvbuf(stdout, NULL, _IONBF, 0);
				setvbuf(stdin, NULL, _IONBF, 0);

#ifdef _WIN32
				// protect password memory against swaping //
				VirtualLock(szPassword, sizeof(szPassword));
#else
#ifdef __linux__
				mlock(szPassword, sizeof(szPassword));
#endif
#endif

				memcpy(szPassword, argv[2], strlen(argv[2]));	// copies the trailing \0, argv arguments are always null terminated

#ifdef _WIN32
				// clear the password in the command line //
				SecureZeroMemory(argv[2], strlen(argv[2]));
#else
#ifdef __linux__
				my_memclr(argv[2], strlen(argv[2]));
#endif
#endif

				// Start the encryption/Decryption operation
				iStatus = RootFile->Op(prf, szPassword, cbSalt, bForDecrypt);

#ifdef _WIN32
				// clear the password in szPassword //
				SecureZeroMemory(szPassword, sizeof(szPassword));
#else
#ifdef __linux__
				my_memclr(szPassword, sizeof(szPassword));
#endif
#endif
			}

			delete RootFile;
			RootFile = nullptr;
		}	
	}

	prf.cleanData();

	/* Remove error strings */
	ERR_free_strings();

	return iStatus;
}

