/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifdef  __linux__

#include "File_Struct.h"

class Linux_File
	: public File_Struct
{
private:

    std::string relativeInpath{};			// relative input path 
    std::string relativeOutpath{};			// relative output path 

    std::string absInpath{};				// absolute input path 
    std::string absOutpath{};				// absolute output path

public:

    Linux_File();

    /*	Copy, Move constructor and assignment operators declared to fulfill "Rule of Five"
        Declared as delete because they are never used 
    */

    // Copy constructor
    Linux_File(const Linux_File & other) = delete;
    // Copy assignment
    Linux_File & operator=(const Linux_File & other) = delete;
    // Move constructor
    Linux_File(Linux_File && other) = delete;
    // Move assignment
    Linux_File & operator=(Linux_File && other) = delete;

    ~Linux_File();

    int relativeToAbsolutePath(const int & inOut) override;

    int setPaths(const std::string & pIn, const std::string & pOut) override;

    int Op(Hmac_PRF & prf, const char szPassword[], const size_t & cbSalt, const int & bForDecrypt) override;

};

#endif // __linux__
