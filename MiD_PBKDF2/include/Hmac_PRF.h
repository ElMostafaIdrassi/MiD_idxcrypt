/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#ifndef HMACPRF_H
#define HMACPRF_H

#define POWER1 4294967296 // 2^32

#include <cstddef>

typedef enum {
	md5h = 0,
	sha1h,
	sha256h,
	sha384h,
	sha512h
} HmacAlgo;

class Hmac_PRF
{
	void* hc = nullptr; // Hmac context
	/* PRF = F : Key x Input -> Output */
	unsigned int hLen = 0; // = Hash size
public:
	 Hmac_PRF();
	  ~Hmac_PRF();

	 Hmac_PRF(const Hmac_PRF & other);
	 Hmac_PRF & operator = (const Hmac_PRF & other);
	 Hmac_PRF(Hmac_PRF && other);
	 Hmac_PRF & operator = (Hmac_PRF && other);

	/*	========================================================================================
	*	Sets the PRF Hmac Context (hc) to one of the HMACs : MD5, SHA1, SHA256, SHA384 or SHA512
	*	Initializes hLen with the output size = hash size
	*	========================================================================================
	*/
	  int setHmacContext(const HmacAlgo & h);

	/*	==============================
	*	Sets the key => PRF of Key (x)
	*	==============================
	*/
	  int setHmacKey(const unsigned char key[], const unsigned int & keyLength);

	/*	=============================================
	*	Calculates the HMAC of input using PRF of key 
	*	=============================================
	*/
	  int opPRF(const unsigned char *input, const size_t & inLen, unsigned char *output, size_t & outLen);

	/*	===========================
	*	Cleans data in HMAC context 
	*	===========================
	*/
	  void cleanData();

	/*	=================
	*	Returns hash size 
	*	=================
	*/
	  unsigned int gethLen();

	  HmacAlgo getHmacAlgo();
};

#endif // !HMACPRF_H