/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifdef _WIN32

#include "Win32_File.h"

#include "mem_impl.h"                     // my_memclr

#include "ANSI_UTF16_converter.h"

#include <Shlwapi.h>							// PathIsRelative and company
#include <io.h>									// _filelengthi64

/* Function to display information about the progress of the current operation */
clock_t startClock = 0;
clock_t currentClock = 0;

void ShowProgress(LPCTSTR szOperationDesc, __int64 inputLength, __int64 totalProcessed, BOOL bFinalBlock)
{
	/* display progress information every 2 seconds */
	clock_t t = clock();
	if ((currentClock == 0) || bFinalBlock || ((t - currentClock) >= (2 * CLOCKS_PER_SEC)))
	{
		currentClock = t;
		if (currentClock == startClock) currentClock = startClock + 1;
		double processingTime = (double)(currentClock - startClock) / (double)CLOCKS_PER_SEC;
		if (bFinalBlock)
			printf("\r%sDone! (time: %.2fs - speed: %.2f MiB/s)\n", szOperationDesc, processingTime, (double)totalProcessed / (processingTime * 1024.0 * 1024.0));
		else
			printf("\r%s (%.2f%% - %.2f MiB/s)", szOperationDesc, ((double)totalProcessed * 100.0) / (double)inputLength, (double)totalProcessed / (processingTime * 1024.0 * 1024.0));
	}
}

Win32_File::Win32_File()
{
}

Win32_File::~Win32_File()
{
}

int Win32_File::relativeToAbsolutePath(const int & inOut)
{
	std::wstring absWidePath = {};

	DWORD len = GetFullPathNameW(inOut == 0 ? relativeWideInpath.data() : relativeWideOutpath.data(), 0, NULL, NULL);
	// len contains the size, in WCHARs, of the buffer that is required to hold the path and the terminating null character.

	if (len == 0) {
		printf("\nAn error occured while transforming the relative path to an absolute one! 1st call - Aborting...(%d)\n", GetLastError());
		return -1;
	}

	absWidePath.resize(len);

	DWORD temp = GetFullPathNameW(inOut == 0 ? relativeWideInpath.data() : relativeWideOutpath.data(), len, &absWidePath[0], NULL);
	// temp contains, if succeeds, the size, in WCHARs, of the buffer that is required to hold the path without the terminating null character.
	// Since the path is relative, it is limited to MAX_PATH = 260 chars. No need to prepend "\\?\"

	if (temp == 0) {
		printf("An error occured while transforming the relative path to an absolute one! 2nd call - Aborting... (%d)\n", GetLastError());
		return -1;
	}
	absWidePath.resize(temp);		// W functions always add extra space (usually full of \0); We resize
	absWidePath.shrink_to_fit();	// To set capacity to be equal to size

	std::string absPath(UTF16_To_ANSI(absWidePath));

	// Prepend the "\\?\" for absolute paths to get rid of MAX_PATH limitation
	if (inOut == 0) {
		absWideInpath = L"\\\\?\\" + absWidePath;
		absInpath = absPath;	// we dont prepend \\?\ because it is ANSI

	}
	if (inOut == 1) {
		absWideOutpath = L"\\\\?\\" + absWidePath;
		absOutpath = absPath;	// we dont prepend \\?\ because it is ANSI
	}

	return 0;
}

int Win32_File::setPaths(const std::string & pIn, const std::string & pOut)
{
	int iStatus = 0;

	// Convert from ANSI to UTF-16 to use Wide Win32 APIs
	std::wstring wideInP(ANSI_To_UTF16(pIn));
	std::wstring wideOutP(ANSI_To_UTF16(pOut));

	// If empty, there was an error during the conversion ANSI -> UTF-16
	if (wideInP.empty() || wideOutP.empty()) return (iStatus = -1);

	if (TRUE == PathIsRelativeW(wideInP.data())) {	// input path is relative, get absolute path and store both

		relativeInpath = pIn;
		relativeWideInpath = wideInP;

		iStatus = relativeToAbsolutePath(0);
	}
	else {											// if input is absolute, prepend "\\?\" for Wide version and store
													// GetFullPathNameW doesn't check whether input path is a valid path => \\?\ prefix with ".." and "." possible
		std::wstring absWidePath = {};
		absWideInpath = L"\\\\?\\" + wideInP;

		DWORD len = GetFullPathNameW(absWideInpath.data(), 0, NULL, NULL);
		// len contains the size, in WCHARs, of the buffer that is required to hold the path and the terminating null character.

		if (len == 0) {
			printf("\nAn error occured while transforming the relative path to an absolute one! 1st call - Aborting...(%d)\n", GetLastError());
			return -1;
		}
		absWidePath.resize(len);
		DWORD temp = GetFullPathNameW(absWideInpath.data(), len, &absWidePath[0], NULL);
		// temp contains, if succeeds, the size, in WCHARs, of the buffer that is required to hold the path without the terminating null character.
		// Since the path is relative, it is limited to MAX_PATH = 260 chars. No need to prepend "\\?\"

		if (temp == 0) {
			printf("An error occured while transforming the relative path to an absolute one! 2nd call - Aborting... (%d)\n", GetLastError());
			return -1;
		}
		absWidePath.resize(temp);		// W functions always add extra space (usually full of \0); We resize
		absWidePath.shrink_to_fit();	// To set capacity to be equal to size

		absWideInpath = absWidePath;
		absWideInpath.shrink_to_fit();
		absInpath = UTF16_To_ANSI(absWideInpath);
	}
	if (TRUE == PathIsRelativeW(wideOutP.data())) {	// output path is relative, get absolute path and store both

		relativeOutpath = pOut;
		relativeWideOutpath = wideOutP;

		iStatus = relativeToAbsolutePath(1);
	}
	else {											// if input is absolute, prepend "\\?\" for Wide version and store
													// GetFullPathNameW doesn't check whether input path is a valid path => \\?\ prefix with ".." and "." possible
		std::wstring absWidePath = {};
		absWideOutpath = L"\\\\?\\" + wideOutP;

		DWORD len = GetFullPathNameW(absWideOutpath.data(), 0, NULL, NULL);
		// len contains the size, in WCHARs, of the buffer that is required to hold the path and the terminating null character.

		if (len == 0) {
			printf("\nAn error occured while transforming the relative path to an absolute one! 1st call - Aborting...(%d)\n", GetLastError());
			return -1;
		}
		absWidePath.resize(len);
		DWORD temp = GetFullPathNameW(absWideOutpath.data(), len, &absWidePath[0], NULL);
		// temp contains, if succeeds, the size, in WCHARs, of the buffer that is required to hold the path without the terminating null character.
		// Since the path is relative, it is limited to MAX_PATH = 260 chars. No need to prepend "\\?\"

		if (temp == 0) {
			printf("An error occured while transforming the relative path to an absolute one! 2nd call - Aborting... (%d)\n", GetLastError());
			return -1;
		}
		absWidePath.resize(temp);		// W functions always add extra space (usually full of \0); We resize
		absWidePath.shrink_to_fit();	// To set capacity to be equal to size

		absWideOutpath = absWidePath;
		absWideOutpath.shrink_to_fit();
		absOutpath = UTF16_To_ANSI(absWideOutpath);

	}

	return iStatus;
}

static int opFile(FILE* fin, FILE* fout, const std::wstring & outPath, Hmac_PRF & prf, const char szPassword[], const size_t & cbSalt, const int & bForDecrypt)
{
	BYTE pbDerivedKey[32] = {};
	BYTE pbSalt[64] = {}, pbIV[16] = {};
	BYTE pbData[READ_BUFFER_SIZE + 32] = {};
	size_t cbData = 0;
	size_t readLen = 0;
	__int64 totalProcessed = 0;
	__int64 inputLength = 0;
	int iStatus = 0;

	AES_CTX ctx{};

	inputLength = _filelengthi64(_fileno(fin));

	if (bForDecrypt) {

		// Read the salt + IV from the encrypted input file
		// Check if we read the entire cbSalt bytes for salt and 16 bytes for IV
		if ((cbSalt != fread(pbSalt, 1, cbSalt, fin)) || (16 != fread(pbIV, 1, 16, fin)))
		{
			printf("An unexpected error occured while reading the input file. Aborting...\n");
			iStatus = 1;
		}
		else
		{
			/* remove size of salt and IV from the input length */
			inputLength -= (__int64)(16 + cbSalt);

			printf("Generating the decryption key...");

			// Generate the decryption key using Hmac-PBKDF using the salt retrieved from the file + user password
			if (0 != PBKDF2(prf, STRONG_ITERATIONS, (unsigned char*)szPassword, (unsigned int)strlen(szPassword), pbSalt, (unsigned int)cbSalt, pbDerivedKey, 32))
			{
				printf("Error!\nAn unexpected error occured while creating the decryption key. Aborting...\n");
				iStatus = 1;
			}

			else
			{
				printf("Done!\nInitializing decryption...");

				// Initialization of the AES context
				if (0 != CreateCipher(ctx, CBC, pbDerivedKey, 256, pbIV, 0)) {
					printf("An error occured during the creationg of the decryption context. Aborting...\n");
					iStatus = 1;
				}

				else
				{
					TCHAR szOpDesc[64]{};
					BYTE pbHeader[16]{};
					memcpy(pbHeader, "IDXCRYPTTPYRCXDI", 16);

					printf("Done!\n");

					memcpy(szOpDesc, "Decrypting the input file...\0", 29);

					printf(szOpDesc);

					// Read the next 16 bytes in the encrypted file, which are supposed to represent the encrypted header
					cbData = fread(pbData, 1, 16, fin);
					/* remove size of header from input length */
					inputLength -= 16;

					if (cbData != 16) {
						printf("An unexpected error occured while reading data from input file. Aborting\n");
						iStatus = 1;
					}

					else
					{
						// AES decryption of the header using IV,DerivedKey
						if (0 != OpCipher(ctx, pbData, 16, pbData, 16, cbData, 0))
						{
							printf("Unexpected error occured while decrypting. Aborting\n");
							iStatus = 1;
						}

						else
						{
							// If the decrypted header is different than pbHeader, maybe the password is incorrect
							if ((cbData != 16) || (0 != memcmp(pbData, pbHeader, 16)))
							{
								printf("Password incorrect or the input file is not a valid encrypted file. Aborting!\n");
								iStatus = 1;
							}
							else
							{
								BOOL bFinal = FALSE;
								startClock = clock();

								// We read 65536 bytes of the decrypted file at a time, which we decrypt
								// Until we reach the last 65536 block which we decrypt here, or we reach final block that is < 65536
								// We redo the 2nd initialization for every block of 65536 by giving it the currentIV
								while (((cbData = fread(pbData, 1, READ_BUFFER_SIZE, fin)) == READ_BUFFER_SIZE) && FALSE == bFinal)
								{
									totalProcessed += (__int64)READ_BUFFER_SIZE;
									bFinal = (totalProcessed == inputLength) ? TRUE : FALSE;
									if (0 == OpCipher(ctx, pbData, cbData, pbData, cbData, cbData, 0))
									{
										if (cbData == fwrite(pbData, 1, cbData, fout))
										{
											ShowProgress(szOpDesc, inputLength, totalProcessed, bFinal);
										}
										else
										{
											printf("Not all decrypted bytes were written to disk. Aborting!\n");
											iStatus = 1;
											break;
										}
									}
									else
									{
										printf("\nUnexpected error occured while decrypting data. Aborting!\n");
										iStatus = 1;
										break;
									}
								}

								if (iStatus == 0) {
									// if there is still a trailing block < 65536
									if (FALSE == bFinal)
									{
										if (ferror(fin))
										{
											printf("Unexpected error occured while reading data from input file. Aborting!\n");
											iStatus = 1;
										}
										else
										{
											//cbData = (DWORD)readLen;	// cbData already contains last block size
											totalProcessed += (__int64)cbData;
											if (0 == OpCipher(ctx, pbData, cbData, pbData, cbData + 16, cbData, 1))
											{
												if (cbData == fwrite(pbData, 1, cbData, fout))
												{
													ShowProgress(szOpDesc, inputLength, totalProcessed, TRUE);
												}
												else
												{
													printf("Not all decrypted bytes were written to disk. Aborting!\n");
													iStatus = 1;
												}
											}
											else {
												printf("Unexpected error occured while decrypting. Aborting!\n");
												iStatus = 1;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		/* Entropy collection : seed the generator using the system entropy source */
		RAND_poll();
		/* generate random salt and IV */
		if (0 == RAND_bytes(pbSalt, (int)cbSalt) || 0 == RAND_bytes(pbIV, 16))
		{
			DWORD dwErr = ERR_get_error();
			printf("An unexpected error occured while preparing for the encryption (Code 0x%.8X). Aborting...\n", dwErr);
			iStatus = 1;
		}

		else
		{
			printf("Generating the encryption key...");

			// Generate the encryption key using Hmac-PBKDF using the salt generated randomly + user password
			if (0 != PBKDF2(prf, STRONG_ITERATIONS, (unsigned char*)szPassword, (unsigned int)strlen(szPassword), pbSalt, (unsigned int)cbSalt, pbDerivedKey, 32))
			{
				printf("Error!\nAn unexpected error occured while creating the encryption key. Aborting...\n");
				iStatus = 1;
			}
			else
			{
				printf("Done!\nInitializing encryption...");

				BOOL bStatus = 0;

				// Initialization of the AES context
				if (0 != CreateCipher(ctx, CBC, pbDerivedKey, 256, pbIV, 1)) {
					printf("An error occured during the creationg of the encryption context. Aborting...\n");
					iStatus = 1;
				}

				else
				{
					TCHAR szOpDesc[64] = {};
					BYTE pbHeader[16] = {};
					memcpy(pbHeader, "IDXCRYPTTPYRCXDI", 16);

					printf("Done!\n");

					memcpy(szOpDesc, "Encrypting the input file...\0", 29);

					printf(szOpDesc);

					/* write the random salt */
					/* write the random IV */

					if ((cbSalt != fwrite(pbSalt, 1, cbSalt, fout)) || (16 != fwrite(pbIV, 1, 16, fout))) {
						printf("An unexpected error occured while writing data to the output file. Aborting!\n");
						iStatus = 1;
					}

					else
					{
						/* protect encryption memory against swaping */
						VirtualLock(pbData, sizeof(pbData));

						memcpy(pbData, pbHeader, 16);
						cbData = 16;

						// AES encryption of the header using IV,DerivedKey
						if (0 != OpCipher(ctx, pbData, 16, pbData, cbData, cbData, 0))
						{
							printf("Unexpected error occured while encrypting. Aborting\n");
							iStatus = 1;
						}

						else
						{
							// write encrypted header 
							fwrite(pbData, 1, cbData, fout);
							startClock = clock();

							// We read 65536 bytes of the file at a time, which we encrypt
							// Until we reach the last 65536 block which we encrypt here, or we reach final block that is < 65536
							// We redo the 2nd initialization for every block of 65536 by giving it the currentIV
							while ((readLen = fread(pbData, 1, READ_BUFFER_SIZE, fin)) == READ_BUFFER_SIZE)
							{
								cbData = readLen;
								if (0 == OpCipher(ctx, pbData, cbData, pbData, cbData, cbData, 0))
								{
									totalProcessed += (__int64)READ_BUFFER_SIZE;
									if (cbData == fwrite(pbData, 1, cbData, fout))
									{
										ShowProgress(szOpDesc, inputLength, totalProcessed, FALSE);
									}
									else
									{
										printf("Not all encrypted bytes were written to disk. Aborting!\n");
										iStatus = 1;
										break;
									}
								}
								else
								{
									printf("Unexpected error occured while encrypting. Aborting!\n");
									iStatus = 1;
									break;
								}
							}

							if (iStatus == 0) {
								if (readLen < READ_BUFFER_SIZE && 0 != readLen)
								{
									if (ferror(fin))
									{
										printf("Unexpected error occured while reading data from input file. Aborting\n");
										iStatus = 1;
									}
									else
									{
										cbData = readLen;
										if (0 == OpCipher(ctx, pbData, cbData, pbData, cbData + 16, cbData, 1))
										{
											totalProcessed += (__int64)readLen;
											if (cbData == fwrite(pbData, 1, cbData, fout))
											{
												ShowProgress(szOpDesc, inputLength, totalProcessed, TRUE);
											}
											else
											{
												printf("Not all encrypted bytes were written to disk. Aborting!\n");
												iStatus = 1;
											}
										}
										else {
											printf("Unexpected error occured while encrypting. Aborting\n");
											iStatus = 1;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (0 == iStatus) {
		printf("Flushing output file data to disk, please wait...");
		printf("\rInput file %s successfully as \"%ls\"\n", bForDecrypt ? "decrypted" : "encrypted", outPath.data());
	}


	ctx.cleanCtx();
	SecureZeroMemory(pbData, READ_BUFFER_SIZE + 32);
	SecureZeroMemory(pbDerivedKey, 32);
	SecureZeroMemory(pbIV, 16);
	SecureZeroMemory(pbSalt, 64);

	return (iStatus);
}

/*
* Variant of Recursive Depth-First-Search(DFS) Algorithm
*/

static int opDir(WIN32_FIND_DATAW f, HANDLE h, const std::wstring finPath, const std::wstring foutPath, Hmac_PRF & prf, const char szPassword[], size_t & cbSalt, const int & bForDecrypt)
{
	FILE* fin = NULL;
	FILE* fout = NULL;
	std::wstring fileName{}, fileInPath{}, fileOutPath{};
	__int64 inputLength = 0;
	int iStatus = 0;

	while (FindNextFileW(h, &f) != 0) {
		fileName = f.cFileName;
		if ((fileName == L".") || (fileName == L"..")) {
			continue;
		}
		if ((f.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0) {	// directory

			fileOutPath = foutPath + fileName + L'\\';

			fileInPath = finPath.substr(0, finPath.length() - 1) + fileName + L"\\*"; // get rid of *

			if (!CreateDirectoryW(fileOutPath.data(), NULL) && ERROR_ALREADY_EXISTS != GetLastError()) {
				printf("Could not create/open the output folder %ls . (%d). Aborting...\n", fileOutPath.data(), GetLastError());
				iStatus = 1;
			}

			else
			{
				WIN32_FIND_DATAW F{};
				HANDLE H = FindFirstFileW(fileInPath.data(), &F);
				// Recursive call
				iStatus = opDir(F, H, fileInPath, fileOutPath, prf, szPassword, cbSalt, bForDecrypt);
				FindClose(H);
				SecureZeroMemory(&F, sizeof(F));
			}
		}
		else {	// file

			fileInPath = finPath.substr(0, finPath.length() - 1) + fileName;	// get rid of *

			fileOutPath = foutPath;
			if (0 == bForDecrypt) { // file to be encrypted => add .idx extension
				fileOutPath += fileName;
				fileOutPath += L".idx";
			}
			else fileOutPath += fileName.substr(0, fileName.length() - 4); // when decrypting, remove the .idx extension

			_wfopen_s(&fin, fileInPath.data(), L"rb");

			if (!fin)
			{
				printf("Failed to open the input file %ls for reading. Aborting...\n", fileInPath.data());
				iStatus = 1;
			}
			else
			{
				if ((inputLength = _filelengthi64(_fileno(fin))) == 0)
				{
					printf("The input file %ls is empty. Aborting...\n", fileInPath.data());
					iStatus = 1;
				}
				else
				{
					if (bForDecrypt && ((wcscmp(fileInPath.data() + fileInPath.size() - 4, L".idx") != 0) || (inputLength < (__int64)(48 + cbSalt)) || (inputLength % 16))) // salt+IV+header+some data (>=16 with padding) at least
					{
						printf("Error : input file %ls is not a valid encrypted file. Aborting...\n", fileInPath.data());
						iStatus = 1;
					}
					else
					{
						_wfopen_s(&fout, fileOutPath.data(), L"wb");
						if (!fout)
						{
							printf("Failed to open the output file %ls for writing. Aborting...\n", fileOutPath.data());
							iStatus = 1;
						}
						else {
							iStatus = opFile(fin, fout, fileOutPath, prf, szPassword, cbSalt, bForDecrypt);
						}
					}
				}
				if (fin) fclose(fin);
				if (fout) {
					fclose(fout);
					if (iStatus != 0) DeleteFileW(fileOutPath.data());
				}
			}
		}
	}

	return iStatus;
}


// check if file or dir
// if file, calls opfile
// if dir, calls opdir

int Win32_File::Op(Hmac_PRF & prf, const char szPassword[], const size_t & cbSalt, const int & bForDecrypt)
{
	WIN32_FIND_DATAW FindFileData = {};
	HANDLE hFind = {};
	BOOL isFile = TRUE;
	__int64 inputLength = 0;

	FILE* fin = nullptr;
	FILE* fout = nullptr;

	int iStatus = 0;

	// Stores information about the first file/folder which path corresponds to in FindFileData
	hFind = FindFirstFileW(absWideInpath.data(), &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		// We cannot know whether it is a file or a folder since it cannot even be found
		printf("File/folder %s not found (%d). Aborting...\n", absInpath.data(), GetLastError()); // 2 = ERROR_FILE_NOT_FOUND
		iStatus = 1;
	}
	else {
		// Determine whether argv[1] is a file or a folder
		FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? isFile = FALSE : isFile = TRUE;

		if (isFile) {
			_wfopen_s(&fin, absWideInpath.data(), L"rb");	// reading bits, no need to define the encoding

			if (!fin)
			{
				printf("Failed to open the input file %s for reading\n. Aborting...", absInpath.data());
				iStatus = -1;
			}

			else
			{
				if ((inputLength = _filelengthi64(_fileno(fin))) == 0)
				{
					printf("The input file %s is empty. No action will be performed. Aborting...\n", absInpath.data());
					iStatus = 1;
				}

				else
				{
					if (bForDecrypt && ((strcmp(absInpath.data() + absInpath.size() - 4, ".idx") != 0) || (inputLength < (__int64)(48 + cbSalt)) || (inputLength % 16))) // salt+IV+header at least
					{
						printf("Error : input file %s is not a valid encrypted file. Aborting...\n", absInpath.data());
						iStatus = 1;
					}

					else
					{
						// Check whether the user entered the output file with the correct extension ".idx"
						// Add it if it's not the case, before creating the file
						if (!bForDecrypt) {
							if (memcmp(absOutpath.data() + absOutpath.size() - 4, ".idx", 4) != 0) {
								absWideOutpath += L".idx";

								absOutpath += ".idx";
							}
						}

						_wfopen_s(&fout, absWideOutpath.data(), L"wb");
						if (!fout)
						{
							printf("Failed to open the output file %s for writing. Aborting...\n", absOutpath.data());
							iStatus = 1;
						}

						else
						{
							iStatus = opFile(fin, fout, absWideOutpath, prf, szPassword, cbSalt, bForDecrypt);
						}
					}
				}
			}
		}
		else {
			// If there is an error creating the output directory and this error is not ERROR_ALREADY_EXISTS
			if (CreateDirectoryW(absWideOutpath.data(), NULL) == 0 && ERROR_ALREADY_EXISTS != GetLastError()) {
				printf("Could not create/open the output folder %s . (%d). Aborting...\n", absOutpath.data(), GetLastError());
				iStatus = 1;
			}

			else
			{
				if (absInpath[absInpath.size() - 1] != '\\') {
					absInpath += "\\*";

					absWideInpath += L"\\*";
				}

				if (absOutpath[absOutpath.size() - 1] != '\\') {
					absOutpath += '\\';

					absWideOutpath += L'\\';
				}

				hFind = FindFirstFileW(absWideInpath.data(), &FindFileData);

				iStatus = opDir(FindFileData, hFind, absWideInpath, absWideOutpath, prf, szPassword, (size_t&)cbSalt, bForDecrypt);
			}
		}
	}

	FindClose(hFind);
	SecureZeroMemory(&FindFileData, sizeof(FindFileData));

	if (fin) fclose(fin);
	if (fout) {
		fclose(fout);
		if (iStatus != 0) DeleteFileW(absWideOutpath.data());
	}

	return iStatus;
}



#endif
