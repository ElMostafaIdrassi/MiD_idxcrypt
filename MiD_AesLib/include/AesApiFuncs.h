/*
 *	=====================================
 *	Copyright (c) El Mostafa IDRASSI 2017
 *	mostafa.idrassi@tutanota.com
 *	Apache License
 *	=====================================
 */

#ifndef AESAPIFUNCS_H
#define AESAPIFUNCS_H

#include "AES_CTX.h"

/*  ====================================================================
AES context object initialization - Encryption/Decryption parameters
*	Sets the mode_number and the key_size (provided in bits)
*	Expands the userKey into AES round keys
*	Sets the IV and the encryption parameter
*/
 int CreateCipher(AES_CTX & ctx, const Mode_Number & mode_number, const unsigned char * userKey, const int & key_size, const unsigned char * iv, const int & toBeEncrypted);

/*	====================================================================
Applies the appropriate AES cipher mode
*	Stores the input in a permanent buffer
*	Applies PKCS padding when required/needed during encryption
*	Checks padding and removes it during decryption
*	out_initial_len contains the length of the output when first initialized; alimented by the user; input of the function
*	out_final_len will contain the proper length of the output after enc/dec and obviously removing the padding; output of the function
*/
 int OpCipher(AES_CTX & ctx, const unsigned char * in, const size_t & in_len, unsigned char * out, const size_t & out_initial_len, size_t & out_final_len, const int & paddingParam);

/*	====================================================================
Deletes securely all CTX variables/fields
*	memset instructions can be optimized by the compiler, and thus be ignored => my_memclr
*/
 void CleanCipher(AES_CTX & ctx);

/*	====================================================================
Makes sure all the functions provided by the library are working as expected
Return 0 if successful and 1 if there was a failure.
*	Based on NIST Advanced Encryption Standard Algorithm Validation Suite and partially on RFC 3686 (CTR)
*	(1 GFSbox and 1 KeySbox test / key length / mode (ECB, CBC, CFB, OFB))
*	(1 RFC 3686 test / key length for CTR)
*	(1 Multi-Block-Message test / key length / mode (all modes) : 36 bytes for CTR and 32 bytes for the other modes)
*	(Arbitrary length plaintext testing on ECB/CBC will only validate padding functionning => already validated (PKCS#7) => not included in the tests)
*/
 int AesLib_Init();

#endif