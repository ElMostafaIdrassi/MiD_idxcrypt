/*
 *	=====================================
 *	Copyright (c) El Mostafa IDRASSI 2017
 *	mostafa.idrassi@tutanota.com
 *	Apache License
 *	=====================================
 */

#ifndef AESCIPHERMODE_H
#define AESCIPHERMODE_H

#include <cstddef>		// size_t

typedef enum
{
	ECB = 0,
	CBC,
	CFB,
	OFB,
	CTR
} Mode_Number;

class AES_CIPHER_MODE
{
	Mode_Number mode_number = ECB;		/* Must be one of Mode_Number enum values */
	int key_size = 0;					/* Must be 128, 192 or 256 */
public:
	AES_CIPHER_MODE();

	/* 2-argument constructor (for initialization) */
	AES_CIPHER_MODE(const Mode_Number & n, const int & size);

	/* Copy constructor (initialization from a lvalue) */
	AES_CIPHER_MODE(const AES_CIPHER_MODE & other); 

	/* Copy assignment (from a lvalue) */
	AES_CIPHER_MODE & operator=(const AES_CIPHER_MODE & other);

	/* Move constructor (initialization from a rvalue, temporary) */
	AES_CIPHER_MODE(AES_CIPHER_MODE && other);				

	/* Move assignment (from a rvalue, temporary) */
	AES_CIPHER_MODE & operator= (AES_CIPHER_MODE && other);	

	Mode_Number getMode() const;
	int getKeySize() const;

	void setMode(Mode_Number m);
	void setKeySize(int keySize);

	void cleanMode();

	~AES_CIPHER_MODE();

};

#endif /* AESCIPHERMODE_H */