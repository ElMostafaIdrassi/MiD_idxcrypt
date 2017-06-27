/*
 *	=====================================
 *	Copyright (c) El Mostafa IDRASSI 2017
 *	mostafa.idrassi@tutanota.com
 *	Apache License
 *	=====================================
 */

#ifndef AESCTX_H
#define AESCTX_H

#include "AES_CIPHER_MODE.h"

#include <vector>

class AES_CTX
{
	AES_CIPHER_MODE mode{ ECB, 0 };							/* Holds AES operation mode ; Automatic allocation, will be freed automatically */
	std::vector<unsigned char> key{};						/* Holds AES_Key structure ; we use generic buffer with sizeof(AES_KEY) to avoid including openssl headers. Automatic allocation */
	size_t in_len{ 0 };										/* Holds input length */
	int ctxInitialized{ 0 };								/* Determines whether the AES_CTX object has been initialized or not */
	int toBeEncrypted{ 1 };									/* Determines whether to encrypt or decrypt */
	int toBePadded{ 1 };									/* Determines whether the padding is required or not by the cipher mode - Padding required by default */
	int num{ 0 };											/* Contains the number of bytes operated on in last block (for cfb/ofb/ctr) - Always initialized with 0, until modified later */
	std::vector<unsigned char> buf_in{};					/* Dynamic array, which will hold the input that needs or not to be padded, and enc/dec */
	unsigned char original_iv[16] = {};						/* Static array which holds the original value of the iv */
	unsigned char current_iv[16] = {};						/* Static array which holds the last value of iv in order to operate later */
public:
	AES_CTX();
	/* 1-argument constructor (for initialization) */
	explicit AES_CTX(const AES_CIPHER_MODE & m);

	// Copy constructor (initialization from a lvalue)
	AES_CTX(const AES_CTX & other);
	// Copy assignment (from a lvalue)
	AES_CTX& operator=(const AES_CTX & other);

	// Move constructor (initialization from a rvalue, temporary)
	AES_CTX(AES_CTX && other);
	// Move assignment (from a rvalue, temporary)
	AES_CTX& operator=(AES_CTX && other);

	~AES_CTX();

	void cleanCtx();

	AES_CIPHER_MODE getCtxMode() const;
	Mode_Number getCtxModeNumber() const;
	int getCtxKeySize() const;
	std::vector<unsigned char> * getKey();
	size_t getInLen() const;
	int getInitParam() const;
	int getEncryptionParam() const;
	int getPaddingParam() const;
	int * getNum();
	std::vector<unsigned char> * getInBuffer();
	const unsigned char * getOriginalIV() const;
	unsigned char * getCurrentIV();

	void setCtxModeNumber(Mode_Number m);
	void setCtxKeySize(int keySize);
	void setInLen(const size_t & len);
	void setInitParam();
	void setEncryptionParam(const int & n);
	void setPaddingParam(const int & p);
	void setNum(const int & n);
	void initInBuffer(const size_t & in_len);
	void setInBuffer(const unsigned char * in);
	void setOriginalIV(const unsigned char * ivec);
	void setCurrentIV(const unsigned char * ivec);
};


#endif /* AESCTX_H */
