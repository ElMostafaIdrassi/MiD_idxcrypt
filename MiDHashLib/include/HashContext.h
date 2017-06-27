/*
 *	=====================================
 *	Copyright (c) El Mostafa IDRASSI 2017
 *	mostafa.idrassi@tutanota.com
 *	Apache License
 *	=====================================
 */

#ifndef HASHCONTEXT_H
#define HASHCONTEXT_H

#include <cstddef> // size_t declaration

typedef enum {
	md5 = 0,
	sha1,
	sha256,
	sha384,
	sha512
} HashAlgo;

class HashContext // Factory Class
{
public:
	  HashContext();
	  virtual ~HashContext() = 0;

	  // deleted because not used => no implicit declaration/use by the compiler
	  // All derived class copy/move constructors actually call the default base ctor
	  HashContext(const HashContext & ) = delete;
	  HashContext & operator=(const HashContext & ) = delete;
	  HashContext(HashContext && ) = delete;
	  HashContext & operator=(const HashContext && ) = delete;

	  virtual int InitHashCtx() = 0;
	  virtual int UpdateHashCtx(const char * in_data, const size_t & in_len) = 0;
	  virtual int FinalHashCtx(unsigned char * md) = 0;

	  virtual unsigned int getHashSize() const = 0;	// In case we use a base pointer to a derived object, this will be called instead of the derived one => Needs to be declared virtual (pure or not) and redefined in every derived class
	  virtual unsigned int getBlockSize() const = 0;

	  virtual HashAlgo getHashAlgo() const = 0;

	  // Factory generator, uses default ctor of all hash algos
	  static HashContext * CreateHashContext(const HashAlgo & h);

	  // Factory generator, uses copy ctor of all hash algos
	  static HashContext * CreateHashContext(const HashContext & other, const HashAlgo & h);

	  // Factory generator, uses move ctor of all hash algos
	  static HashContext * CreateHashContext(HashContext && other, const HashAlgo & h);

	  virtual void cleanup() = 0;
};

#endif // !HASHCONTEXT_H