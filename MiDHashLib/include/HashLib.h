/*
 *	=====================================
 *	Copyright (c) El Mostafa IDRASSI 2017
 *	mostafa.idrassi@tutanota.com
 *	Apache License
 *	=====================================
 */

#ifndef HASHLIB_H
#define HASHLIB_H

/* Library's headers */
#include "HashContext.h"

/*  ====================================================================
Makes sure all the functions provided by the library are working as expected
*	MD5 : Based on RFC 1321 ; No NIST based test vector available
*	SHA1, SHA2 : Based on FIPS 180-4 NIST test vectors (byte-oriented) : http://csrc.nist.gov/groups/STM/cavp/secure-hashing.html
*/
 int HashLib_Init();

#endif // !HASHLIB_H