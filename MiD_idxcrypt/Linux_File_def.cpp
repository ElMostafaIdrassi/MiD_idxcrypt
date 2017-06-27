/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifdef __linux__

#include "Linux_File_dec.cpp"

#include "mem_impl_dec.cpp"                     // my_memclr

#include "MyLinuxSysFunctions.h"				// getAbsolutePath

#include <errno.h>
#include <iostream>								// cerr, cout
#include <sys/mman.h>							// mlock

/* Function to display information about the progress of the current operation */
clock_t startClock = 0;
clock_t currentClock = 0;

void ShowProgress(char * szOperationDesc, __int64 inputLength, __int64 totalProcessed, bool bFinalBlock)
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

Linux_File::Linux_File()
{
}

Linux_File::~Linux_File()
{
}

int Linux_File::relativeToAbsolutePath(const int & inOut)
{
    std::string absPath{};
    int iStatus = getAbsolutePath(inOut == 0 ? relativeInpath : relativeOutpath, absPath);
    
    if (0 != iStatus) 
        std::cerr << "\nThere was an error while converting relative " << (inOut == 0 ? "input" : "output") << " path to an absolute one. Last error code : " << errno << "\n\n";
    else
        inOut == 0 ? absInpath = absPath : absOutpath = absPath;
    
    return iStatus;
}

int Linux_File::setPaths(const std::string & pIn, const std::string & pOut)
{
    int iStatus = 0;

    if (pIn[0] != '/') {	// input path is relative to the current working directory, get absolute path and store both
        
        relativeInpath = pIn;
        iStatus = relativeToAbsolutePath(0);
        
    }
    else {					// input path is absolute, store without the trailing "/"
        absInpath = pIn;
        if (absInpath != "/" && absInpath.back() == '/') absInpath.pop_back();  // in case input path is "/", dont pop it
    }

    // For the output, we seperate dir from base, because the user can enter a path
    // where the file (base) doesn't actaully exist. However, the dir should exist.

    if (pOut[0] != '/') {	// output path is relative to the current working directory, get absolute path and store both
            
        std::string dir{}, base{};

        // relativeOutpath = pOut;
        
        // We separate at this stage because base could be non-existant
        dirname_base_separator(pOut, dir, base);
        
        if (base==".." || base == ".") relativeOutpath = pOut;
        else relativeOutpath = dir;
        
        iStatus = relativeToAbsolutePath(1);
        
        if (base!=".." && base!="."){
            relativeOutpath += base;
            absOutpath += "/"+base;
        }
        
    }
    else {                      // if output path is absolute, store without the trailing "/"
        absOutpath = pOut;
        if (absOutpath != "/" && absOutpath.back() == '/') absOutpath.pop_back();   // in case output path is "/", dont pop it
    }

    return iStatus;
}

static int opFile(FILE* fin, FILE* fout, __int64 inputLength, const std::string & outPath, Hmac_PRF & prf, const char szPassword[], const size_t & cbSalt, const int & bForDecrypt)
{
    unsigned char pbDerivedKey[32] = {};
    unsigned char pbSalt[64] = {}, pbIV[16] = {};
    unsigned char pbData[READ_BUFFER_SIZE + 32] = {};
    size_t cbData = 0;
    size_t readLen = 0;
    __int64 totalProcessed = 0;
    int iStatus = 0;

    AES_CTX ctx{};

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
                    char szOpDesc[64]{};
                    unsigned char pbHeader[16]{};
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
                                bool bFinal = false;
                                startClock = clock();

                                // We read 65536 bytes of the decrypted file at a time, which we decrypt
                                // Until we reach the last 65536 block which we decrypt here, or we reach final block that is < 65536
                                // We redo the 2nd initialization for every block of 65536 by giving it the currentIV
                                while (((cbData = fread(pbData, 1, READ_BUFFER_SIZE, fin)) == READ_BUFFER_SIZE) && false == bFinal)
                                {
                                    totalProcessed += (__int64)READ_BUFFER_SIZE;
                                    bFinal = (totalProcessed == inputLength) ? true : false;
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
                                
                                if (iStatus == 0){
                                    // if there is still a trailing block < 65536
                                    if (false == bFinal)
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
                                                    ShowProgress(szOpDesc, inputLength, totalProcessed, true);
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
            unsigned long dwErr = ERR_get_error();
            printf("An unexpected error occured while preparing for the encryption (Code 0x%.8lu). Aborting...\n", dwErr);
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

                AES_CTX ctx = {};
                
                // Initialization of the AES context
                if (0 != CreateCipher(ctx, CBC, pbDerivedKey, 256, pbIV, 1)) {
                    printf("An error occured during the creationg of the encryption context. Aborting...\n");
                    iStatus = 1;
                }
                
                else
                {
                    char szOpDesc[64] = {};
                    unsigned char pbHeader[16] = {};
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
                    else{
                        /* protect encryption memory against swaping */
                        mlock(pbData, sizeof(pbData));

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
                                        ShowProgress(szOpDesc, inputLength, totalProcessed, false);
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
                            
                            if (iStatus == 0){
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
                                                ShowProgress(szOpDesc, inputLength, totalProcessed, true);
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
        printf("\rInput file %s successfully as \"%s\"\n", bForDecrypt ? "decrypted" : "encrypted", outPath.data());
    }

    ctx.cleanCtx();
    my_memclr(pbData, READ_BUFFER_SIZE + 32);
    my_memclr(pbDerivedKey, 32);
    my_memclr(pbIV, 16);
    my_memclr(pbSalt, 64);

    return (iStatus);
}

/*
 * Variant of Recursive Depth-First-Search(DFS) algorithm without an explicit stack used
 */
static int opDir(DIR* dir, const std::string finPath, const std::string foutPath, Hmac_PRF & prf, const char szPassword[], size_t & cbSalt, const int & bForDecrypt)
{
    int iStatus = 0;
    std::string fileName{}, fileInPath{}, fileOutPath{};
    __int64 inputLength=0;
    dirent *entry = {};                         // to collect the dir entries info (names...)
    
    while ((entry = readdir(dir)) != nullptr)	// As long as there	are still entries in the directory pointed by dir
    {
        fileName = entry->d_name;
        if ((fileName == ".") || (fileName == "..")) 
        {
            continue;
        }
        if (entry->d_type == DT_DIR)			// Entry is a directory
        {   
            fileOutPath = foutPath + fileName + "/";
            
            fileInPath = finPath + fileName + "/";
            
            // mode 755 for directories
            // If there is an error creating the output directory and this error is not "Directory already exists"
            if (0 != mkdir(fileOutPath.data(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
            {
                std::cerr << "An error occured while attempting to create the output directory " << fileOutPath << " . (OpDir - mkdir) Error code : " << errno << ". Aborting...\n";
                iStatus = 1;
            }
            else
            {
                DIR* dir = opendir(fileInPath.data()); // Contains info about all directories under fileInPath
                
                if (nullptr == dir)     // opendir error
                { 
                    std::cerr << "An error occured while attempting to access files under the input directory " << fileInPath << " . Error code : " << errno << ". Aborting...\n";
                    iStatus = 1;
                }
                else{
					// Recursive call 
                    iStatus = opDir(dir, fileInPath, fileOutPath, prf, szPassword, (size_t&)cbSalt, bForDecrypt);
                    closedir(dir);
                }
            }
        }
        
        else if (entry->d_type == DT_REG)		// Entry is a regular File
        {  
            struct stat stat_buf{};
            FILE * fin = nullptr;
            FILE * fout = nullptr;
            
            fileOutPath = foutPath;
            if (0 == bForDecrypt)           // if the file is to be encrypted => add .idx extension
            { 
                fileOutPath += fileName;
                fileOutPath += ".idx";
            }
            else fileOutPath += fileName.substr(0, fileName.length() - 4); // if decrypting, remove the .idx extension
            
            fileInPath = finPath + fileName;
            
            fin = fopen(fileInPath.data(), "rb");

            if (!fin)
            {
                printf("Failed to open the input file (%s) for reading. Aborting...\n", fileInPath.data());
                iStatus = 1;
            }
            else
            {
				// Retrieve information about the file
                if (0 == stat(fileInPath.data(), &stat_buf))		// stat ok
                {
                    if ((inputLength = stat_buf.st_size) == 0)
                    {
                        printf("The input file %s is empty. Aborting...\n", fileInPath.data());
                        iStatus = 1;
                        my_memclr(&stat_buf, sizeof(stat_buf));
                    }
                    else
                    {
                        my_memclr(&stat_buf, sizeof(stat_buf));
                        if (bForDecrypt && ((strcmp(fileInPath.data() + fileInPath.size() - 4, ".idx") != 0) || (inputLength < (__int64)(48 + cbSalt)) || (inputLength % 16))) // salt+IV+header+some data (>=16) at least
                        {
                            printf("Error : input file %s is not a valid encrypted file. Aborting...\n", fileInPath.data());
                            iStatus = 1;
                        }
                        else
                        {
                            fout = fopen(fileOutPath.data(), "wb");
                            if (!fout)
                            {
                                printf("Failed to open the output file %s for writing. Aborting...\n", fileOutPath.data());
                                iStatus = 1;
                            }
                            else iStatus = opFile(fin, fout, inputLength, fileOutPath, prf, szPassword, cbSalt, bForDecrypt);
                        }
                    }
                }
                else        // stat error : while getting stat structure
                {  
                    std::cerr << "An error occured when trying to get " << fileInPath << " ST_STAT. (OpDir - fstat) Error code : " << errno << ". Aborting...\n";
                    iStatus = 1;
                }
                if (fin) fclose(fin);
                if (fout){
                    fclose(fout);
                    if (iStatus != 0) remove(fileOutPath.data());     // Delete output file in case of an error
                }
            }
        }
    }
    
    return iStatus;
}

int Linux_File::Op(Hmac_PRF & prf, const char szPassword[], const size_t & cbSalt, const int & bForDecrypt)
{
    struct stat stat_buf{};
    FILE* fin = nullptr;
    FILE* fout = nullptr;
    int isFile = 1;
    __int64 inputLength = 0;
    int initial_fd = 0;
    int iStatus = 0;
    
    // First, attempt to get a file descriptor of absInpath
    if ((initial_fd = open(absInpath.data(), O_RDONLY)) <= 0){        // open error
        std::cerr << "An error occured when trying to get " << absInpath << " File Descriptor. (Op - open) Error code : " << errno << ". Aborting...\n";
        iStatus = 1;
    }
    else    // Successful attempt to get file descriptor
    {
        // Attempt to get stat structure, and precisely the file mode (type)
        if (0 == fstat(initial_fd, &stat_buf))                  // fstat OK
        {                 
            close(initial_fd);
            if ((stat_buf.st_mode & S_IFMT) == S_IFREG)         // if regular file
            {        
                isFile = 1;
                if ((inputLength = stat_buf.st_size) == 0)      // N.B : stat_buf.st_size is not always accurate
                {
                    std::cerr << "The input file " << absInpath << " is empty. No action will be performed. Aborting...\n";
                    iStatus = 1;
                }
                else
                {
                    fin = fopen(absInpath.data(), "rb");
                
                    if (!fin)
                    {
                        std::cerr << "Failed to open the input file " << absInpath <<  " for reading. Error code : " << errno << "%d. Aborting...\n";
                        iStatus = 1;
                    }
                    
                    else
                    {
                        if (bForDecrypt && ((memcmp(absInpath.data() + absInpath.size() - 4, ".idx", 4) != 0) || (inputLength < (__int64)(48 + cbSalt)) || (inputLength % 16))) // salt+IV+header at least + some data (at least 16 because of padding, otherwise > 16)
                        {
                            std::cerr << "Error : input file " << absInpath << " is not a valid encrypted file. Aborting...\n";
                            iStatus = 1;
                        }

                        else
                        {
                            // Check whether the user entered the output file with the correct extension ".idx"
                            // Add it if it's not the case, before creating the file
                            if (!bForDecrypt) {
                                if (memcmp(absOutpath.data() + absOutpath.size() - 4, ".idx", 4) != 0) 
                                {
                                    absOutpath += ".idx";
                                }
                            }

                            fout = fopen(absOutpath.data(), "wb");
                            if (!fout)
                            {
                                std::cerr << "Failed to open the output file " << absOutpath << " for writing. Error code : " << errno << " .Aborting...\n";
                                iStatus = 1;
                            }
                            else
                            {
                                iStatus = opFile(fin, fout, inputLength, absOutpath, prf, szPassword, cbSalt, bForDecrypt);
                            }
                        }
                    }
                }
            }
            else if ((stat_buf.st_mode & S_IFMT) == S_IFDIR)    // if directory
            {    
                isFile = 0;
                
                // mode 755 for directories (Owner : all permissions, Group : read and search, Others : read and search)
                // If there is an error creating the output directory and this error is not "Directory already exists"
                if (0 != mkdir(absOutpath.data(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST){
                    std::cerr << "An error occured while attempting to create the output directory " << absOutpath << " . (Op - mkdir) Error code : " << errno << ". Aborting...\n";
                    iStatus = 1;
                }
                else
                {
                    DIR* dir = opendir(absInpath.data());   // Contains info about all directories under input directory
                
                    if (nullptr == dir)                     // opendir error
                    { 
                        std::cerr << "An error occured while attempting to access files under the input directory " << absInpath << " . Error code : " << errno << ". Aborting...\n";
                        iStatus = 1;
                    }
                    else
                    {
                        iStatus = opDir(dir, absInpath + "/", absOutpath + "/", prf, szPassword, (size_t&)cbSalt, bForDecrypt);
                        closedir(dir);
                    }
                }
            }
            else {	// if another type
                std::cerr << "File type not supported. Only supported types are regular files and directories. Aborting...\n";
                iStatus = 1;
            }
        }
        else		// fstat error : while getting stat structure
        {  
            std::cerr << "An error occured when trying to get " << absInpath << " ST_STAT. (Op - fstat) Error code : " << errno << ". Aborting...\n";
            iStatus = 1;
        }
    }
    
    if (fin) fclose(fin);
    if (fout){
        fclose(fout);
        if (iStatus != 0) remove(absOutpath.data());     // Delete output file in case of an error
    }
    my_memclr(&stat_buf, sizeof(stat_buf));
    
    return 0;
}

#endif
