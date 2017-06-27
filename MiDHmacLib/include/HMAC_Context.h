/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifndef HMAC_CONTEXT_H
#define HMAC_CONTEXT_H

#include <vector>
#include <cstddef>

typedef enum {
	md5f = 0,
	sha1f,
	sha256f,
	sha384f,
	sha512f
} HashFunction;

class HMAC_Context
{
	void* HFunctionPtr = nullptr;

	std::vector<unsigned char> Key{};		// After padding, a.k.a K0

	std::vector<unsigned char> InnerPad{};
	std::vector<unsigned char> OuterPad{};

public:
	 HMAC_Context();
	 ~HMAC_Context();

	 HMAC_Context(const HMAC_Context & other);
	 HMAC_Context & operator = (const HMAC_Context & other);
	 HMAC_Context(HMAC_Context && other);
	 HMAC_Context & operator = (HMAC_Context && other);

	/*  ========================================================
	*	Defines the Hash context to which HFunctionPtr points to
	*	========================================================
	*/
	 int setHPtr(const HashFunction & h);

	/*  ===============================================================================================
	*	Initializes Key with the padded input key (1st / preprocessing step of the HMAC algorithm)
	*	Initializes InnerPad and OuterPad (with respectively a sequence of 0x36 and 0x5c) then
	*	Reinitializes InnerPad and OuterPad (with respectively, Key XOR InnerPad and Key XOR OuterPad)
	*	(in case we need to reuse the same HMAC type but with a different key, so same blocksize etc)
	*	===============================================================================================
	*/
	 int setKey(const unsigned char key[], const unsigned int & keyLength);

	/*  ================================================
	*	Returns the hash size of the used hash algorithm
	*	================================================
	*/
	unsigned int getHashSize();

	/*  ============================================================================
	*	Main HMAC algorithm
	*	Input length must be < 2^(B-3) - B bytes such as B is Hash block size (NIST)
	*	============================================================================
	*/
	 int HMAC(const unsigned char *input, const size_t & inLen, unsigned char *output, size_t & outLen);

	/*	==================================================
	*	Cleans key, InnerPad, OuterPad and the HashContext
	*	==================================================
	*/
	 void cleanData();

	 HashFunction getHashFunction() const;

};

#endif // !HMAC_CONTEXT_H